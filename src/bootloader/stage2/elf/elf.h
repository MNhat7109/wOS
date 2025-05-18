#pragma once
#include "../stdint.h"
#include <stdbool.h>

bool ELF_load_file(const char* path, void** entry_point);