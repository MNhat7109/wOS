#pragma once
#include <stdbool.h>

struct hba_memory_t;
typedef struct hba_memory_t hba_memory_t;

struct generic_driver_t;

void ahci_probe_features(hba_memory_t* abar);
bool ahci_detect_controllers();