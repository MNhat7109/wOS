#include <devices/driver.h>

#include <libk/stdlib.h>

#include <stdarg.h>

static const char* res_type_str[] = {
    [DRIVER_RES_TYPE_DEPENDENCY] = "Dependency Driver",
    [DRIVER_RES_TYPE_DMA] = "DMA",
    [DRIVER_RES_TYPE_MMIO] = "MMIO",
    [DRIVER_RES_TYPE_PIO] = "PIO",
    [DRIVER_RES_TYPE_INTLINE] = "IRQ Line",
    [DRIVER_RES_TYPE_MSI] = "MSI Vector",
    [DRIVER_RES_TYPE_UNKNOWN] = "N/A"
};

struct generic_driver_resource_t* driver_request_resource(
    struct generic_driver_tree_node_t* driver,
    generic_driver_res_type_t res_type
)
{
    if (!driver) 
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Unspecified host driver node\n"); 
        return NULL;
    }

    // Create a new node
    struct generic_driver_resource_t* new_node = 
    kmalloc(sizeof(struct generic_driver_resource_t));
    if (!new_node) 
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Out of memory to allocate new resource nodes\n"); 
        return NULL;
    }

    // Populate
    new_node->res_type = res_type;
    new_node->next = NULL;

    // Link
    if (!driver->resource_list)
    {
        driver->resource_list = new_node;
        return new_node;
    }

    struct generic_driver_resource_t* current_node = driver->resource_list;
    while (current_node->next) current_node = current_node->next;
    current_node->next = new_node;

    driver_log_state(driver, DRIVER_LOG_NOTICE, 
        "Requested new resource at node offset=0x%p, "
        "type: %s\n",
        new_node,
        res_type_str[new_node->res_type]
    ); 
    return new_node;
}

bool driver_untie_resource(
    struct generic_driver_tree_node_t* driver,
    struct generic_driver_resource_t* resource
)
{
    if (!driver || !resource) 
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, "Unspecified host driver node or resource\n"); 
        return false;
    }

    // Unlink the node
    struct generic_driver_resource_t** res_link = &driver->resource_list;
    while (*res_link && *res_link != resource)
        res_link = &(*res_link)->next;

    if (!*res_link) 
    {
        driver_log_state(driver, DRIVER_LOG_ERROR, 
            "Cannot find the specified resource node 0x%p\n",
            resource
        ); 
        return false;
    }
    *res_link = resource->next;

    // Send signal to clients to release themselves
    switch (resource->res_type)
    {
        case DRIVER_RES_TYPE_DEPENDENCY:
            driver_deref(resource->resource.dependency);
            break;
        case DRIVER_RES_TYPE_MMIO:
            mmio_release(&resource->resource.mmio_info);
            break;
        case DRIVER_RES_TYPE_PIO:
        {
            int status = pio_release(&resource->resource.pio_info);
            if (status == -PIO_ERR_UNSUPPORTED)
                driver_log_state(driver, DRIVER_LOG_ERROR, "API does NOT support PIO\n");
            break;
        }
        case DRIVER_RES_TYPE_INTLINE:
        case DRIVER_RES_TYPE_MSI:
        case DRIVER_RES_TYPE_DMA:
            // Ignore that, or if you're paranoid,
            // You can set it to 0
            break;
        default:
            driver_log_state(driver, DRIVER_LOG_ERROR, "Unknown resource type\n");
            return false;
    }
    
    // Free the node
    kfree(resource);

    return true;
}

/*
* Set resource node's data based on its type.
*
* Dependencies: Driver node
* MMIO: Base, size and optional flags
* PIO: Port value
* DMA: Channel number
* MSI: Vector number
* IRQ: IRQ Line number
*/
void driver_set_res_data(
    struct generic_driver_tree_node_t* driver,
    struct generic_driver_resource_t* resource,
    ...
)
{
    va_list args;
    va_start(args, resource);
    switch (resource->res_type)
    {
        case DRIVER_RES_TYPE_DEPENDENCY:
            resource->resource.dependency = 
            va_arg(args, struct generic_driver_tree_node_t*);
            driver_ref(resource->resource.dependency);
            break;
        case DRIVER_RES_TYPE_MMIO:
            mmio_acquire(
                &resource->resource.mmio_info, 
                va_arg(args, u64), // Base address
                va_arg(args, u64), // Size
                va_arg(args, u32) // Flags
            );
            break;
        case DRIVER_RES_TYPE_PIO:
        {
            int status = pio_acquire(
                &resource->resource.pio_info,
                va_arg(args, int) // Port number
            );
            if (status == -PIO_ERR_UNSUPPORTED)
                driver_log_state(driver, DRIVER_LOG_ERROR, "API does NOT support PIO\n");
            break;
        }
        case DRIVER_RES_TYPE_INTLINE:
            resource->resource.interrupt_info.int_line = va_arg(args, int);
            break;
        case DRIVER_RES_TYPE_MSI:
            resource->resource.interrupt_info.msi_vector = va_arg(args, int);
            break;
        case DRIVER_RES_TYPE_DMA:
            resource->resource.dma_channel = va_arg(args, int);
            break;
        default:
            driver_log_state(driver, DRIVER_LOG_ERROR, "Unknown resource type\n");
            break;
    }
    va_end(args);
}

void driver_find_res_data(
    struct generic_driver_tree_node_t* driver,
    generic_driver_res_type_t type,
    void* data_out,
    void* criteria
)
{

    for (
        struct generic_driver_resource_t* res=driver->resource_list;
        res; 
        res=res->next
    )
    {
        if (res->res_type != type) continue;

        switch (type)
        {
            case DRIVER_RES_TYPE_DEPENDENCY:
            {
                struct generic_driver_tree_node_t* dep =
                res->resource.dependency;
                if (driver_match_id(dep, criteria))
                {
                    *(void**)data_out = dep;
                    return;
                }
                break;
            }
            case DRIVER_RES_TYPE_MMIO:
            {
                struct mmio_info_t* expected_mmio_criterion = 
                (struct mmio_info_t*)criteria;

                struct mmio_info_t* mmio_res = &res->resource.mmio_info;

                if (mmio_res->base == expected_mmio_criterion->base
                && mmio_res->size == expected_mmio_criterion->size)
                {
                    *(void**)data_out = mmio_res;
                    return;
                }
                break;
            }
            case DRIVER_RES_TYPE_PIO:
            {
                struct pio_info_t* expected_pio_criterion = 
                (struct pio_info_t*)criteria;

                struct pio_info_t* pio_res = &res->resource.pio_info;

                if (pio_res->port == expected_pio_criterion->port)
                {
                    *(void**)data_out = pio_res;
                    return;
                }
                break;
            }
            case DRIVER_RES_TYPE_INTLINE:
            {
                u8 expected_int_line = *(u8*)criteria;
                struct int_info_t* int_res = &res->resource.interrupt_info;
                if (expected_int_line == int_res->int_line)
                {
                    *(void**)data_out = int_res;
                    return;
                }
                break;
            }
            case DRIVER_RES_TYPE_MSI:
            {
                u8 expected_msi = *(u8*)criteria;
                struct int_info_t* int_res = &res->resource.interrupt_info;
                if (expected_msi == int_res->msi_vector)
                {
                    *(void**)data_out = int_res;
                    return;
                }
                break;
            }
            case DRIVER_RES_TYPE_DMA:
            {
                u8 expected_dma = *(u8*)criteria;
                u8 dma_res = res->resource.dma_channel;
                if (expected_dma == dma_res)
                {
                    *(void**)data_out = &res->resource.dma_channel;
                    return;
                }
                break;
            }
            default:
                driver_log_state(driver, DRIVER_LOG_ERROR, "Unknown resource type\n");
                *(void**)data_out = NULL;
                break;
        }
    }
}