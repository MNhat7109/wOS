#include <devices/cpu/arch/x86/apic/lapic.h>
#include <devices/cpu/arch/x86/apic/apic_defs.h>
#include <devices/cpu/arch/x86/apic/lapic_defs.h>

#include <devices/cpu/arch/x86/global/cpu_defs_x86.h>
#include <devices/cpu/arch/x86/core/cpu_core_defs_x86.h>

#include <devices/cpu/cpu_core_utils.h>
#include <devices/cpu/cpu_defs.h>
#include <devices/cpu/cpu.h>
#include <devices/driver.h>

static struct
{
    const struct mmio_info_t* lapic_mmio;
    const cio_layer_t* lapic_cpu_io;
    bool x2apic;
} lapic_global_data;

static void lapic_write(u32 reg, u64 value);
static u64 lapic_read(u32 reg);

static int lapic_send_ipi(
    struct generic_driver_tree_node_t* self,
    const struct cpu_ipi_request_t* req
);
static int lapic_broadcast_ipi(
    struct generic_driver_tree_node_t* self,
    const struct cpu_ipi_request_t* req
);

usize lapic_get_base(
    struct generic_driver_tree_node_t* cpu_self
);

void lapic_global_init(
    struct generic_driver_tree_node_t* cpu_self
)
{
    x86_cpu_global_param_t* param = (x86_cpu_global_param_t*)cpu_self->additionals;

    if (!param)
    {
        driver_log_state(cpu_self, DRIVER_LOG_ERROR, "Parameter is empty\n");
        return;
    }

    if (param->global_cpu_flags.x2apic_on)
    {
        // x2APIC is supported, obtain the MSR instead.
        param->export_params.lx2apic_msr = X2APIC_APICID_MSR;
        lapic_global_data.lapic_cpu_io = param->export_params.cpu_io;
        lapic_global_data.x2apic = true;
    }
    else
    {
        // Obtain the LAPIC base
        usize lapic_base = lapic_get_base(cpu_self);
        
        // Set up resource
        struct generic_driver_resource_t* res_node = driver_request_resource(cpu_self, DRIVER_RES_TYPE_MMIO);
        driver_set_res_data(cpu_self, res_node, lapic_base, 4096, MMIO_FLAG_UNCACHEABLE);

        lapic_global_data.lapic_mmio = &res_node->resource.mmio_info;
        lapic_global_data.x2apic = false;
    }

    // Load IPI ops
    GET_DEV_OPS(cpu, cpu_self)->send_ipi = &lapic_send_ipi;
    GET_DEV_OPS(cpu, cpu_self)->broadcast_ipi = &lapic_broadcast_ipi;
}

void lapic_init(
    struct generic_driver_tree_node_t* cpu_core_self
)
{
    // Enable LAPIC, and set Spurious Interrupt to vector 0xFF
    lapic_write(LAPIC_REG_SVR, LAPIC_SVR_APIC | (0xFF<<0));

    // Enable all interrupts
    lapic_write(LAPIC_REG_TPR, 0);
}

void lapic_disable(
    struct generic_driver_tree_node_t* cpu_core_self
)
{
    // No op, just in case
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void lapic_send_eoi(u8 _reserved);
static u32 lapic_get_id();

static int cpu_core_send_self_ipi(
    struct generic_driver_tree_node_t* cpu_core_self,
    struct cpu_ipi_request_t* req
);
static u32 cpu_core_get_proc_id(
    struct generic_driver_tree_node_t* cpu_core_self
);

struct cpu_core_ops_t core_ops = {
    .send_eoi = &lapic_send_eoi,
    .get_arch_id = &lapic_get_id,
    .get_core_id = &cpu_core_get_proc_id,
    .send_self_ipi = &cpu_core_send_self_ipi
};

const struct cpu_core_ops_t* cpu_core_load_core_ops()
{
    return &core_ops;
}


///////////////////////////////////////////////////////

static void lapic_timer_init(u32 tick_count, u8 vector);
static void lapic_timer_countdown();
static u64 lapic_timer_get_freq(u64 elapsed_ticks, u64 multiplier);

struct cpu_timer_ops_t timer_ops = {
    .timer_init = &lapic_timer_init,
    .timer_get_freq = &lapic_timer_get_freq,
    .timer_countdown = &lapic_timer_countdown
};

const struct cpu_timer_ops_t* cpu_core_load_timer_ops()
{
    return &timer_ops;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void lapic_send_eoi(u8 _useless_reserved_for_arm64)
{
    (void)_useless_reserved_for_arm64;
    lapic_write(LAPIC_REG_EOI, 0);
}

static u32 lapic_get_id()
{
    return lapic_read(LAPIC_REG_ID);
}

static u32 cpu_core_get_proc_id(
    struct generic_driver_tree_node_t* cpu_core_self
)
{
    if (!cpu_core_self->additionals)
    {
        driver_log_state(cpu_core_self, DRIVER_LOG_ERROR, "Parameter is empty\n");
        return -1;
    }

    x86_cpu_core_param_t* param = (x86_cpu_core_param_t*)cpu_core_self->additionals;
    return param->proc_id;
}

static int cpu_core_send_self_ipi(
    struct generic_driver_tree_node_t* cpu_core_self,
    struct cpu_ipi_request_t* req
)
{
    if (!GET_DEV_OPS(cpu, cpu_core_self->parent)->broadcast_ipi)
    {
        driver_log_state(cpu_core_self, DRIVER_LOG_WARN, "Parent's ops has not been fully loaded\n");
        return 1;
    }

    if (!req)
    {
        driver_log_state(cpu_core_self, DRIVER_LOG_WARN, "IPI request param is required\n");
        return -1;
    }

    // Force self IPI sending
    req->dest_type = CPU_IPI_DEST_SELF;

    return GET_DEV_OPS(cpu, cpu_core_self->parent)
    ->broadcast_ipi(cpu_core_self->parent, req);
}

/////////////////////////////////////////////////////////

#define LAPIC_DIV_MODE(_m) (((_m)&0b11) | ((((_m)>>2)&0b11)<<3))

static void lapic_timer_init(u32 tick_count, u8 vector)
{
    u8 div_mode = LAPIC_DIV_MODE(LAPIC_TIMER_DIVIDE_16);

    lapic_write(LAPIC_REG_DIVCFG, div_mode);

    // Unmasked + Periodic
    lapic_write(LAPIC_REG_LVT, ((vector) | ((LAPIC_TIMER_MODE_PERIODIC&3)<<17)));

    lapic_write(LAPIC_REG_INITCNT, tick_count);
}

static void lapic_timer_countdown()
{
    u8 div_mode = LAPIC_DIV_MODE(LAPIC_TIMER_DIVIDE_16);

    lapic_write(LAPIC_REG_DIVCFG, div_mode);

    // Masked + One-shot
    lapic_write(LAPIC_REG_LVT, ((1<<16) | ((LAPIC_TIMER_MODE_ONE_SHOT&3)<<17)));

    // Reset initial count
    lapic_write(LAPIC_REG_INITCNT, (u32)-1);
}

static u64 lapic_timer_get_freq(u64 elapsed_ticks, u64 multiplier)
{
    u32 lapic_current_cnt = lapic_read(LAPIC_REG_CURRCNT);
    u32 lapic_elapsed = 0xFFFFFFFF - lapic_current_cnt;

    return (lapic_elapsed*multiplier)/elapsed_ticks;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void lapic_write(u32 reg, u64 value)
{
    if (lapic_global_data.x2apic)
    {
        lapic_global_data.lapic_cpu_io->wrmsr((LAPIC_MSR_BASE+(reg>>4)), value);
        return;
    }

    struct mmio_info_t* mmio_tool = lapic_global_data.lapic_mmio;
    mmio_tool->layer->writel(mmio_tool, reg, (u32)value);
}

static u64 lapic_read(u32 reg)
{
    if (lapic_global_data.x2apic)
        return lapic_global_data.lapic_cpu_io->rdmsr((LAPIC_MSR_BASE+(reg>>4)));

    struct mmio_info_t* mmio_tool = lapic_global_data.lapic_mmio;
    return (u32)mmio_tool->layer->readl(mmio_tool, reg);
}

///////////////////////////////////////////////////////

static int lapic_send_ipi(
    struct generic_driver_tree_node_t* self,
    const struct cpu_ipi_request_t* req
)
{
    if (!req)
    {
        driver_log_state(self, DRIVER_LOG_WARN, "IPI request param is required\n");
        return -1;
    }

    if (req->dest_type != CPU_IPI_DEST_CORE)
    {
        driver_log_state(self, DRIVER_LOG_WARN, "send_ipi() only supports sending to one dest core only\n");
        return -1;
    }

    if (!req->dest_core || !req->dest_core->additionals)
    {
        driver_log_state(self, DRIVER_LOG_WARN, "Dest core not specified\n");
        return -1;
    }

    x86_cpu_core_param_t* dest_core_param = (x86_cpu_core_param_t*)req->dest_core->additionals;

    // Fill up the form
    u32 icr_hi, icr_lo;

    icr_hi = dest_core_param->lapic_id << (lapic_global_data.x2apic?0:24);
    icr_lo = 
    (u32)req->vector | // Dest interrupt vector
    ((u32)(req->delivery_mode&15) << 8) // Delivery mode
    | ((u32)req->level << 14) |  // Level mode (Assert or De-assert)
    ((u32)req->trigger_mode << 15); // Trigger mode (Edge or Level)

    // PRETTY-LAME-AND-OBVIOUSLY-UNHOLY HACK: Prior, I modded lapic_write() and
    // lapic_read() to accept 64-bit value (for x2APIC support of course),
    // Now to the hack itself, **drum rolls...** I wrote both icr_lo and icr_hi 
    // to the LAPIC's ICR low register.
    // Why? 
    // In x2APIC mode, the lapic_write() function can grab both DWORDs and shove them into the MSR
    // (which is 0x800 + (0x300>>4) = 0x830, how "clever").
    // In xAPIC mode (the regular APIC mode), the said function can just discard the high DWORD, and
    // put it in the MMIO base. Now you'll be asking, then where will the high DWORD be then?
    // Easy, we can just check if only the APIC mode is on (x2apic == false), then we will write that
    // missing DWORD to the ICR high register. Pretty neat, eh?

    lapic_write(LAPIC_REG_ICR_LO, ((icr_lo) | ((u64)icr_hi<<32)));

    if (!lapic_global_data.x2apic)
        lapic_write(LAPIC_REG_ICR_HI, icr_hi);

    // Send pending check
    int spin=0;
    while (lapic_read(LAPIC_REG_ICR_LO) & LAPIC_ICR_STATUS_PENDING)
    {
        if (spin == 1000000)
        {
            driver_log_state(self, DRIVER_LOG_WARN, "LAPIC at dest core %u hung\n",
            dest_core_param->proc_id);
            return 1;
        }
        spin++;
    }

    return 0;
}

static int lapic_broadcast_ipi(
    struct generic_driver_tree_node_t* self,
    const struct cpu_ipi_request_t* req
)
{
    if (!req)
    {
        driver_log_state(self, DRIVER_LOG_WARN, "IPI request param is required\n");
        return -1;
    }

    if (req->dest_type == CPU_IPI_DEST_CORE)
    {
        driver_log_state(self, DRIVER_LOG_WARN, "broadcast_ipi() can't send IPIs to one core only. Use send_ipi(), jackass.\n");
        return -1;
    }

    // Fill up the form
    u32 icr_lo;

    // Destination field is stored in the high DWORD, but we won't use it.
    icr_lo = 
    (u32)req->vector | // Dest interrupt vector
    ((u32)(req->delivery_mode&15) << 8) | // Delivery mode
    ((u32)req->level << 14) |  // Level mode (Assert or De-assert)
    ((u32)req->trigger_mode << 15) | // Trigger mode (Edge or Level)
    ((u32)(req->dest_type&3) << 18); // Destination shorthand

    lapic_write(LAPIC_REG_ICR_LO, (icr_lo));

    // Send pending check
    int spin=0;
    while (lapic_read(LAPIC_REG_ICR_LO) & LAPIC_ICR_STATUS_PENDING)
    {
        if (spin == 1000000)
        {
            driver_log_state(self, DRIVER_LOG_WARN, "LAPIC hung\n");
            return 1;
        }
        spin++;
    }

    return 0;
}