#include "ahci.h"
#include "ahci_shared.h"
#include "ahci_interrupts.h"
#include "ahci_defs.h"
#include "ahci_hw.h"
#include "ahci_utils.h"
#include "ahci_device.h"
#include "ahci_detect.h"

#include "../pci/pci.h"
#include "../../paging/paging.h"
#include "../../memory/memory.h"
#include "../../string/string.h"

struct ahci_shared_t ahci_shared;

void ahci_config(struct generic_driver_t* driver)
{
    for (u32 i=0;i<ahci_shared.ctl_count;i++)
    {
        driver_log_state(driver, DRIVER_LOG_NOTICE, "At controller:"); // will add str-fmt later
        ahci_controller_t* ctl = &ahci_shared.controllers[i];
        if (!ctl->abar)
        {
            driver_log_state(driver, DRIVER_LOG_WARN, "Weird: Skipping controller...");
            continue;
        }
        
        driver_log_state(driver, DRIVER_LOG_NOTICE, "Probing features...");
        ahci_probe_features(ctl->abar);

        u32 cap_x = ctl->abar->host_capability_ext;

        if ((cap_x>>0)&1) // Check if BIOS/OS handoff is supported
        {
            driver_log_state(driver, DRIVER_LOG_NOTICE, "Performing BIOS/OS handoff...");
            if (!hba_perform_bios_os_handoff(ctl->abar))
            {
                driver_log_state(driver, DRIVER_LOG_ERROR, "Failed to perform the handoff");
                return;
            }
            driver_log_state(driver, DRIVER_LOG_NOTICE, "Handoff completed");
        }

        // Allocate memory for the device array
        ctl->devices = (ahci_dev_t*)memory_allocate(AHCI_CONFIG_MAX_PORT*sizeof(ahci_dev_t));

        hba_reset(ctl->abar);
        ahci_device_reset_all(driver, ctl);

        ahci_interrupt_setup(driver, ctl);

        driver_log_state(driver, DRIVER_LOG_NOTICE, "Probing ports...");

        ahci_device_detect(driver, ctl);

        for (u32 dev_idx = 0; dev_idx<ctl->device_count;dev_idx++)
        {
            ahci_dev_t* dev = &ctl->devices[dev_idx];
            if (!ahci_device_rebase(driver, dev))
            {
                driver_log_state(driver, DRIVER_LOG_WARN, "Failed to rebase device. Skipping...");
                ahci_device_clear_flag(dev, AHCI_DEV_CONNECTED);
                continue;
            }

            if (!ahci_device_identify(driver, dev))
            {
                driver_log_state(driver, DRIVER_LOG_WARN, "Failed to identify device. Skipping...");
                ahci_device_clear_flag(dev, AHCI_DEV_CONNECTED);
                continue;
            }
        }
    }
}

void ahci_disable(struct generic_driver_t* driver)
{
    u32 ctl_success_cnt = 0;
    for (u32 i=0;i<ahci_shared.ctl_count;i++)
    {
        driver_log_state(driver, DRIVER_LOG_NOTICE, "Disabling AHCI controller...");
        
        ahci_controller_t* ctl = &ahci_shared.controllers[i];
        if (!ctl->abar)
        {
            driver_log_state(driver, DRIVER_LOG_WARN, "Weird: Missing or corrupted controller. Skipping controller...");
            continue;
        }

        // Disable interrupts
        ahci_interrupt_disable(driver, ctl);

        // Nuke the ports
        u32 success_rate=0;
        for (u32 dev_idx=0; dev_idx<ctl->device_count; dev_idx++)
        {
            ahci_dev_t* dev = &ctl->devices[dev_idx];
            bool nuke_status = ahci_device_nuke(driver, dev);
            if (!nuke_status)
            {
                driver_log_state(driver,DRIVER_LOG_WARN, "Weird: Device is not ready. Skipping device...");
                continue;
            }
            success_rate++;
        }
        
        if (success_rate!=ctl->device_count)
        {
            driver_log_state(driver, DRIVER_LOG_WARN, "All devices are not ready. Skipping controller...");
            continue;
        }

        // Disable AHCI mode
        ctl->abar->global_host_ctl &= ~HBA_GHC_AE;

        // Reset the ABAR
        hba_reset(ctl->abar);

        // Free up ABAR
        u32 j;
        for (j=0;j<AHCI_CONFIG_MAX_ABAR_PAGE;j++)
        {
            u32 base = page_manager_unmap_memory((u32)ctl->abar+j*PAGE_SIZE);
            if (!base)
            {
                driver_log_state(driver, DRIVER_LOG_WARN, "Cannot unmap ABAR. Skipping controller...");
                break;
            }
        }
        if (j<AHCI_CONFIG_MAX_ABAR_PAGE) continue;

        ctl_success_cnt++;
    }

    if (ctl_success_cnt!=ahci_shared.ctl_count)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "All controllers are not ready. Cannot continue.");
        return;
    }
    
    // Zero everything out
    for (u32 i=0;i<ahci_shared.ctl_count;i++)
    {
        ahci_controller_t* ctl = &ahci_shared.controllers[i];
        memset((void*)ctl->devices, 0, AHCI_CONFIG_MAX_PORT*sizeof(ahci_dev_t));
        memory_free((void*)ctl->devices);
    }
    memset((void*)ahci_shared.controllers, 0, AHCI_CONFIG_MAX_CTL*sizeof(ahci_controller_t));
    memory_free((void*)ahci_shared.controllers);
}

void ahci_probe(struct generic_driver_t* driver)
{
    ahci_shared.pci_dev = (struct pci_driver_t*)driver_get("PCI");
    if (!ahci_shared.pci_dev)
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "PCI driver not found");
        return;
    }
    if (!driver_run((struct generic_driver_t*)ahci_shared.pci_dev))
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "PCI driver failed to start");
        return;
    }

    ahci_shared.controllers = 
    (ahci_controller_t*)memory_allocate(AHCI_CONFIG_MAX_CTL*sizeof(ahci_controller_t));
    memset(ahci_shared.controllers, 0, AHCI_CONFIG_MAX_CTL*sizeof(ahci_controller_t));

    if (!ahci_detect_controllers())
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "No controllers detected");
        return;
    }
}


void ahci_export_dev(struct generic_driver_t* driver, ahci_export_cb_t callback)
{
    driver_log_state(driver, DRIVER_LOG_NOTICE, "Now exporting devices to higher-level API...");
    if (!callback)
    {
        driver_log_state(driver, DRIVER_LOG_WARN, "Client callback not specified. Ignoring...");
        return;
    }

    callback(ahci_shared.controllers, ahci_shared.ctl_count);

    driver_log_state(driver, DRIVER_LOG_NOTICE, "Export complete.");
}

struct ahci_driver_t ahci_dev_driver = {
    .driver_hdr = {
        .name = "AHCI",
        .config = &ahci_config,
        .probe = &ahci_probe,
        .disable = &ahci_disable,
        .state = DRIVER_STATE_UNPROBED,
    },
    .export_dev = &ahci_export_dev,
    .ioctl = &ahci_handle_ioctl
};

const struct generic_driver_t* ahci_get_driver()
{
    return (struct generic_driver_t*)&ahci_dev_driver;
}