#include <devices/driver.h>

#include <devices/cpu/cpu_utils.h>
#include <devices/cpu/arch/x86/global/cpu_utils_x86.h>
#include <devices/cpu/arch/x86/global/cpu_defs_x86.h>

#include <libk/mmu/mmu.h>
#include <libk/utils/verify.h>
#include <libk/string.h>

static bool cpu_mpfp_criterion(usize paddr);
static void* cpu_search_mpfp();
static mpct_hdr_t* cpu_mpct_check(u32 mpct_phys_base); 


struct generic_driver_tree_node_t* cpu_create_mp_table_node(
    struct generic_driver_tree_node_t* cpu_self
)
{
    struct generic_driver_tree_node_t* mp_table_drv_node = NULL;
    
    // Find the MPFP
    driver_log_state(cpu_self, DRIVER_LOG_NOTICE, "Searching for the MP Floating Pointer...\n");

    void* desired_mpfp = cpu_search_mpfp();
    if (!desired_mpfp)
    {
        driver_log_state(cpu_self, DRIVER_LOG_WARN, "The MPFP cannot be found\n");
        return NULL;
    }

    // Find the MPCT, our needed fallback table, in case there's no ACPI.
    driver_log_state(cpu_self, DRIVER_LOG_NOTICE, 
        "Found the MPFP. Checking if the MPCT base is valid...\n"
    );
    
    u32 mpct_phys = ((mpfp_t*)desired_mpfp)->paddr;

    mmu_munmap((u32)desired_mpfp);

    mpct_hdr_t* valid_mpct = cpu_mpct_check(mpct_phys);
    if (!valid_mpct)
    {
        driver_log_state(cpu_self, DRIVER_LOG_WARN, "invalid MPCT\n");
        return NULL;
    }
    
    // Create the driver node, attach the MP Table to it, and return.
    mp_table_drv_node = driver_add_to_parent(
        cpu_self,
        DRIVER_ID_TYPE_INTERNAL,
        DRIVER_BUS_TYPE_CPU,
        90,
        DRIVER_MODE_KRNL
    );
    if (!mp_table_drv_node)
    {
        driver_log_state(cpu_self, DRIVER_LOG_WARN, 
            "Out of memory trying to create a new MP node\n"
        );
        return NULL;
    }

    driver_set_id_data(mp_table_drv_node, "GENERIC_CPU_MPS");
    mp_table_drv_node->state = DRIVER_STATE_READY;

    mp_table_drv_node->additionals = valid_mpct;
    return mp_table_drv_node;
}

void cpu_remove_mp_table_node(
    struct generic_driver_tree_node_t* cpu_self, 
    struct generic_driver_tree_node_t* mps_driver_node 
)
{
    mpct_hdr_t* mpct = (mpct_hdr_t*)mps_driver_node->additionals;
    if (!mpct) goto final;

    u64 page_count = mmu_byte_to_page_count(mpct->length);

    mmu_munmapn((usize)mpct, &page_count);

    if (mpct->spec_revision == 4)
    {
        u64 ext_page_count = mmu_byte_to_page_count(mpct->ext_fields.ext_length);
        mmu_munmapn((usize)((u8*)mpct+mpct->length), &ext_page_count);
    }

    mps_driver_node->additionals = NULL;
    
final:
    driver_remove_from_tree(
        NULL, mps_driver_node
    );
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

static bool cpu_mpfp_criterion(usize paddr)
{
    mpfp_t* tmp = (mpfp_t*)paddr;
    return  /* We check for the following: */
    memcmp(tmp->signature, "_MP_", 4) == 0 // If the Signature is "_MP_"
    && tmp->length == 1 // If the length is exactly 1
    && verify_8bit_checksum(tmp, tmp->length*16) // Checksum is valid
    && (tmp->spec_revision==1 || tmp->spec_revision==4); // Only rev 1.1 and 1.4 is acceptable
}

static void* cpu_search_mpfp()
{
    void* desired_mpfp = NULL;

    // Search for the MPFP

    // According to the Intel MultiProcessor Specification v1.4 (May 1997)
    // (See https://en.wikipedia.org/wiki/MultiProcessor_Specification)
    // The MPS can be in one of four regions below:
    // [1]: First KiB of the Extended BIOS Data Area (EBDA). Its base
    // will be stored in 0x40E, at the BIOS Data Area (BDA) region.
    // [2]: Last KiB of the System Base Memory (Conventional memory before 0xA0000).
    // The search size will be stored in 0x413, at the BDA region.
    // [3]: The BIOS ROM (0xF0000 to 0xFFFFF)

    // As the structure is aligned, the search steps will be 16-byte aligned.
    // If signature "_MP_" is found, we have found the struct.
    // We'll need to map our MPFP as RO, as always.

    // Case 1: The EBDA
    // Get the base from 0x40E, but map that first before doing anything
    
    mmu_mmap(0x40E, 0x40E, 0); // RO
    u16* ebda_ptr = (u16*)0x40E;
    u32 ebda_base = ((u32)*ebda_ptr)<<4;
    mmu_munmap(0x40E);

    // Search within the first 1024 bytes of the EBDA
    // MMU search will map this, so no worries.
    desired_mpfp = mmu_search_memrange(ebda_base, 1024, 16, 0, cpu_mpfp_criterion); // RO
    if (desired_mpfp) goto done;

    // Case 2: Conventional memory sub-0xA0000

    mmu_mmap(0x413, 0x413, 0);
    u16* low_mem_size_ptr = (u16*)0x413;
    u16 low_mem_size_in_kb = *low_mem_size_ptr;
    mmu_munmap(0x413);

    u32 low_mem_end = low_mem_size_in_kb*1024;
    
    // Check if the low memory is 1K or higher. 
    // Else, there's not enough memory to do the search.
    if (low_mem_end >= 1024)
    {
        u32 search_area = low_mem_end-1024; // Search in the last KiB of low memory
        desired_mpfp = mmu_search_memrange(search_area, 1024, 16, 0, cpu_mpfp_criterion); // RO
        if (desired_mpfp) goto done;
    }

    // Case 3: The BIOS ROM
    desired_mpfp = mmu_search_memrange(0xF0000, 0x10000, 16, MMU_PT_FLAG_CACHE_DISABLE, cpu_mpfp_criterion); // RO
    if (desired_mpfp) goto done;

    return NULL; // Not found
done:
    return desired_mpfp;
}

static mpct_hdr_t* cpu_mpct_check(u32 mpct_phys_base)
{
    if (mpct_phys_base == 0) return NULL;

    mmu_mmap(mpct_phys_base, mpct_phys_base, 0); // RO
    mpct_hdr_t* valid_mpct = (mpct_hdr_t*)mpct_phys_base;

    // Check the signature. Is it "PCMP"?
    if (memcmp(valid_mpct->signature, "PCMP", 4) != 0)
    {
        mmu_munmap(mpct_phys_base);
        return NULL;
    }
    
    // Check the revision if it's either 1.1 or 1.4
    if (valid_mpct->spec_revision != 1 && valid_mpct->spec_revision != 4)
    {
        mmu_munmap(mpct_phys_base);
        return NULL;
    }
    
    // Calculate the base length
    u16 length = valid_mpct->length;

    // Sanity check the length of table
    if (length < sizeof(mpct_hdr_t)) 
    {
        mmu_munmap(mpct_phys_base);
        return NULL;
    }

    // Remap the whole table based on said length
    u64 page_count = mmu_byte_to_page_count(length);
    mmu_mmapn(mpct_phys_base, 0, page_count, 0); // RO

    // Now do checksum
    if (!verify_8bit_checksum(valid_mpct, length))
    {
        mmu_munmapn(mpct_phys_base, &page_count);
        return NULL;
    }

    // Finally, check if the LAPIC address is sane, basically another sanity check
    if (!valid_mpct->lapic_addr)
    {
        mmu_munmapn(mpct_phys_base, &page_count);
        return NULL;
    }

    // Now, the 1.4-specific check
    if (valid_mpct->spec_revision == 4 && valid_mpct->ext_fields.ext_length > 0)
    {
        // Remap additional pages for the extended section
        usize ext_page_count = mmu_byte_to_page_count(valid_mpct->ext_fields.ext_length);
        mmu_mmapn(mpct_phys_base+length, 0, ext_page_count, 0); // RO

        // Compute checksum of extended section
        u8 sum = 0;
        u8* ext_ptr = (u8*)valid_mpct + length;
        for (u16 i = 0; i < valid_mpct->ext_fields.ext_length; i++) sum += ext_ptr[i];

        // Validate with ext_checksum field
        if ((sum + valid_mpct->ext_fields.ext_checksum) & 0xFF) 
        {
            u64 total_page_count = page_count + ext_page_count;
            mmu_munmapn(mpct_phys_base, &total_page_count);
            return NULL;
        }    
    }

    return valid_mpct;
}