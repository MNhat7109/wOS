#include "acpi_utils.h"
#include "acpi_defs.h"
#include "acpi.h"
#include "../../string/string.h"
#include "../../paging/paging.h"
#include "../../stdio.h"

acpi_sdt_hdr_t* acpi_find_table(struct acpi_driver_t* self, char* signature)
{
    for (u32 i = 0;i<self->__sdt_count;i++)
    {
        u64 sdtp;
        memcpy(&sdtp, (const void*)(self->__sdt_base+i*self->__ptr_size), self->__ptr_size);
        page_manager_map_memory(sdtp, sdtp);
        
        acpi_sdt_hdr_t* hdr = (acpi_sdt_hdr_t*)sdtp;
        if (memcmp(hdr->signature, signature, 4) == 0)
        {
            kprintf("ACPI: Found header with signature: %s, base: 0x%x\n", signature, hdr);
            return hdr;
        }
    }

    kprintf("ACPI: Header with signature: %s not found!\n", signature);
    return NULL;
}