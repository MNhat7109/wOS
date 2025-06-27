#pragma once
#include "ahci_info.h"

int ahci_find_avl_cmd_slots(ahci_port_t* port);
void ahci_bios_os_handoff();
void ahci_hba_reset();
void ahci_port_reset(hba_port_t* port);
void ahci_soft_reset(ahci_port_t* port);

void ahci_stop_cmd(hba_port_t* port, bool reset);
void ahci_stop_fis(hba_port_t* port, bool reset);
void ahci_port_shutdown(hba_port_t* port);

void ahci_start_cmd(hba_port_t* port, bool reset);
void ahci_start_fis(hba_port_t* port, bool reset);
void ahci_port_startup(hba_port_t* port);
void ahci_probe_port();

bool ahci_get_port_attributes(ahci_port_t* port, void* out_buffer);