#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

bool verify_8bit_checksum(void* ptr, usize len);