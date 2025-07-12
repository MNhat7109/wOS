#pragma once
#include "../../stdint.h"
#include "../driver.h"
#include <stdbool.h>

struct hba_memory_t;
typedef struct hba_memory_t hba_memory_t;

void ahci_probe_features(hba_memory_t* abar);
bool ahci_probe(struct generic_driver_t* driver);