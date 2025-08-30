#pragma once

#define APIC_BASE_MSR 0x1B
#define X2APIC_APICID_MSR 0x802

#define APIC_BASE_BSP_FLAG         (1ULL << 8)   // RO: this CPU is BSP
#define APIC_BASE_X2APIC_ENABLE    (1ULL << 10)  // x2APIC mode
#define APIC_BASE_APIC_GLOBAL_EN   (1ULL << 11)  // APIC globally enabled