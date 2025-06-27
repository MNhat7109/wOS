#include "ahci_info.h"
#include "ahci_drv.h"
#include "ahci_utils.h"
#include "ahci_interrupts.h"
#include "../../paging/paging.h"
#include <stdbool.h>
#include "../../stdint.h"
#include "../../stdio.h"
#include "../../string/string.h"
#include "../../hal/interrupt/irq.h"
#include "../../timer/timer.h"
#include "../../memory/memory.h"

ahci_port_t ports[AHCI_MAX_PORT_ENTRIES];
hba_memory_t* abar;
int current_port_count = 0;
bool emb_support=false, pm_support=false;
bool boh_support=false;
int max_cmd_slot_cnt;
int max_port_cnt = 0;
generic_driver_io_t ahci_io_pack = 
{
    .cmd_sig = AHCI_DRV_CMD_IDLE,
    .pool_value = 0,
    .receive = false,
    .send = false
};


static void port_rebase(hba_port_t* port)
{
    ahci_port_shutdown(port);
    // Sets up command list

    void* cmd_list_base = (void*)page_alloc_request();
    memset(cmd_list_base, 0, 1024);
    port->cmd_list_base_lo = (u32)cmd_list_base;
    port->cmd_list_base_hi = 0;

    // Sets up FIS
    void* fis_base = (void*)page_alloc_request();
    memset(fis_base, 0, 256);
    port->fis_base_lo = (u32)fis_base;
    port->fis_base_hi = 0;

    // Command header
    hba_cmd_hdr_t* cmd_hdr_base = (hba_cmd_hdr_t*)(port->cmd_list_base_lo);
    for (int i=0;i<max_cmd_slot_cnt;i++)
    {
        cmd_hdr_base[i].prdt_length = 8;

        void* cmd_table_base = (void*)page_alloc_request();
        memset(cmd_table_base, 0, 4096);
        cmd_hdr_base[i].cmd_table_base_lo = (u32)cmd_table_base;
        cmd_hdr_base[i].cmd_table_base_hi = 0;
    }

    ahci_port_startup(port);
    port->interrupt_enable = (u32)-1;
}

void ahci_probe_features()
{
    u32 cap = abar->host_capability;
    u32 cap_x = abar->host_capability_ext;
    
    kprintf("%x, %x\n", cap, cap_x);

    kprintf("AHCI Capability: Number of ports: %u\n"
        , (cap>>0)&0x1F);
    max_port_cnt = (cap>>0)&0x1F;
    
    kprintf("AHCI Capability: eSATA Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>5)&1]);
        kprintf("AHCI Capability: Enclosure Bridge Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>6)&1]);
    emb_support = (cap>>6)&1;

    kprintf("AHCI Capability: Command Completion Coalescing Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>7)&1]);
    kprintf("AHCI Capability: Number of cmd slots: %u\n"
        , (cap>>8)&0x1F);
        max_cmd_slot_cnt = (cap>>8)&0x1F;
        
    kprintf("AHCI Capability: Partial State Capable: %s\n"
    , (const char*[]){"no", "yes"}[(cap>>13)&1]);
    kprintf("AHCI Capability: Slumber State Capable: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>14)&1]);
    kprintf("AHCI Capability: PIO Multiple DRQ Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>15)&1]);
        kprintf("AHCI Capability: FIS-based Switching Support: %s\n"
            , (const char*[]){"no", "yes"}[(cap>>16)&1]);
            kprintf("AHCI Capability: Port Multiplier Support: %s\n"
                , (const char*[]){"no", "yes"}[(cap>>17)&1]);
                pm_support = (cap>>17)&1;
                
    kprintf("AHCI Capability: AHCI mode only: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>18)&1]);
    
        const char* iss[16] = {"no",};
        iss[1] = "Gen 1.0"; iss[2] = "Gen 2.0"; iss[3] = "Gen 3.0"; 
    kprintf("AHCI Capability: Interface Speed Support: %s\n"
    , iss[(cap>>20)&0xF]);
    kprintf("AHCI Capability: Cmd List Override Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>24)&1]);
    kprintf("AHCI Capability: Activity LED Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>25)&1]);
        kprintf("AHCI Capability: Aggressive Link Power Management Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap>>26)&1]);
    kprintf("AHCI Capability: Staggered Spin-up Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>27)&1]);
        kprintf("AHCI Capability: Mechanical Presence Switch Support: %s\n"
            , (const char*[]){"no", "yes"}[(cap>>28)&1]);
            kprintf("AHCI Capability: PxSNTF Register Support: %s\n"
                , (const char*[]){"no", "yes"}[(cap>>29)&1]);
    kprintf("AHCI Capability: Native Cmd Queuing Support: %s\n"
        , (const char*[]){"no", "yes"}[(cap>>30)&1]);
        kprintf("AHCI Capability: 64-bit DMA Support: %s\n"
            , (const char*[]){"no", "yes"}[(cap>>31)&1]);

            kprintf("AHCI Capability Extended: BIOS/OS Handoff Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap_x>>0)&1]);
    boh_support = (cap_x>>0)&1;
    
    kprintf("AHCI Capability Extended: NVMHCI Present: %s\n"
        , (const char*[]){"no", "yes"}[(cap_x>>1)&1]);
        kprintf("AHCI Capability Extended: Auto Partial to Slumber Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap_x>>2)&1]);
    kprintf("AHCI Capability Extended: Device Sleep Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap_x>>3)&1]);
    kprintf("AHCI Capability Extended: Aggressive Device Sleep Support: %s\n"
    , (const char*[]){"no", "yes"}[(cap_x>>4)&1]);
    kprintf("AHCI Capability Extended: Device Sleep Entrance from Slumber only: %s\n"
        , (const char*[]){"no", "yes"}[(cap_x>>5)&1]);
}

u16 identify_buffer[256];
bool ahci_get_device_attributes(ahci_port_t* port)
{
    kprintf("%x\n", &identify_buffer);
    
    if (!ahci_get_port_attributes(port, identify_buffer))
    {
        kprintf("AHCI: Failed to identify device at port no. %d!\n", port->num);
        return false;
    }
    kprintf("Yo\n");
    
    port->lba_mode = (identify_buffer[49] >> 9)&1;
    port->is_lba48 = (identify_buffer[83] >> 10)&1;
    return true;
}

void debug(registers_t* regs)
{
    kprintf("Heyyyy:3\n");
    // pic_driver->write(DRIVER_CMD, PIC_DRV_CMD_SEND_EOI);
    // pic_driver->write(DRIVER_DATA, interrupt_line);
}

void ahci_config()
{
    _x86_disable_interrupt();
    u32 abar_virt = page_alloc_request();
    page_manager_map_memory(abar_virt, abar_phys);
    abar = (hba_memory_t*)abar_virt;
    kprintf("AHCI: Probing features...\n");
    ahci_probe_features();
    
    if (boh_support)
    {
        kprintf("AHCI: Performing BIOS/OS handoff...");
        ahci_bios_os_handoff();
        kprintf("ok\n");
    }
    kprintf("%x\n", abar_virt);
    ahci_hba_reset();
    abar->global_host_ctl |= (HBA_GHC_AE|HBA_GHC_IE);
    kprintf("AHCI: Probing ports...\n");
    memset(ports, 0, AHCI_MAX_PORT_ENTRIES*sizeof(ahci_port_t));
    ahci_probe_port();
    // IRQ_reg_handler(interrupt_line, debug);
    // pic_driver->write(DRIVER_CMD, PIC_DRV_CMD_SEND_UNMASK);
    // pic_driver->write(DRIVER_DATA, interrupt_line);
    _x86_enable_interrupt();
    for (int i=0;i<current_port_count;i++)
    {
        port_rebase(ports[i].hba_port);
        
        ahci_get_device_attributes(&ports[i]);  
    }
    // kprintf("%x\n", abar_virt);
}