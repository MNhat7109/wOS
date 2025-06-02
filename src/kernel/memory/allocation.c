#include "memory.h"
#include "../paging/page_allocator.h"
#include "../paging/page_table_manager.h"
#include "../string/string.h"
#include <stdbool.h>
#include "../stdio.h"

#define MAX_SLAB_DATA 3584
#define MAX_ALLOC 3584
#define MAX_QUEUE_ENTRIES 128
#define BUDDY_FREE 0
#define BUDDY_USED 1
#define BUDDY_PARTIALLY_USED 2

typedef struct
{
    bool _reserved;
    u8 size;
    struct slab_chunk_t* next;
    bitmap_t used_bmp;
    u8 bmp_buffer[494];
    u8 data[MAX_SLAB_DATA];
} __attribute__((packed)) slab_chunk_t;

typedef struct
{
    slab_chunk_t* list;
    u32 pos;
    u32 page_count;
} __attribute__((packed)) slab_t;

typedef struct
{
    u32 size;
    u8 status;
    u8 _padding0[3];
    struct buddy_chunk_t *left, *right, *parent;
    void* memory_addr;
    u8 _padding1[8];
} __attribute__((packed)) buddy_chunk_t;

typedef struct
{
    buddy_chunk_t* list;
    u32 list_data;
    buddy_chunk_t* last_item;
    u32 list_max_cnt;
    u32 page_count;
    u32 current_count;
    u32 memory_limit;
    void* memory_area;
} __attribute__((packed)) buddy_t;

struct mem_alloc_t
{
    u32 total_page_count;
    u32 slab_addr;
    slab_t slab_list;
    u32 buddy_addr;
    buddy_t buddy_list;
} __attribute__((packed)) *alloc_pack=NULL;

u8 slab_avl_size[] = {8, 16, 32, 64};

u32 memory_find_smaller_power_of_two(u32 number)
{
    number |= (number>>1);
    number |= (number>>2);
    number |= (number>>4);
    number |= (number>>8);
    number |= (number>>16);

    return number-(number>>1);
}

u32 memory_find_larger_power_of_two(u32 number)
{
    number--;
    number |= (number>>1);
    number |= (number>>2);
    number |= (number>>4);
    number |= (number>>8);
    number |= (number>>16);
    number++;

    return number;
}

void memory_init_buddy(buddy_chunk_t** bud, u32 size, u32 offset, struct buddy_chunk_t* parent);

void memory_init_alloc(u32 address, u32 page_count)
{
    kprintf("%u\n", page_count);
    u32 pos = address;
    for (u32 i=0;i<page_count;i++)
    {
        page_manager_map_memory(pos, page_alloc_request());
        pos+=0x1000;
    }

    alloc_pack = (struct mem_alloc_t*)address;
    alloc_pack->total_page_count = page_count;
    alloc_pack->slab_addr = address+page_convert_from_bytes(sizeof(struct mem_alloc_t))*0x1000;
    page_count--;
    alloc_pack->slab_list.list = NULL;
    alloc_pack->slab_list.pos =0;
    alloc_pack->slab_list.page_count = page_count /2;
    
    alloc_pack->buddy_addr = alloc_pack->slab_addr+alloc_pack->slab_list.page_count*0x1000;
        
    alloc_pack->buddy_list.list = NULL;
    alloc_pack->buddy_list.current_count = 0;
    alloc_pack->buddy_list.page_count = page_count/2;
    alloc_pack->buddy_list.list_data = alloc_pack->buddy_addr;
    alloc_pack->buddy_list.list_max_cnt = ((alloc_pack->buddy_list.page_count/2)*0x1000)/sizeof(buddy_chunk_t);
    alloc_pack->buddy_list.memory_area = (void*)alloc_pack->buddy_addr+alloc_pack->buddy_list.list_max_cnt*sizeof(buddy_chunk_t);
    alloc_pack->buddy_list.memory_limit = (alloc_pack->buddy_list.page_count>>1)*0x1000;
    memset(alloc_pack->buddy_list.memory_area, 0, alloc_pack->buddy_list.memory_limit);
    
    u32 total_mem_in_po2 = memory_find_smaller_power_of_two(alloc_pack->buddy_list.memory_limit);
    memory_init_buddy(&alloc_pack->buddy_list.list, total_mem_in_po2, (u32)alloc_pack->buddy_list.memory_area, NULL);
    alloc_pack->buddy_list.last_item = alloc_pack->buddy_list.list;

}

void memory_init_slab(slab_chunk_t** slab, u8 size)
{
    if (alloc_pack->slab_list.pos==alloc_pack->slab_list.page_count) 
    {
        *slab = NULL; return;
    }
    *slab = (slab_chunk_t*)(alloc_pack->slab_addr+alloc_pack->slab_list.pos++*4096);
    memset(*slab, 0, 4096);
    (*slab)->size = size;
    (*slab)->next = NULL;
    bitmap_init_buffer(&(*slab)->used_bmp, MAX_SLAB_DATA/size, (u32)(*slab)->bmp_buffer);
    bitmap_set_bits(&(*slab)->used_bmp, 0);
}

slab_chunk_t* memory_slab_append(slab_chunk_t* head, slab_chunk_t* new_slab)
{
    if (!new_slab) return head;
    
    if (!head) return new_slab;
    slab_chunk_t* last = head;
    
    while (last->next != NULL)
        last = (slab_chunk_t*)last->next;

    last->next = (struct slab_chunk_t*)new_slab;
    return head;
}

void memory_slab_find_size(u32* size)
{
    u32 lo = 0, hi = 3;
    u32 floor = 0;
    while (lo <= hi)
    {
        u32 mid = (lo+hi)>>1;
        if (slab_avl_size[mid] == *size) return;
        else if (slab_avl_size[mid] > size) 
            hi = mid-1;
        else 
        {
            floor = slab_avl_size[mid];
            lo = mid+1;
        }
    }

    *size = floor;
}

void* memory_slab_alloc(u32 size)
{
    memory_slab_find_size(&size);
    if (size == 0) return NULL;
    
    slab_chunk_t* s = alloc_pack->slab_list.list;
    while (s)
    {
        u32 bmp_size_bits = s->used_bmp.size*8;
        for (u32 i=0;i<bmp_size_bits;i++)
        {
            u8 is_used = bitmap_get_bits(&s->used_bmp, i);
            if (!is_used && s->size == size)
            {
                kprintf("Slab: Found free pointer at 0x%x\n", &s->data[i*size]);
                void* p = (void*)&s->data[i*size];
                bitmap_set_bits(&s->used_bmp, i);
                return p;
            }
        }
        s = (slab_chunk_t*)s->next;
    }
    memory_init_slab(&s, size);
    if (!s) return NULL;
    alloc_pack->slab_list.list = memory_slab_append(alloc_pack->slab_list.list, s);
    
    kprintf("Slab: Created free slabs, got free pointer at 0x%x\n", &s->data[0]);
    return &s->data[0];
}

bool memory_slab_free_ptr(slab_chunk_t** list, void* ptr)
{
    slab_chunk_t* s = *list;
    while (s)
    {
        // Check if the pointer actually belongs to that slab
        if ((u32)ptr >= (u32)s->data && (u32)ptr < (u32)(s->data+MAX_SLAB_DATA))
        {
            u32 bit_pos = ((u32)ptr-(u32)s->data)/s->size;
            u8 is_used = bitmap_get_bits(&s->used_bmp, bit_pos);
            if (!is_used) 
            {
                kprintf("Slab: That pointer has already been freed :(\n");
                return false;
            }
            kprintf("Slab: Pointer at address 0x%x is now freed\n", ptr);
            bitmap_clear_bits(&s->used_bmp, bit_pos);
            return true;
        }
        s = (slab_chunk_t*)s->next;
    }

    kprintf("Slab: Pointer not found in slab :(\n");
    return false;
}

void memory_slab_reclaim(slab_chunk_t** slab)
{
    if (!*slab) return;
    *slab = (slab_chunk_t*)(*slab)->next;
    alloc_pack->slab_list.pos--;
}

bool memory_slab_free(void* ptr)
{
    slab_chunk_t* s = alloc_pack->slab_list.list;
    if (!memory_slab_free_ptr(&s, ptr)) return false;
    // CHeck for bitmap.
    u32 bmp_size_bits = s->used_bmp.size*8;
    for (u32 i=0;i<bmp_size_bits;i++)
    {
        u8 is_used = bitmap_get_bits(&s->used_bmp, i);
        if (is_used) return true;
    }
    kprintf("Slab: Slab is completely freed, reclaiming\n");
    memory_slab_reclaim(&s);
    return true;
}

void memory_init_buddy(buddy_chunk_t** bud, u32 size, u32 offset, struct buddy_chunk_t* parent)
{
    u32 buddy_max_cnt = alloc_pack->buddy_list.list_max_cnt;
    if (alloc_pack->buddy_list.current_count > buddy_max_cnt)
        return;
    
    buddy_chunk_t* free = 0;
    buddy_chunk_t* data = (buddy_chunk_t*)alloc_pack->buddy_list.list_data;
    for (u32 i=0;i<buddy_max_cnt;i++)
    {
        // If the list data is empty, the data extracted shall be zero
        if (data[i].memory_addr == 0) 
        {
            free = &data[i];
            alloc_pack->buddy_list.current_count++;
            break;
        }
    }
    *bud = free;
    (*bud)->size = size;
    (*bud)->left = (*bud)->right = NULL;
    (*bud)->parent = parent;
    (*bud)->status = false;
    (*bud)->memory_addr = (void*)offset;
}

buddy_chunk_t* memory_buddy_split(buddy_chunk_t* head, u8 status)
{
    if (!head) return NULL; // Will never happen

    head->status = status;
    u32 split_size = head->size >> 1;
    buddy_chunk_t* left_child = (buddy_chunk_t*)head->left;
    buddy_chunk_t* right_child = (buddy_chunk_t*)head->right;
    memory_init_buddy(&left_child, split_size, (u32)head->memory_addr, (struct buddy_chunk_t*)head);
    memory_init_buddy(&right_child, split_size, (u32)(head->memory_addr+split_size), (struct buddy_chunk_t*)head);
    head->left = (struct buddy_chunk_t*)left_child;
    head->right= (struct buddy_chunk_t*)right_child; // I HATE C
    return head;
}

void* memory_buddy_alloc(u32 size)
{
    buddy_chunk_t* b = alloc_pack->buddy_list.list;
    u32 size_po2 = memory_find_larger_power_of_two(size);

    while (b && b->size > size_po2)
    {
        b->status = BUDDY_PARTIALLY_USED;
        // Only splits if it's a leaf node
        if (!b->left && !b->right) 
        {
            // kprintf("Block size of: %u at address: 0x%x will be split\n", b->size, b->memory_addr);
            b = memory_buddy_split(b, b->status);
        }
        buddy_chunk_t* left_child = (buddy_chunk_t*)b->left;
        buddy_chunk_t* right_child = (buddy_chunk_t*)b->right;
        if (left_child->status != BUDDY_USED) 
        {
            // kprintf("Left child is %s.\n", (const char*[]){"free", "used", "partially used"}[left_child->status]);
            b = left_child;
        }
        else if (right_child->status != BUDDY_USED) 
        {
            // kprintf("Right child is %s.\n", (const char*[]){"free", "used", "partially used"}[right_child->status]);
            b = right_child;
        }
        else 
        {
            b->status = BUDDY_USED;
            b = (buddy_chunk_t*)b->parent; 
        }
    }

    if (!b)
    {
        kprintf("Buddy: The memory area is now full. Pointers can no longer be allocated in here...\n");
        return NULL;
    } 
    b->status = BUDDY_USED;
    kprintf("Buddy: Block allocated at address: 0x%x, size: %u bytes\n", b->memory_addr, b->size);
    alloc_pack->buddy_list.last_item = b;
    // Returns the memory address
    return b->memory_addr;
}

void memory_destroy_buddy(buddy_chunk_t** buddy)
{
    if (!(*buddy)) return;
    memset(*buddy, 0, sizeof(buddy_chunk_t));
    *buddy = NULL;
}

void memory_buddy_merge(buddy_chunk_t** head)
{
    if (!(*head)) return;
    // kprintf("Item size: %u, %s\n", (*head)->size, (const char*[]){"free", "used", "partial"}[(*head)->status]);
    buddy_chunk_t* left =((buddy_chunk_t*)(*head)->left);
    buddy_chunk_t* right =((buddy_chunk_t*)(*head)->right);
    if ((left && left->status == BUDDY_FREE) && (right&&right->status == BUDDY_FREE))
    {
        memory_destroy_buddy(&left);
        memory_destroy_buddy(&right);
        (*head)->left = (struct buddy_chunk_t*)left;
        (*head)->right = (struct buddy_chunk_t*)right;
        (*head)->status = BUDDY_FREE;
    }
    memory_buddy_merge(&left);
    (*head)->left = (struct buddy_chunk_t*)left;
    memory_buddy_merge(&right);
    (*head)->right = (struct buddy_chunk_t*)right;
}

bool memory_buddy_free(void* ptr)
{
    buddy_chunk_t* b = alloc_pack->buddy_list.list;
    // Find our block
    // We will actually use BFS for this.
    buddy_chunk_t* queue[MAX_QUEUE_ENTRIES] = {NULL};
    int queue_rear = -1, queue_front = 0;
    queue[++queue_rear] = b;
    while (queue_front <= queue_rear)
    {
        buddy_chunk_t* current_item = queue[queue_front];
        queue_front++;
        
        if (current_item->status == BUDDY_FREE) 
        {
            if (current_item->memory_addr == ptr)
            {
                kprintf("Buddy: that pointer has already been freed\n");
                return false;
            }
            continue; /*We do not want to free an already freed block*/
        }
        
        if (current_item->memory_addr == ptr && current_item->status == BUDDY_USED)
        {
            current_item->status = BUDDY_FREE;
            b = current_item;
            break;
        }
        
        if (queue_rear >= MAX_QUEUE_ENTRIES - 1
            ||queue_rear+1 >= MAX_QUEUE_ENTRIES - 1) 
            {
                kprintf("Buddy: Out of queue entries\n");
                return false;
            }
        queue[++queue_rear] = (buddy_chunk_t*)current_item->left; 
        queue[++queue_rear] = (buddy_chunk_t*)current_item->right; 
    }
    

    // Merge
    memory_buddy_merge(&alloc_pack->buddy_list.list);
    kprintf("Buddy: Block finally freed, area merged\n");
    return true;
}

void* memory_allocate(u32 size)
{
    void* ptr = NULL;
    if (size <= 64)
    {
        ptr = memory_slab_alloc(size);
        if (ptr) return ptr;
    }
    ptr = memory_buddy_alloc(size);
    if (ptr) return ptr;
    return NULL;
}

void memory_free(void* block)
{
    bool status = true;
    status = memory_slab_free(block);
    if (status) return;
    kprintf("Slab allocator doesn't work, was allocated using buddy maybe?\n");
    status = memory_buddy_free(block);
    if (status) return;
    return;
}

void memory_destroy_alloc()
{
    u32 page_count = alloc_pack->total_page_count;
    kprintf("Count: %u\n", page_count);
    memset(alloc_pack, 0, page_count*0x1000);
    u32 pos = (u32)alloc_pack;
    for (int i=0;i<page_count;i++)
    {
        bool stat = page_manager_unmap_memory(pos);
        if (!stat)
        {
            kprintf("Unmapping failed at address 0x%x\n", pos);
            return;
        }
        pos+=0x1000;
    }
    // page_alloc_freen((u32)alloc_pack, page_count);
}