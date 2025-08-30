#include <devices/driver.h>
#include <devices/cpu/cpu_utils.h>

#include <devices/cpu/arch/x86/global/cpu_defs_x86.h>
#include <devices/cpu/arch/x86/global/cpu_utils_x86.h>

#include <devices/cpu/arch/x86/core/cpu_core_defs_x86.h>

#include <devices/cpu/arch/x86/apic/ioapic.h>
#include <devices/cpu/arch/x86/apic/ioapic_defs.h>

#include <libk/string.h>
#include <libk/utils/algs.h>

#define MAX_IOAPIC_ENTRIES 256
#define MAX_IRQ_MAP_ENTRIES 16

// MADT area

#define MADT_IOAPIC_FLAG_ACTIVE_LO (1<<1)
#define MADT_IOAPIC_FLAG_LEVEL_TRIGGERED (1<<3)

typedef struct
{
    madt_record_entry_hdr_t entry_hdr;
    u8 ioapic_id;
    u8 _reserved;
    u32 ioapic_addr;
    u32 gsi_base; // Global System Interrupt Base
} __attribute__((packed)) madt_record_ioapic_t;

typedef struct
{
    madt_record_entry_hdr_t entry_hdr;
    u8 bus_source;
    u8 irq_source;
    u32 gsi;
    u16 flags;
} __attribute__((packed)) madt_record_ioapic_iso_t;

typedef struct
{
    madt_record_entry_hdr_t entry_hdr;
    u8 nmi_source;
    u16 flags;
    u32 gsi;
} __attribute__((packed)) madt_ioapic_nmi_src_t;

// MPS area

#define MPCT_INT_ASSIGN_FLAG_ACTIVE_HI (1<<0)
#define MPCT_INT_ASSIGN_FLAG_ACTIVE_LO (3<<0)

#define MPCT_INT_ASSIGN_FLAG_EDGE_TRIGGERED (1<<2)
#define MPCT_INT_ASSIGN_FLAG_LEVEL_TRIGGERED (3<<2)

typedef struct
{
    u8 entry_type;
    u8 ioapic_id;
    u8 ioapic_version;
    u8 flags;
    u32 ioapic_base;
} __attribute__((packed)) mpct_ioapic_entry_t;

typedef struct
{
    u8 entry_type;
    u8 interrupt_type; // 0: INT (normal), 1: NMI, 2: SMI, 3: ExtINT (8259 PIC thing, eh)
    u16 flags; /*
    * Bit 0-1: Polarity (00): bus default, (01): active hi, (11): active lo
    * Bit 2-3: Trigger node (00): default, (01): edge, (11): level
    * Bit 4-15: Reserved
    */
    u8 src_bus_id;
    u8 src_bus_irq;
    u8 dst_ioapic_id;
    u8 dst_ioapic_intin;
} __attribute__((packed)) mpct_int_assign_entry_t;

//

static struct
{
    usize ioapic_count;
    struct ioapic_info_t
    {
        u32 id, base, max_redirs, gsi_start;
        struct ioapic_nmi_t
        {
            u32 gsi;
            u16 flags;
        } *nmi_list;
        struct mmio_info_t* mmio_util;
    } __attribute__((packed)) ioapic_list[MAX_IOAPIC_ENTRIES];
    struct irq_map_entry_t
    {
        bool present;
        u8 irq_line;
        u16 flags;
        u32 gsi_num;
    } __attribute__((packed)) legacy_irq_map[MAX_IRQ_MAP_ENTRIES];
    x86_cpu_global_additional_param_t* saved_parms;
} ioapic_data;

static void ioapic_detect_with_madt(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* madt_node,
    void* record,
    void* ctx
);

static int ioapic_find_by_gsi(u8 gsi);
static void ioapic_sort_list();

static void ioapic_write(struct ioapic_info_t* entry, u8 reg, u32 value);
static u32 ioapic_read(struct ioapic_info_t* entry, u8 reg);

static void ioapic_detect_with_mps(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* mpct_node,
    void* record,
    void* ctx
);
static void legacy_irq_detect_with_madt(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* madt_node,
    void* record,
    void* ctx
);
static void nmi_detect_with_madt(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* madt_node,
    void* record,
    void* ctx
);
static void int_assign_with_mps(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* mpct_node,
    void* record,
    void* ctx
);

void ioapic_init(
    struct generic_driver_tree_node_t* cpu_self, 
    cpu_core_additional_param_t* required
)
{
    if (!required)
    {
        driver_log_state(cpu_self, DRIVER_LOG_ERROR,
        "Required param not found\n");
        return;
    }

    if (ioapic_data.ioapic_count > 0)
    {
        driver_log_state(cpu_self, DRIVER_LOG_NOTICE,
        "IOAPIC already initialized. Skipping...\n");
        return;
    }

    ioapic_data.ioapic_count = 0;
    memset(ioapic_data.ioapic_list, 0, sizeof(ioapic_data.ioapic_list));
    memset(ioapic_data.legacy_irq_map, 0, sizeof(ioapic_data.legacy_irq_map));

    if (ioapic_data.saved_parms->madt_driver_node &&
    ioapic_data.saved_parms->madt_driver_node->additionals)
    {
        cpu_scan_madt(
            cpu_self, 
            ioapic_data.saved_parms->madt_driver_node, 
            ioapic_detect_with_madt, NULL
        );

        if (ioapic_data.ioapic_count == 0)
        {
            driver_log_state(cpu_self, DRIVER_LOG_WARN,
                "Cannot find any IOAPICs under MADT\n");
            return;
        }

        ioapic_sort_list();

        cpu_scan_madt(
            cpu_self,
            ioapic_data.saved_parms->madt_driver_node,
            legacy_irq_detect_with_madt, NULL
        );

        cpu_scan_madt(
            cpu_self,
            ioapic_data.saved_parms->madt_driver_node,
            nmi_detect_with_madt, NULL
        );
    }
    else if (ioapic_data.saved_parms->mps_driver_node &&
    ioapic_data.saved_parms->mps_driver_node->additionals)
    {
        cpu_scan_mps(
            cpu_self,
            ioapic_data.saved_parms->mps_driver_node,
            ioapic_detect_with_mps, NULL
        );

        if (ioapic_data.ioapic_count == 0)
        {
            driver_log_state(cpu_self, DRIVER_LOG_WARN,
                "Cannot find any IOAPICs under MPCT\n");
            return;
        }

        ioapic_sort_list();

        cpu_scan_mps(
            cpu_self,
            ioapic_data.saved_parms->mps_driver_node,
            int_assign_with_mps, NULL
        );
    }

    driver_log_state(cpu_self, DRIVER_LOG_NOTICE,
    "IOAPIC(s) detected: %zu\n", ioapic_data.ioapic_count);
}

void ioapic_disable(struct generic_driver_tree_node_t* cpu_self)
{
    memset(ioapic_data.ioapic_list, 0, sizeof(ioapic_data.ioapic_list));
    memset(ioapic_data.legacy_irq_map, 0, sizeof(ioapic_data.legacy_irq_map));
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

static u32 ioapic_normalize_irq(u8 irq);
static void ioapic_redirect_gsi(u8 gsi, u8 vector, u8 lapic_id);
static void ioapic_rewire_gsi(u8 gsi);
static void ioapic_cut_gsi(u8 gsi);
// TODO: MORE

struct cpu_interrupt_ops_t int_ops = {
    .normalize_interrupt_num = &ioapic_normalize_irq,
    .redirect_interrupt = &ioapic_redirect_gsi,
    .rewire_interrupt = &ioapic_rewire_gsi,
    .cut_interrupt = &ioapic_cut_gsi,
};

const struct cpu_interrupt_ops_t* cpu_load_int_ops()
{
    return &int_ops;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

static int ioapic_cmp(const void* a, const void* b)
{
    const struct ioapic_info_t* cmp_a =
    (const struct ioapic_info_t*)a,
    *cmp_b = (const struct ioapic_info_t*)b;

    if (cmp_a->id < cmp_b->id) return -1;
    if (cmp_a->id > cmp_b->id) return 1;
    return 0;
}

static void ioapic_sort_list()
{
    sort(
        ioapic_data.ioapic_list,
        ioapic_data.ioapic_count,
        sizeof(struct ioapic_info_t),
        ioapic_cmp
    );
}

static int ioapic_find_by_gsi(u8 gsi)
{
    for (usize i=0;i<ioapic_data.ioapic_count;i++)
    {
        struct ioapic_info_t* ioapic_list_entry = 
        &ioapic_data.ioapic_list[i];

        u32 gsi_start = ioapic_list_entry->gsi_start;
        u32 gsi_end = gsi_start+ioapic_list_entry->max_redirs;
        if (gsi >= ioapic_list_entry->gsi_start &&
            gsi < gsi_end
        ) return i;
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static void ioapic_write(struct ioapic_info_t* entry, u8 reg, u32 value)
{
    entry->mmio_util->layer->writel(entry->mmio_util, IOAPIC_IOREGSEL, reg);
    entry->mmio_util->layer->writel(entry->mmio_util, IOAPIC_IOREGWIN, value);
}

static u32 ioapic_read(struct ioapic_info_t* entry, u8 reg)
{
    entry->mmio_util->layer->writel(entry->mmio_util, IOAPIC_IOREGSEL, reg);
    return entry->mmio_util->layer->readl(entry->mmio_util, IOAPIC_IOREGWIN);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static u32 ioapic_normalize_irq(u8 irq)
{
    struct irq_map_entry_t* irq_map_entry = irq<16?
    &ioapic_data.legacy_irq_map[irq]:
    NULL;

    if (irq_map_entry && irq_map_entry->present)
        return irq_map_entry->gsi_num;
    return irq;
}

static void ioapic_redirect_gsi(u8 gsi, u8 vector, u8 lapic_id)
{
    int ioapic_num = ioapic_find_by_gsi(gsi);
    if (ioapic_num < 0) return;

    struct ioapic_info_t* ioapic_list_entry = 
    &ioapic_data.ioapic_list[ioapic_num];

    struct irq_map_entry_t* irq_map_entry = gsi<16?
    &ioapic_data.legacy_irq_map[gsi]:
    NULL;

    u32 index = gsi-ioapic_list_entry->gsi_start;

    u32 low = vector | (0<<8); // Fixed delivery | Vector Number
    u32 high = lapic_id << 24; // Bits 56 to 63: LAPIC ID
    if (irq_map_entry && irq_map_entry->present)
    {
        u16 flags = irq_map_entry->flags;

        if (flags & MADT_IOAPIC_FLAG_ACTIVE_LO 
        || flags & MPCT_INT_ASSIGN_FLAG_ACTIVE_LO)
            low |= (1<<13);
        
        if (flags & MADT_IOAPIC_FLAG_LEVEL_TRIGGERED
        || flags & MPCT_INT_ASSIGN_FLAG_LEVEL_TRIGGERED)
            low |= (1<<15);
    }

    ioapic_write(ioapic_list_entry, IOAPIC_REGTBLn_HI(index), high);
    ioapic_write(ioapic_list_entry, IOAPIC_REGTBLn_LO(index), low);
}

static void ioapic_rewire_gsi(u8 gsi)
{
    int ioapic_num = ioapic_find_by_gsi(gsi);
    if (ioapic_num < 0) return;

    struct ioapic_info_t* ioapic_list_entry = 
    &ioapic_data.ioapic_list[ioapic_num];

    struct irq_map_entry_t* irq_map_entry = gsi<16?
    &ioapic_data.legacy_irq_map[gsi]:
    NULL;

    u32 index = gsi-ioapic_list_entry->gsi_start;

    u32 low = ioapic_read(ioapic_list_entry, IOAPIC_REGTBLn_LO(index));
    u32 high = ioapic_read(ioapic_list_entry, IOAPIC_REGTBLn_HI(index));

    low &= ~(1<<16); // Clear mask bit

    ioapic_write(ioapic_list_entry, IOAPIC_REGTBLn_HI(index), high);
    ioapic_write(ioapic_list_entry, IOAPIC_REGTBLn_LO(index), low);
}

static void ioapic_cut_gsi(u8 gsi)
{
    int ioapic_num = ioapic_find_by_gsi(gsi);
    if (ioapic_num < 0) return;

    struct ioapic_info_t* ioapic_list_entry = 
    &ioapic_data.ioapic_list[ioapic_num];

    struct irq_map_entry_t* irq_map_entry = gsi<16?
    &ioapic_data.legacy_irq_map[gsi]:
    NULL;

    u32 index = gsi-ioapic_list_entry->gsi_start;

    u32 low = ioapic_read(ioapic_list_entry, IOAPIC_REGTBLn_LO(index));
    u32 high = ioapic_read(ioapic_list_entry, IOAPIC_REGTBLn_HI(index));

    low |= (1<<16); // Set mask bit

    ioapic_write(ioapic_list_entry, IOAPIC_REGTBLn_HI(index), high);
    ioapic_write(ioapic_list_entry, IOAPIC_REGTBLn_LO(index), low);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void ioapic_detect_with_madt(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* madt_node,
    void* record,
    void* ctx
)
{
    (void)ctx;
    madt_record_entry_hdr_t* p = (madt_record_entry_hdr_t*)madt_node->additionals;
    if (p->entry_type != 2) return;
    if (ioapic_data.ioapic_count >= MAX_IOAPIC_ENTRIES) return;

    madt_record_ioapic_t* ioapic_ptr = (madt_record_ioapic_t*)p;

    struct ioapic_info_t* ioapic_list_entry = 
    &ioapic_data.ioapic_list[ioapic_data.ioapic_count];

    ioapic_list_entry->id = ioapic_ptr->ioapic_id;
    ioapic_list_entry->base = ioapic_ptr->ioapic_addr;
    ioapic_list_entry->gsi_start = ioapic_ptr->gsi_base;

    // Request MMIO resource for the IOAPIC base
    struct generic_driver_resource_t* res_node = 
    driver_request_resource(
        cpu_self,
        DRIVER_RES_TYPE_MMIO
    );
    driver_set_res_data(cpu_self, res_node, ioapic_list_entry->base,
    4096, MMIO_FLAG_UNCACHEABLE);
    
    ioapic_list_entry->mmio_util = &res_node->resource.mmio_info;
    u32 ioapicver_reg = ioapic_read(ioapic_list_entry, IOAPIC_REG_VER);

    ioapic_list_entry->max_redirs = ((ioapicver_reg>>16)&0xFF)+1;

    ioapic_data.ioapic_count++;
}

static void legacy_irq_detect_with_madt(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* madt_node,
    void* record,
    void* ctx
)
{
    (void)ctx;
    madt_record_entry_hdr_t* p = (madt_record_entry_hdr_t*)record;
    if (p->entry_type != 2) return;
    
    madt_record_ioapic_iso_t* iso_ptr = (madt_record_ioapic_iso_t*)p;
    if (iso_ptr->irq_source >= MAX_IRQ_MAP_ENTRIES) return;
    
    struct irq_map_entry_t* legacy_irq_map_entry = &ioapic_data.legacy_irq_map[iso_ptr->irq_source];
    
    legacy_irq_map_entry->present = true;
    legacy_irq_map_entry->irq_line = iso_ptr->irq_source;
    legacy_irq_map_entry->flags = iso_ptr->flags;
    legacy_irq_map_entry->gsi_num = iso_ptr->gsi;
}

static void nmi_detect_with_madt(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* madt_node,
    void* record,
    void* ctx
)
{
    (void)ctx;
    madt_record_entry_hdr_t* p = (madt_record_entry_hdr_t*)record;
    if (p->entry_type != 3) return;

    madt_ioapic_nmi_src_t* nmi_ptr = (madt_ioapic_nmi_src_t*)p;

    
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void ioapic_detect_with_mps(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* mpct_node,
    void* record,
    void* ctx
)
{
    (void)ctx;
    mpct_ioapic_entry_t* ioapic_ptr = (mpct_ioapic_entry_t*)record;
    if (ioapic_ptr->entry_type != 2) return;

    struct ioapic_info_t* ioapic_list_entry_pred = ioapic_data.ioapic_count==0?
    NULL :
    &ioapic_data.ioapic_list[ioapic_data.ioapic_count-1];

    struct ioapic_info_t* ioapic_list_entry = 
    &ioapic_data.ioapic_list[ioapic_data.ioapic_count];

    ioapic_list_entry->id = ioapic_ptr->ioapic_id;
    ioapic_list_entry->base = ioapic_ptr->ioapic_base;
    ioapic_list_entry->gsi_start = (ioapic_data.ioapic_count==0)?
    0:
    ioapic_list_entry_pred->gsi_start+ioapic_list_entry_pred->max_redirs;

    // Request MMIO resource for the IOAPIC base
    struct generic_driver_resource_t* res_node = 
    driver_request_resource(
        cpu_self,
        DRIVER_RES_TYPE_MMIO
    );
    driver_set_res_data(cpu_self, res_node, ioapic_list_entry->base,
    4096, MMIO_FLAG_UNCACHEABLE);
    
    ioapic_list_entry->mmio_util = &res_node->resource.mmio_info;
    u32 ioapicver_reg = ioapic_read(ioapic_list_entry, IOAPIC_REG_VER);

    ioapic_list_entry->max_redirs = ((ioapicver_reg>>16)&0xFF)+1;

    ioapic_data.ioapic_count++;
}

static void int_assign_with_mps(
    struct generic_driver_tree_node_t* cpu_self,
    struct generic_driver_tree_node_t* mpct_node,
    void* record,
    void* ctx
)
{
    (void)ctx;
    mpct_int_assign_entry_t* assign_ptr = (mpct_int_assign_entry_t*)record;
    if (assign_ptr->entry_type != 3) return;

    if (assign_ptr->src_bus_irq >= MAX_IRQ_MAP_ENTRIES) return;

    switch (assign_ptr->interrupt_type)
    {
        case 0:
        {
            // Only map the first 16 GSIs => Only IOAPIC ID 0 is needed.
            if (assign_ptr->dst_ioapic_id > 0) break;
            
            struct irq_map_entry_t* irq_map_entry = &ioapic_data.legacy_irq_map[assign_ptr->src_bus_irq];

            irq_map_entry->present = true;
            irq_map_entry->irq_line = assign_ptr->src_bus_irq;
            irq_map_entry->gsi_num = assign_ptr->dst_ioapic_intin;
            irq_map_entry->flags = assign_ptr->flags;
            break;
        }
        case 1:
            // TODO: Add NMI support
            break;
        case 2:
            // Skip SMI
            break;
        case 3:
            // TODO: 90s-2000s era machine support 
            break;
    }
}