#pragma once
#include <libk/stdint.h>
#include <stdbool.h>

struct cpu_core_ops_t;
struct cpu_timer_ops_t;

const struct cpu_timer_ops_t* cpu_core_load_timer_ops();
const struct cpu_core_ops_t* cpu_core_load_core_ops();
