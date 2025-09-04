#include <devices/driver.h>

#include <libk/stdio.h>
#include <stdarg.h>

static const char* stat_str[] = {
    "failed",
    "ready to use",
    "probed",
    "not probed (yet)",
    "disabled"
};

static void driver_call_wrapper(struct generic_driver_tree_node_t* driver, u8 asserted_state, u8 next_state, 
    void (*drv_cb)(struct generic_driver_tree_node_t*));
static void driver_read_state(struct generic_driver_tree_node_t* driver);
static void driver_print_prefix(struct generic_driver_tree_node_t* driver);

void driver_log_state(
    struct generic_driver_tree_node_t* driver, 
    log_state_t level, 
    const char* fmt, ...
)
{
    if (level == DRIVER_LOG_ERROR) driver->state = DRIVER_STATE_FAILED;

    driver_print_prefix(driver);

    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);
}

bool driver_run(struct generic_driver_tree_node_t* driver)
{
    if (!driver) return false;

    bool status, running=true;

    while (running)
    {
        driver_read_state(driver);

        switch (driver->state)
        {
            case DRIVER_STATE_READY:
                status = true;
                running = false;
                break;
            case DRIVER_STATE_FAILED:
                status = false;
                running = false;
                break;
            case DRIVER_STATE_DISABLED:
            case DRIVER_STATE_PROBED:
                driver_call_wrapper(
                    driver,
                    DRIVER_STATE_PROBED,
                    DRIVER_STATE_READY,
                    driver->ops->config
                );
                break;
            case DRIVER_STATE_UNPROBED:
                driver_call_wrapper(
                    driver,
                    DRIVER_STATE_UNPROBED,
                    DRIVER_STATE_PROBED,
                    driver->ops->probe
                );
                break;
            default:
                driver->state = DRIVER_STATE_FAILED;
                break;
        }
    }

    return status;
} 

bool driver_terminate(struct generic_driver_tree_node_t* driver)
{
    if (!driver) return false;
    if (driver->state != DRIVER_STATE_READY) return false;

    driver_call_wrapper(
        driver,
        DRIVER_STATE_READY,
        DRIVER_STATE_DISABLED,
        driver->ops->disable
    );

    if (driver->state == DRIVER_STATE_FAILED) return false;
    return true;
}

bool driver_load_ops(
    struct generic_driver_tree_node_t* driver,
    driver_callback_t callback
)
{
    if (!callback) return false;

    struct generic_driver_ops_t* ops = callback();
    if (!ops) return false;

    driver->ops = ops;
    return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static void driver_call_wrapper(
    struct generic_driver_tree_node_t* driver, 
    u8 asserted_state, 
    u8 next_state, 
    void (*drv_cb)(struct generic_driver_tree_node_t*)
)
{
    if (!driver || driver->state != asserted_state) return;

    if (!drv_cb)
    {
        driver->state = DRIVER_STATE_FAILED;
        return;
    }

    drv_cb(driver);

    if (driver->state == DRIVER_STATE_FAILED) return;
    driver->state = next_state;
}

static void driver_read_state(struct generic_driver_tree_node_t* driver)
{
    driver_print_prefix(driver);
    kprintf("%s\n", stat_str[driver->state]);
}

static void driver_print_prefix(struct generic_driver_tree_node_t* driver)
{
    kprintf("Driver [%s, ", driver->name);
    switch (driver->id_type)
    {
        case DRIVER_ID_TYPE_HWID:
            kprintf("VEN_%x&DEV_%x", 
                driver->id.hwid.vendor,driver->id.hwid.device);
            break;
        case DRIVER_ID_TYPE_INTERNAL:
            kprintf("%s", driver->id.compatible);
            break;
        default:
            kprintf("N/A");
            break;
    }

    kprintf("]: ");
}