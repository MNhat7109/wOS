#include <kernel/acpi.h>
#include <kernel/mmu_vmem.h>

#define ACPI_TYPE_RSDP 1
#define ACPI_TYPE_XSDP 2

typedef struct
{
    char signature[8];
    u8 checksum;
    char oem_id[6];
    u8 revision;
    u32 rsdt_addr;
} __attribute__((packed)) rsdp_t;

typedef struct
{
    rsdp_t rsdp;

    u32 length;
    u64 xsdt_addr;
    u8 ext_checksum;
    u8 reserved[3];
} __attribute__((packed)) xsdp_t;

static struct
{
    u8 ptr_stride;
    void* table_base;
    usize table_size;
    usize table_cnt;
} acpi_data;

int acpi_init(system_desc_ptr_t* sdp)
{
    if (!sdp) return -1;

    int status = 0;
    
    mmu_vmem_alloc(sdp, sizeof(system_desc_ptr_t), MMU_VMA_PHYS | MMU_VMA_R, NULL); 
    void* rxsdp_base = (void*)sdp->address, *rxsdt;
    
    // Map the RSDP/XSDP
    mmu_vmem_alloc(rxsdp_base, 4096, MMU_VMA_PHYS | MMU_VMA_R, NULL);

    // Save base of RSDT/XSDT
    switch (sdp->type)
    {
    case ACPI_TYPE_RSDP:
        acpi_data.ptr_stride = 4;
        rxsdt = ((rsdp_t*)rxsdp_base)->rsdt_addr;
        break;
    case ACPI_TYPE_XSDP:
        acpi_data.ptr_stride = 8;
        rxsdt = ((xsdp_t*)rxsdp_base)->xsdt_addr;
        break;
    default:
        status = -1;
        break;
    }

    mmu_vmem_alloc(rxsdt, 4096, MMU_VMA_PHYS | MMU_VMA_R, NULL);

    acpi_data.table_base = (void*)((u8*)rxsdt+sizeof(acpi_sdt_hdr_t));
    acpi_data.table_size = (usize)(((acpi_sdt_hdr_t*)rxsdt)->length-sizeof(acpi_sdt_hdr_t));
    acpi_data.table_cnt = acpi_data.table_size/acpi_data.ptr_stride;

    mmu_vmem_alloc(acpi_data.table_base, acpi_data.table_size, MMU_VMA_PHYS | MMU_VMA_R, NULL);

    // Unmap RSDP/XSDP
    mmu_vmem_free(rxsdt);
    mmu_vmem_free(rxsdp_base);
    mmu_vmem_free(sdp);
    
    return status;
}

acpi_sdt_hdr_t* acpi_get_table(char* signature);
void acpi_invalidate_table(acpi_sdt_hdr_t* tbl);