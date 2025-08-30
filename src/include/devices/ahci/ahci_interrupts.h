#pragma once

struct hba_memory_t;
typedef struct hba_memory_t hba_memory_t;

struct ahci_controller_t;
typedef struct ahci_controller_t ahci_controller_t;

struct generic_driver_t;

void ahci_interrupt_setup(struct generic_driver_t* driver, ahci_controller_t* controller);
void ahci_interrupt_disable(struct generic_driver_t* driver, ahci_controller_t* controller);