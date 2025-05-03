#include "ide.h"
#include "../pci/pci.h"
#include "../io/io.h"
#include "../stdio.h"
#include "../timer/pit.h"


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

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

// Directions:
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

struct IDE_channel_reg_t channels[2];
struct ide_device_t ide_devices[4];

volatile u8* ide_buf=(volatile u8*)0x20000;
u32 BAR0,BAR1,BAR2,BAR3,BAR4;
bool read_native_prm = false, read_native_snd = false;

static bool IDE_detect(pci_dev_t* dev)
{
    u16 class = (dev->type>>8)&0xFF, 
    subclass = (dev->type)&0xFF;
    if (class != 0x01 || subclass != 0x01) return false;
    kprintf("IDE: Found controller\n");

    u16 prog_if = (PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x8)>>8) & 0xFF;

    if (prog_if & (1<<0)) read_native_prm = true;
    if (prog_if & (1<<2)) read_native_snd = true;

    if (read_native_prm)
    {
        BAR0=(u32)(PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x10)
        |PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x10+2));
        BAR1=(u32)(PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x14)
        |PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x14+2));
    }
    if (read_native_snd)
    {
        BAR2=(u32)(PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x18)
        |PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x18+2));
        BAR3=(u32)(PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x1C)
        |PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x1C+2));
    }
    BAR4=(u32)(PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x20)
    |PCI_config_read_word(dev->bus, dev->slot, dev->func, 0x20+2));
    return true;
}

void IDE_write_reg(u8 channel, u8 reg, u8 value)
{
    if (reg > 0x07 && reg < 0x0C)
        IDE_write_reg(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    if (reg < 0x08)
        outb(channels[channel].base + reg - 0x00, value);
    else if (reg < 0x0C)
        outb(channels[channel].base + reg - 0x06, value);
    else if (reg < 0x0E)
        outb(channels[channel].ctrl + reg - 0x0A, value);
    else if (reg < 0x16)
        outb(channels[channel].bmide + reg - 0x0E, value);
    if (reg > 0x07 && reg < 0x0C)
        IDE_write_reg(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

u8 IDE_read_reg(u8 channel, u8 reg)
{
    u8 result;
    if (reg > 0x07 && reg < 0x0C)
        IDE_write_reg(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    if (reg < 0x08)
        result = inb(channels[channel].base + reg - 0x00);
    else if (reg < 0x0C)
        result = inb(channels[channel].base + reg - 0x06);
    else if (reg < 0x0E)
        result = inb(channels[channel].ctrl + reg - 0x0A);
    else if (reg < 0x16)
        result = inb(channels[channel].bmide + reg - 0x0E);
    if (reg > 0x07 && reg < 0x0C)
        IDE_write_reg(channel, ATA_REG_CONTROL, channels[channel].nIEN);
    return result;
}

static void IDE_read_buffer(u8 channel, u8 reg, void* buffer, u32 count)
{
    if (reg > 0x07 && reg < 0x0C)
        IDE_write_reg(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    if (reg < 0x08)
        insl(channels[channel].base + reg - 0x00, buffer, count);
    else if (reg < 0x0C)
        insl(channels[channel].base + reg - 0x06, buffer, count);
    else if (reg < 0x0E)
        insl(channels[channel].ctrl + reg - 0x0A, buffer, count);
    else if (reg < 0x16)
        insl(channels[channel].bmide + reg - 0x0E, buffer, count);
    if (reg > 0x07 && reg < 0x0C)
        IDE_write_reg(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

u8 IDE_poll(u8 channel, u32 advanced)
{
    // Read the Alternate Status port 4 times.
    // As each loop costs us 100ns, we need to loop them four times
    // in order for BSY bit to be set, as it will only be set after 400ns.
    for (int i=0;i<4;i++)
        IDE_read_reg(channel, ATA_REG_ALTSTATUS);
    
    // Then, wait for BSY to be cleared.
    while (IDE_read_reg(channel, ATA_REG_STATUS)&ATA_SR_BSY);

    if (advanced)
    {
        u8 state = IDE_read_reg(channel, ATA_REG_STATUS);

        // Is there an error?
        if (state&ATA_SR_ERR) return 2;

        // Is there a device fault?
        if (state&ATA_SR_DF) return 1;

        // BSY = 0, DF = 0, ERR = 0. Is the DRQ bit set?
        if ((state&ATA_SR_DRQ)==0) return 3; // Should be set.
    }
    return 0;
}

u8 IDE_panic(u32 drive, u8 err)
{
    switch (err)
    {
    case 0:
        break;
    default:
    {       
        kprintf("IDE: ");
        switch (err)
        {
        case 1:
            kprintf("Device Fault\n");
            err = 7;
            break;
        case 2:
        {
            u8 st = 
            IDE_read_reg(ide_devices[drive].channel, ATA_REG_ERROR);
            if (st&ATA_ER_AMNF)
            {
                kprintf("No Address Mark Found\n");
                err=7;
            }
            if (st&ATA_ER_TK0NF)
            {
                kprintf("No Media or Media Error\n");
                err=3;
            }
            if (st&ATA_ER_ABRT)
            {
                kprintf("Commoand Aborted\n");
                err=20;
            }
            if (st&ATA_ER_MCR)
            {
                kprintf("No Media or Media Error\n");
                err=3;
            }
            if (st&ATA_ER_IDNF)
            {
                kprintf("ID Mark Not Found\n");
                err=21;
            }
            if (st&ATA_ER_MC)
            {
                kprintf("No Media or Media Error\n");
                err=3;
            }
            if (st&ATA_ER_UNC)
            {
                kprintf("Uncorrectable Data Error\n");
                err=22;
            }
            if (st&ATA_ER_BBK)
            {
                kprintf("Bad Sector\n");
                err=13;
            }
            break;
        }
        case 3:
            kprintf("Reads Nothing\n"); err = 23;
            break;
        case 4:
            kprintf("Write Protected\n"); err = 8;
            break;
        }
        kprintf("Drive: %s %s, Model: %s\n", 
            (const char*[]){"Primary", "Secondary"}[ide_devices[drive].channel],
            (const char*[]){"Master", "Slave"}[ide_devices[drive].drive],
            ide_devices[drive].model
        );
        break;
    }    
    }
    return err;
}

static void IDE_setup(u32 BAR0, u32 BAR1, u32 BAR2, u32 BAR3, u32 BAR4)
{
    int i, j, k, cnt=0;
    
    // Set IO Ports
    channels[ATA_PRIMARY  ].base  = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
    channels[ATA_PRIMARY  ].ctrl  = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
    channels[ATA_SECONDARY].base  = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
    channels[ATA_SECONDARY].ctrl  = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
    channels[ATA_PRIMARY  ].bmide = (BAR4 & 0xFFFFFFFC) + 0; // Bus Master IDE
    channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8; // Bus Master IDE

    // Disable IRQs
    IDE_write_reg(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    IDE_write_reg(ATA_SECONDARY, ATA_REG_CONTROL, 2);

    // Detect ATA-ATAPI devices
    for (i=0;i<2;i++)
        for (j=0;j<2;j++)
        {
            u8 err = 0, type = IDE_ATA, status;
            ide_devices[cnt]._reserved = 0;
    
            IDE_write_reg(i, ATA_REG_HDDEVSEL, 0xA0 | (j<<4));
            sleep(1);
            IDE_write_reg(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            sleep(1);
    
            // Poll.
            if (!IDE_read_reg(i, ATA_REG_STATUS)) continue;
    
            while (true)
            {
                status = IDE_read_reg(i, ATA_REG_STATUS);
                if ((status&ATA_SR_ERR))
                {
                    err=1; break;
                }
                if (!(status&ATA_SR_BSY)&&(status&ATA_SR_DRQ)) break;
            }
            
            if (err)
            {
                u8 cl = IDE_read_reg(i, ATA_REG_LBA1);
                u8 ch = IDE_read_reg(i, ATA_REG_LBA2);

                if (cl==0x14&&ch==0xEB) type=IDE_ATAPI;
                else if (cl==0x69&&ch==0x96) type=IDE_ATAPI;
                else continue;

                IDE_write_reg(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                sleep(1);
            }
            
            // Read Identification Space.
            IDE_read_buffer(i, ATA_REG_DATA, (void*)ide_buf, 128);
            
            // Read Drive Params
            ide_devices[cnt]._reserved =1;
            ide_devices[cnt].type =type;
            ide_devices[cnt].channel=i;
            ide_devices[cnt].drive=j;
            ide_devices[cnt].signature=*((u16*)(ide_buf+ATA_IDENT_DEVICETYPE));
            ide_devices[cnt].capabilities=*((u16*)(ide_buf+ATA_IDENT_CAPABILITIES));
            ide_devices[cnt].cmd_sets=*((u16*)(ide_buf+ATA_IDENT_COMMANDSETS));

            // Get Size. Does the Device use LBA or CHS?
            if (ide_devices[cnt].cmd_sets & (1<<26))
                ide_devices[cnt].size = *((u32*)(ide_buf+ATA_IDENT_MAX_LBA_EXT));
            else
                ide_devices[cnt].size = *((u32*)(ide_buf+ATA_IDENT_MAX_LBA));
            
            for (k=0;k<40;k+=2)
            {
                ide_devices[cnt].model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
                ide_devices[cnt].model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
            }
            
            ide_devices[cnt].model[40] = '\0';  
            cnt++;

            for (int l=0;l<4;l++)
                if (ide_devices[l]._reserved)
                {
                    kprintf("IDE: Drive %s %s, type %s found. Model: %s\n",
                    (const char*[]){"Primary", "Secondary"}[ide_devices[l].channel],
                    (const char*[]){"Master", "Slave"}[ide_devices[l].drive],
                    (const char*[]){"ATA", "ATAPI"}[ide_devices[l].type],
                    ide_devices[l].model);
                }
        } 
}
bool IDE_init()
{
    bool PCI_status = PCI_scan(IDE_detect);

    if (!PCI_status)
    {
        kprintf("IDE: PCI controller not found! Using default values....\n");
        BAR0=0x1F0; BAR1=0x3F6; BAR2=0x170; BAR3=0x376;
        BAR4=0;
    }

    IDE_setup(BAR0, BAR1, BAR2, BAR3, BAR4);
    return true;
}