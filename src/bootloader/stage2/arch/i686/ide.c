#include "../../drivers/ide.h"
#include "../../drivers/ata_defs.h"
#include "../../drivers/pci.h"
#include "../../errno.h"
#include "../../stdio.h"
#include "../../drivers/timer.h"
#include "io.h"

#define MODULE_IDE "IDE"

#define      ATAPI_CMD_READ       0xA8
#define      ATAPI_CMD_EJECT      0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_PRIMARY_NATIVE (1<<1)
#define IDE_SECONDARY_NATIVE (1<<2)

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

// Directions:
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

static struct
{
    u8 ctrl_count;
    ide_controller_t ide_controllers[8];
} ide_data;

bool ide_pci_detect(pci_device_t* device);
void ide_setup_devices(ide_controller_t* ctrl);

int ide_init()
{
    pci_device_t pci_ide;
    if (!pci_scan(&pci_ide, ide_pci_detect)) return -ENODEV;

    //kdebugf(DEBUG_INFO, MODULE_IDE, " Found %u controllers:\n", ide_data.ctrl_count);
    for (int i=0;i<ide_data.ctrl_count;i++)
    {
        ide_controller_t* current_ctrler = &ide_data.ide_controllers[i];
        kdebugf(DEBUG_INFO, MODULE_IDE, " On controller %d: \n", i);
        ide_setup_devices(current_ctrler);
    }
    return 0;
}

int ide_export_controller(int* pos, ide_controller_t** out)
{
    if (*pos >=ide_data.ctrl_count) return -1;

    *out = &ide_data.ide_controllers[(*pos)++];
    return 0;
}

void ide_write_reg(ide_controller_t* ctrl, u8 channel, u8 reg, u8 value)
{
    if (reg > 0x07 && reg < 0x0C)
        ide_write_reg(ctrl, channel, ATA_REG_CONTROL, 0x80 | ctrl->channels[channel].nIEN);
    if (reg < 0x08)
        outb(ctrl->channels[channel].base + reg - 0x00, value);
    else if (reg < 0x0C)
        outb(ctrl->channels[channel].base + reg - 0x06, value);
    else if (reg < 0x0E)
        outb(ctrl->channels[channel].ctrl + reg - 0x0A, value);
    else if (reg < 0x16)
        outb(ctrl->channels[channel].bmide + reg - 0x0E, value);
    if (reg > 0x07 && reg < 0x0C)
        ide_write_reg(ctrl, channel, ATA_REG_CONTROL, ctrl->channels[channel].nIEN);
}

u8 ide_read_reg(ide_controller_t* ctrl, u8 channel, u8 reg)
{
    u8 result;
    if (reg > 0x07 && reg < 0x0C)
        ide_write_reg(ctrl, channel, ATA_REG_CONTROL, 0x80 | ctrl->channels[channel].nIEN);
    if (reg < 0x08)
        result = inb(ctrl->channels[channel].base + reg - 0x00);
    else if (reg < 0x0C)
        result = inb(ctrl->channels[channel].base + reg - 0x06);
    else if (reg < 0x0E)
        result = inb(ctrl->channels[channel].ctrl + reg - 0x0A);
    else if (reg < 0x16)
        result = inb(ctrl->channels[channel].bmide + reg - 0x0E);
    if (reg > 0x07 && reg < 0x0C)
        ide_write_reg(ctrl, channel, ATA_REG_CONTROL, ctrl->channels[channel].nIEN);
    return result;
}

void ide_read_buffer(ide_controller_t* ctrl, u8 channel, u8 reg, void* buffer, u32 count)
{
    if (reg > 0x07 && reg < 0x0C)
        ide_write_reg(ctrl, channel, ATA_REG_CONTROL, 0x80 | ctrl->channels[channel].nIEN);
    if (reg < 0x08)
        insl(ctrl->channels[channel].base + reg - 0x00, buffer, count);
    else if (reg < 0x0C)
        insl(ctrl->channels[channel].base + reg - 0x06, buffer, count);
    else if (reg < 0x0E)
        insl(ctrl->channels[channel].ctrl + reg - 0x0A, buffer, count);
    else if (reg < 0x16)
        insl(ctrl->channels[channel].bmide + reg - 0x0E, buffer, count);
    if (reg > 0x07 && reg < 0x0C)
        ide_write_reg(ctrl, channel, ATA_REG_CONTROL, ctrl->channels[channel].nIEN);
}

int ide_error_check(ide_controller_t* ctrl, u32 drive, u8 initial_error)
{
    u8 err = initial_error;
    switch (err)
    {
        case 0: break;
        default:
        {
            kdebugf(DEBUG_INFO, MODULE_IDE, " ");
            switch (err)
            {
                case 1:
                    kprintf("Device Fault\n");
                    err=7;
                    break;
                case 2:
                {
                    u8 additional_errstatus = ide_read_reg(ctrl, ctrl->ide_devices[drive].channel, ATA_REG_ERROR);

                    if (additional_errstatus&ATA_ER_AMNF)
                    {
                        kprintf("No Address Mark Found\n");
                        err=7;
                    }
                    if ((additional_errstatus&ATA_ER_TK0NF) || 
                    (additional_errstatus&ATA_ER_MCR) ||
                    (additional_errstatus&ATA_ER_MC))
                    {
                        kprintf("No Media or Media Error\n");
                        err=3;
                    }
                    if ((additional_errstatus&ATA_ER_ABRT))
                    {
                        kprintf("Command Aborted\n");
                        err=20;
                    }
                    if (additional_errstatus&ATA_ER_IDNF)
                    {
                        kprintf("ID Mark Not Found\n");
                        err=21;
                    }
                    if (additional_errstatus&ATA_ER_UNC)
                    {
                        kprintf("Uncorrectable Data Error\n");
                        err=22;
                    }
                    if (additional_errstatus&ATA_ER_BBK)
                    {
                        kprintf("Bad sector\n");
                        err=13;
                    }
                    break;
                }
                case 3:
                    kprintf("Reads Nothing\n"); err=23;
                    break;
                case 4:
                    kprintf("Write Protected\n"); err=8;
                    break;
            }
            kprintf("Drive: %s %s, Model: %s\n", 
                (const char*[]){"Primary", "Secondary"}[ctrl->ide_devices[drive].channel],
                (const char*[]){"Master", "Slave"}[ctrl->ide_devices[drive].drive],
                ctrl->ide_devices[drive].model
            );
            kprintf("Error number: %u\n", err);
            break;
        }
    }
    return -ECHECKFAIL;
}

int ide_poll(ide_controller_t* ctrl, u8 channel, u8 advanced)
{
    for (int i=0; i<4; i++) ide_read_reg(ctrl, channel, ATA_REG_ALTSTATUS);

    int timeout = 0;
    while (ide_read_reg(ctrl, channel, ATA_REG_STATUS)&ATA_SR_BSY)
    {
        sleep(1); timeout++;
        if (timeout > 1000) return -EDEVHUNG;
    }

    if (advanced)
    {
        u8 status = ide_read_reg(ctrl, channel, ATA_REG_STATUS);

        if (status&ATA_SR_ERR) return -EDEVFAULT;
        if (status&ATA_SR_DF) return -EDEVFAULT;
        if (!(status&ATA_SR_DRQ)) return -EDEVFAULT;
    }

    return 0;
}

u8 ide_buf[512];
void ide_setup_devices(ide_controller_t* ctrl)
{
    int cnt=0;
    
    // Set IO Ports
    ctrl->channels[ATA_PRIMARY  ].base  = (ctrl->BAR0 & 0xFFFFFFFC) + 0x1F0 * (!ctrl->BAR0);
    ctrl->channels[ATA_PRIMARY  ].ctrl  = (ctrl->BAR1 & 0xFFFFFFFC) + 0x3F6 * (!ctrl->BAR1);
    ctrl->channels[ATA_SECONDARY].base  = (ctrl->BAR2 & 0xFFFFFFFC) + 0x170 * (!ctrl->BAR2);
    ctrl->channels[ATA_SECONDARY].ctrl  = (ctrl->BAR3 & 0xFFFFFFFC) + 0x376 * (!ctrl->BAR3);
    ctrl->channels[ATA_PRIMARY  ].bmide = (ctrl->BAR4 & 0xFFFFFFFC) + 0; // Bus Master IDE
    ctrl->channels[ATA_SECONDARY].bmide = (ctrl->BAR4 & 0xFFFFFFFC) + 8; // Bus Master IDE

    // Disable IRQs
    ide_write_reg(ctrl, ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write_reg(ctrl, ATA_SECONDARY, ATA_REG_CONTROL, 2);

    // Detect ATA-ATAPI devices
    for (int i=0;i<2;i++)
        for (int j=0;j<2;j++)
        {
            u8 err = 0, type = IDE_ATA, status;
            ctrl->ide_devices[cnt]._reserved = 0;
    
            ide_write_reg(ctrl, i, ATA_REG_HDDEVSEL, 0xA0 | (j<<4));
            sleep(1);
            ide_write_reg(ctrl, i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            sleep(1);
    
            // Poll.
            if (!ide_read_reg(ctrl, i, ATA_REG_STATUS)) continue;
            
            int timeout=1000000;
            while (timeout--)
            {
                status = ide_read_reg(ctrl, i, ATA_REG_STATUS);
                if ((status&ATA_SR_ERR))
                {
                    err=1; break;
                }
                if (!(status&ATA_SR_BSY)&&(status&ATA_SR_DRQ)) break;
            }
            if (!timeout) continue;
            
            if (err)
            {
                u8 cl = ide_read_reg(ctrl, i, ATA_REG_LBA1);
                u8 ch = ide_read_reg(ctrl, i, ATA_REG_LBA2);

                if (cl==0x14&&ch==0xEB) type=IDE_ATAPI;
                else if (cl==0x69&&ch==0x96) type=IDE_ATAPI;
                else continue;

                ide_write_reg(ctrl, i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                sleep(1);
            }
            
            // Read Identification Space.
            ide_read_buffer(ctrl, i, ATA_REG_DATA, (void*)ide_buf, 128);
            
            // Read Drive Params
            ctrl->ide_devices[cnt]._reserved =1;
            ctrl->ide_devices[cnt].type =type;
            ctrl->ide_devices[cnt].channel=i;
            ctrl->ide_devices[cnt].drive=j;
            ctrl->ide_devices[cnt].signature=*((u16*)(ide_buf+ATA_IDENT_DEVICETYPE));
            ctrl->ide_devices[cnt].capabilities=*((u16*)(ide_buf+ATA_IDENT_CAPABILITIES));
            ctrl->ide_devices[cnt].cmd_sets=*((u16*)(ide_buf+ATA_IDENT_COMMANDSETS));

            // Get Size. Does the Device use LBA or CHS?
            if (ctrl->ide_devices[cnt].cmd_sets & (1<<26))
                ctrl->ide_devices[cnt].size = *((u32*)(ide_buf+ATA_IDENT_MAX_LBA_EXT));
            else
                ctrl->ide_devices[cnt].size = *((u32*)(ide_buf+ATA_IDENT_MAX_LBA));
            
            for (int k=0;k<40;k+=2)
            {
                ctrl->ide_devices[cnt].model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
                ctrl->ide_devices[cnt].model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
            }
            
            ctrl->ide_devices[cnt].model[40] = '\0';  
            cnt++;
        } 

    for (int l=0;l<4;l++)
        if (ctrl->ide_devices[l]._reserved)
        {
            kdebugf(DEBUG_INFO, MODULE_IDE, "Drive %s %s, type %s found. Model: %s\n",
            (const char*[]){"Primary", "Secondary"}[ctrl->ide_devices[l].channel],
            (const char*[]){"Master", "Slave"}[ctrl->ide_devices[l].drive],
            (const char*[]){"ATA", "ATAPI"}[ctrl->ide_devices[l].type],
            ctrl->ide_devices[l].model);
        }
}

bool ide_pci_detect(pci_device_t* device)
{
    u8 pos = ide_data.ctrl_count;
    ide_data.ide_controllers[pos].BAR0=
    ide_data.ide_controllers[pos].BAR1=
    ide_data.ide_controllers[pos].BAR2=
    ide_data.ide_controllers[pos].BAR3=
    ide_data.ide_controllers[pos].BAR4=0;

    u32 raw_off = pci_read(device, 0x8);
    u8 class = raw_off >> 24, sub = raw_off >> 16;

    if (class != 0x01 || sub != 0x01) return false;

    u8 prog_if = raw_off >> 8;
    
    if (prog_if & IDE_PRIMARY_NATIVE)
    {
        ide_data.ide_controllers[pos].BAR0 = pci_read(device, 0x10);    
        ide_data.ide_controllers[pos].BAR1 = pci_read(device, 0x14);    
    }
    
    if (prog_if & IDE_SECONDARY_NATIVE)
    {
        ide_data.ide_controllers[pos].BAR2 = pci_read(device, 0x18);    
        ide_data.ide_controllers[pos].BAR3 = pci_read(device, 0x1C);    
    }

    ide_data.ide_controllers[pos].BAR4 = pci_read(device, 0x20);
    ide_data.ide_controllers[pos].pci_ide = *device;

    ide_data.ctrl_count++;
    return true;
}