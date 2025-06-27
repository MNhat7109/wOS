#include "ahci_info.h"
#include "ahci_utils.h"
#include "ahci_drv.h"
#include "../../stdint.h"
#include <stdbool.h>
#include "../../stdio.h"
#include "../../string/string.h"

// bool ahci_read_sectors(hba_port_t* port, 
//     u32 lba_lo, u32 lba_hi, u32 count, u16* out_buffer)
// {
//     port->interrupt_status = (u32)-1;
//     int spin = 0;
//     int slot = ahci_find_avl_cmd_slots(port);
//     if (slot == -1)
//     {
//         kprintf("AHCI: Out of command slots. Device is busy\n");
//         return false;
//     }

//     hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*)port->cmd_list_base_lo;
//     cmd_hdr += slot;
//     cmd_hdr->cmd_fis_length = sizeof(fis_reg_h2d_t)/sizeof(u32);
//     cmd_hdr->write = 0;
//     cmd_hdr->prdt_length = (u16)((count-1)>>4)+1;

//     hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)cmd_hdr->cmd_table_base_lo;
//     memset(cmd_table, 0, sizeof(hba_cmd_table_t)+(cmd_hdr->prdt_length-1)*sizeof(hba_prdt_entry_t));

//     u16 i;
//     for (i=0; i<cmd_hdr->prdt_length-1; i++)
//     {
//         cmd_table->prdt_entries[i].data_base_addr_lo = (u32)out_buffer;
//         cmd_table->prdt_entries[i].data_byte_cnt = 8*1024-1;
//         cmd_table->prdt_entries[i].interrupt = 1;
//         out_buffer += 4*1024;
//         count -= 16;
//     }

//     cmd_table->prdt_entries[i].data_base_addr_lo = (u32)out_buffer;
//     cmd_table->prdt_entries[i].data_byte_cnt = (count<<9)-1;
//     cmd_table->prdt_entries[i].interrupt = 1;
    
//     fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)(&cmd_table->cmd_fis);

//     cmd_fis->fis_type = FIS_TYPE_REG_H2D;
//     cmd_fis->cmd_ctl = 1;
//     cmd_fis->command = ATA_CMD_READ_DMA_EXT; // TODO
    
//     cmd_fis->lba0 = (u8)lba_lo;
//     cmd_fis->lba1 = (u8)(lba_lo>>8);
//     cmd_fis->lba2 = (u8)(lba_lo>>16);
//     cmd_fis->device = 1<<6;

//     // TODO
//     cmd_fis->lba3 = (u8)(lba_lo>>24);
//     cmd_fis->lba4 = (u8)(lba_hi);
//     cmd_fis->lba5 = (u8)(lba_hi>>8);

//     cmd_fis->count_lo = count & 0xFF;
//     cmd_fis->count_hi = (count>>8)&0xFF;

//     while ((port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) && spin < 1000000)
//     spin++;

//     if (spin == 1000000)
//     {
//         kprintf("AHCI: Port is hung!\n");
//         return false;
//     }

//     port->cmd_issue = 1<<slot;

//     while (1)
//     {
//         if ((port->cmd_issue & (1<<slot)) == 0) break;
//         if (port->interrupt_status & (1<<30))
//         {
//             kprintf("AHCI: Disk read error!\n");
//             return false;
//         }
//     }

//     if (port->interrupt_status & (1<<30))
//     {
//         kprintf("AHCI: Disk read error!\n");
//         return false;
//     }

//     return true;
// }

u32 ahci_read()
{
    if (!ahci_io_pack.receive) return 0;
    ahci_io_pack.pool_value = 0;
    ahci_io_pack.receive = false;
    u32 ret_val = 0;
    switch (ahci_io_pack.cmd_sig)
    {
    case AHCI_DRV_CMD_RECEIVE_PORT_CNT:
        ret_val = current_port_count;
        break;
    default:
        break;
    }
    ahci_io_pack.cmd_sig = AHCI_DRV_CMD_IDLE;
    return ret_val;
}
