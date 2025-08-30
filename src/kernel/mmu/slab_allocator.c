#include <kutils/mmu/slab_allocator.h>
#include <kutils/mmu/utils.h>

#include <libk/bitmap/bitmap.h>
#include <libk/bitmanip/bitmanip.h>
#include <libk/string.h>
#include <libk/stdio.h>
#include <stdbool.h>

struct slab_chunk_t;
typedef struct slab_chunk_t slab_chunk_t;

#define MAX_SLAB_DATA 2048
#define MIN_SLAB_SIZE 64
#define MAX_SLAB_SIZE 512

// Because in cheeky case where items of size 512 bytes cannot
// be allocated because of the object header bulge, we will
// extend to 1024 bytes (hence 5 weight classes)
// 64, 128, 256, 512, 1024

#define MAX_WEIGHT_CLASS 5

#define SLAB_ALLOC_MAGIC 0x51ABA10C
#define SLAB_FULL_MAGIC 0x51ABFFFF
#define SLAB_FREE_MAGIC 0x51ABDEAD

typedef struct slab_chunk_t
{
    struct slab_attributes_t
    {
        u32 magic;
        u16 size;
        u16 padding;
    } __attribute__((packed)) attributes;
    bitmap_t slab_bitmap;
    slab_chunk_t* prev;
    slab_chunk_t* next;
} slab_chunk_t;

typedef struct slab_obj_hdr_t
{
    slab_chunk_t* slab_owner;  
} slab_obj_hdr_t;

static struct
{
    usize total_page_count;
    void* slab_address;
    void* current_slab_address;
    slab_chunk_t* first_free_slab; // Free slabs often have no size
    slab_chunk_t* last_free_slab; // Free slabs often have no size
    slab_chunk_t* last_partial_slab[MAX_WEIGHT_CLASS];
    slab_chunk_t* first_partial_slab[MAX_WEIGHT_CLASS];
    slab_chunk_t* first_full_slab[MAX_WEIGHT_CLASS];
    slab_chunk_t* last_full_slab[MAX_WEIGHT_CLASS];
    usize user_data_offset;
} slab_data;

static u16 slab_alloc_find_known_size(usize size);
static void slab_alloc_reclaim_slab(slab_chunk_t* slab, usize class_idx);
static bool slab_alloc_init_slab(slab_chunk_t** slab, u16 size);

static void slab_alloc_push(slab_chunk_t** head, slab_chunk_t** tail, slab_chunk_t* slab);
static void slab_alloc_pop(slab_chunk_t** head, slab_chunk_t** tail);
static void slab_alloc_remove(slab_chunk_t** head, slab_chunk_t** tail, slab_chunk_t* slab);

void slab_alloc_init(void* address, usize page_count)
{
    kprintf("Slab: Initializing at address 0x%x, max page threshold: %u\n", address, page_count);
    slab_data.slab_address=address;
    slab_data.current_slab_address=address;
    
    slab_data.last_free_slab = NULL;
    memset(slab_data.last_partial_slab, NULL, sizeof(slab_data.last_partial_slab));
    memset(slab_data.last_full_slab, NULL, sizeof(slab_data.last_full_slab));

    memset(slab_data.first_partial_slab, NULL, sizeof(slab_data.first_partial_slab));
    memset(slab_data.first_full_slab, NULL, sizeof(slab_data.first_full_slab));
    
    slab_data.total_page_count = page_count;
    slab_data.user_data_offset = sizeof(slab_chunk_t)+MAX_SLAB_DATA/MIN_SLAB_SIZE;
}

void slab_alloc_destroy()
{
    kprintf("Slab: Destroying slab...\n");
    if (!slab_data.slab_address) return;
    
    memset(slab_data.slab_address, 0, slab_data.total_page_count*0x1000);
    slab_data.slab_address = NULL;
}

void* slab_alloc_request(usize size)
{
    if (size == 0) return NULL;

    u8* user_data;
    
    // We allocate size (size+size of free header)
    u16 fixed_size = slab_alloc_find_known_size(size+sizeof(slab_obj_hdr_t));

    // defined MIN_SLAB_SIZE as 64
    u32 size_class_idx = ctz(fixed_size/MIN_SLAB_SIZE);

    // Bound check the index
    if (size_class_idx >= MAX_WEIGHT_CLASS)
    {
        kprintf("Slab: Max size class exceeded. Expected 64->512 bytes,"
            " got %u bytes instead\n", fixed_size);
        return NULL;
    }
    
    // Here, we check the following:

    // If the slab has already been allocated (by checking the magic number),
    // and has enough space,
    // we can just increase the offset of the slab,
    // then return the pointer to 
    // (the slab's user data (which is after the header) 
    // + the slab's free offset)
    // We will NOT traversing the slabs, as the tail pointer is already the
    // latest slab allocated at that size class 
    
    // Also, the linked list are ensured to NOT be clobbered by "freed" blocks,
    // as they will be unlinked and put in the free slab list
    // Ensuring the constant time complexity (See the free function)

    // We can actually use the buffer as a u64 variable
    // and get the first zero bit using bts (for x86) or ctz (for arm/arm64)
    // at a constant time
    // The algorithm will be reduced to only O(1).

    // There's a catch: ctz will return UB if the input is zero, so we must
    // flip all the bits ("bitwise NOT" the bits) and then check the result if it's zero
    // If so, the current slab is full and the pointer now must be in a new slab
    
    // We will calculate the bits in the slab
    // Since we can allocate at a range from 64 to 512,
    // The buffer will only go up to 64 bits.
    
    // Another case: bound check ptr_idx to the max bit size. 
    // If this exceeds, the current slab is already full 
    // and the pointer now must be in a new slab.
    // Else this can be a really terrible memory leak
    // where the allocator just shove in 64 pointers regardless of the size,
    // which can go up up and away from the 2048-byte limit of user data.
    
    // Another: if the partial list is empty firsthand, 
    // it may need to allocate in a new slab
    
    // So, there will be a "new slab" boolean here, and it will switch to true
    // if either one of the aforementioned cases is true
    
    slab_chunk_t* current_slab = slab_data.last_partial_slab[size_class_idx];
    
    bool new_slab = false;
    u8 ptr_idx;

    if (!current_slab) 
    {
        new_slab = true;
        ptr_idx = 0;
    }
    else
    {
        u64 buffer_bits = *(u64*)current_slab->slab_bitmap.buffer;
        if ((~buffer_bits)==0)
        {
            new_slab = true;
            ptr_idx = 0;
        } 
        else
        {
            ptr_idx = ctz(~buffer_bits);
            u8 max_bits = current_slab->slab_bitmap.size*8;
            if (ptr_idx >= max_bits) 
            {
                new_slab = true;
                ptr_idx = 0;
            }
        }
    }
    
    if (new_slab)
    {
        current_slab->attributes.magic = SLAB_FULL_MAGIC;

        // Pop from the partial list
        slab_alloc_pop(&slab_data.first_partial_slab[size_class_idx],
            &slab_data.last_partial_slab[size_class_idx]);

        // Push to full slab list
        slab_alloc_push(
            &slab_data.first_full_slab[size_class_idx],
            &slab_data.last_full_slab[size_class_idx],
            current_slab);

        // Set current to previous tail
        current_slab = slab_data.last_partial_slab[size_class_idx];

        // if the partial list is now empty, allocate a new slab
        if (!current_slab)
        {
            bool status = slab_alloc_init_slab(&current_slab, fixed_size);
            if (!status) return NULL; // We are only here if there's no more space to allocate the slab
            
            slab_alloc_push(
                &slab_data.first_partial_slab[size_class_idx],
                &slab_data.last_partial_slab[size_class_idx],
                current_slab
            );
            kprintf("Slab: Created new slab base=0x%x, size=%u\n", 
                current_slab,
                fixed_size);
            }
    }
    
    user_data = (u8*)current_slab+slab_data.user_data_offset;
    slab_obj_hdr_t* hdr = (slab_obj_hdr_t*)(user_data+ptr_idx*fixed_size);
    hdr->slab_owner = current_slab;

    bitmap_set_bits(&current_slab->slab_bitmap, ptr_idx);
    
    kprintf("Slab: Allocating memory to address 0x%x, at slab base=0x%x size=%u\n", 
                user_data+ptr_idx*fixed_size, 
                current_slab,
                fixed_size);

    return (void*)((u8*)hdr+sizeof(slab_obj_hdr_t));
}

void slab_alloc_free(void* ptr)
{
    u8* blk_ptr = (u8*)ptr;
    
    // So, for "free" we will do the following:
    
    // Go back to the object header, in order to access the owner of the object
    // Which slab owns the object?
    // Directly accessing this in an arbitrary header helps us to directly find
    // the object's location in order to do anything further.
    // Otherwise, we will have to scorch-earth search the whole list (Oh man, 
    // don't even get me started on "partial" or "full" - you have to
    // search both) just to find the right pointer (Jesus Christ!)

    // This ensure "free" has a constant time complexity of O(1).

    slab_obj_hdr_t* hdr = (slab_obj_hdr_t*)((u8*)ptr-sizeof(slab_obj_hdr_t));
    slab_chunk_t* current_slab = hdr->slab_owner;

    // If the owner slab is NULL, it's a "stray" pointer, and is likely
    // belongs to another allocator. It is thy child, not mine.

    if (!current_slab)
    {
        kprintf("Slab: Pointer 0x%x cannot be found in slab list\n", ptr);
        return;
    }

    // Now we go on to the next step: Getting the bit position corresponds
    // to our object address, and the object's "weight class" in order to access
    // the right size when adding to/removing from the partial and the full list
    
    // As for our object pointer, if we are able to find the right owner of it (which we just did),
    // we will always be able to find its location in the user data region,
    // and ultimately find its corresponding bit in the bitmap buffer

    u8* user_data = ((u8*)current_slab+slab_data.user_data_offset);
    usize pos = (usize)(blk_ptr-user_data)/current_slab->attributes.size;
    usize size_class_idx = ctz(current_slab->attributes.size/MIN_SLAB_SIZE);

    if (!bitmap_get_bits(&current_slab->slab_bitmap, pos))
    {
        // The bit is already cleared => the client called free after
        // free() is called elsewhere before and cleared the bit out
        
        // Error out
        kprintf("Slab: Double free detected\n");
        return;
    }
    bitmap_clear_bits(&current_slab->slab_bitmap, pos);

    // After we've cleared the bit out, if the slab is still in the "full" category,
    // unlink slab from the full list and integrate it back to the partial list

    if (current_slab->attributes.magic == SLAB_FULL_MAGIC)
    {
        current_slab->attributes.magic = SLAB_ALLOC_MAGIC;
        
        slab_alloc_remove(
            &slab_data.first_full_slab[size_class_idx], 
            &slab_data.last_full_slab[size_class_idx],
            current_slab
        );

        slab_alloc_push(
            &slab_data.first_partial_slab[size_class_idx], 
            &slab_data.last_partial_slab[size_class_idx],
            current_slab
        );
    }

    // Here, as the bitmap buffer only goes up to 64 bits, we can just check the buffer
    // as an 8-byte (u64) variable, if it is zero. If so, the slab is now empty and
    // can be freed

    if (*(u64*)current_slab->slab_bitmap.buffer != 0) return;

    kprintf("Slab: Slab base=0x%x, size=%u is empty, reclaiming...\n", 
        current_slab, current_slab->attributes.size);
    slab_alloc_reclaim_slab(current_slab, size_class_idx);

    // Finally, profit.
    return;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/* GLOBAL HELPER FUNCTIONS */
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

usize slab_alloc_get_max_ptr_size()
{
    return MAX_SLAB_SIZE;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

static u16 slab_alloc_find_known_size(usize size)
{
    if (size<MIN_SLAB_SIZE) return MIN_SLAB_SIZE;
    return mmu_find_next_po2(size);
}

static void slab_alloc_reclaim_slab(slab_chunk_t* slab, usize class_idx)
{
    if (!slab) return;

    (slab)->attributes.magic = SLAB_FREE_MAGIC;

    // Zero out the buffer
    *(u64*)slab->slab_bitmap.buffer = 0;

    // Zero out user data
    u8* user_data = (u8*)slab+slab_data.user_data_offset;
    memset(user_data, 0, MAX_SLAB_DATA);
    
    // Unlink from the partial list

    // In the main free() function, if the object of the full slabs
    // are freed, the slab will be demoted to "partial",
    // and it will eventually come down to the "partial" slab having just only
    // one object left to free and being all empty afterwards.

    // Therefore, we do not have to unlink the slab from the "full list",
    // as it has been demoted when free() was called many instructions before.

    slab_alloc_remove(
        &slab_data.first_partial_slab[class_idx],
        &slab_data.last_partial_slab[class_idx],
        slab
    );

    // Push the newly freed slab to the free list
    // As the free list items often have no size, we actually
    // don't need to access element by class_idx

    slab_alloc_push(
        &slab_data.first_free_slab,
        &slab_data.last_free_slab,
        slab
    );
}

static bool slab_alloc_init_slab(slab_chunk_t** slab, u16 size)
{
    // First priority: Always, always retrieve slabs from
    // a free list

    if (slab_data.last_free_slab)
    {
        *slab = slab_data.last_free_slab;
        slab_alloc_pop(
            &slab_data.first_free_slab,
            &slab_data.last_free_slab
        );
    }
    else
    {
        // If the free list is empty, we push a new slab
        // to our chain

        u8* slab_end = (u8*)slab_data.slab_address+slab_data.total_page_count*0x1000;
        usize stride = slab_data.user_data_offset+MAX_SLAB_DATA; // The user data

        u8* new_slab_start = ((u8*)slab_data.current_slab_address);
        
        // Does the new slab address location exceeds
        // our slab page count limit?
        if ((usize)new_slab_start >= (usize)slab_end)
        {
            kprintf("Slab: Slab limit exceeded. Cannot make new slabs\n");
            return false;
        }

        *slab = (slab_chunk_t*)new_slab_start;
        slab_data.current_slab_address+=stride;
    }

    // In this scenario, initializing a bitmap buffer
    // is a to-go
    // In the slab reclaim function, we have zeroed everything out

    u8* buffer = (u8*)*slab+sizeof(slab_chunk_t);

    // We still gonna need to allocate the right size for the slab
    // Because if we just set this to 64-bit map, we will actually get
    // 64 free pointers regardless of size, which can go very
    // very wrong and waste memory thin (e.g. 512B or 256B pointer).
    // The rest of the buffer can be reserved and set to 0.

    bitmap_init(&(*slab)->slab_bitmap, size, buffer);
    
    (*slab)->attributes.magic = SLAB_ALLOC_MAGIC;
    (*slab)->attributes.size = size;

    return true;
}

static void slab_alloc_push(slab_chunk_t** head, slab_chunk_t** tail, slab_chunk_t* slab)
{
    if (!tail || !head || !slab) return;

    if (!*tail) 
    {
        slab->prev = NULL;
        slab->next = NULL;
        *tail = slab;
        *head = slab;
        return;
    }

    slab->prev = *tail;
    (*tail)->next = slab;
    *tail = slab;
}

static void slab_alloc_remove(slab_chunk_t** head, slab_chunk_t** tail, slab_chunk_t* slab)
{
    if (!tail || !head || !slab) return;

    if (slab->prev)
    slab->prev->next = slab->next;
    else
    *head = slab->next;

    if (slab->next)
    slab->next->prev = slab->prev;
    else
    *tail = slab->prev;

    slab->next = NULL;
    slab->prev = NULL;
}

static void slab_alloc_pop(slab_chunk_t** head, slab_chunk_t** tail)
{
    if (!tail || !*tail) return NULL;

    slab_chunk_t* popped = *tail;

    if (popped->prev)
    {
        *tail = popped->prev;
        (*tail)->next = NULL;

    }
    else
    {
        *head = NULL;
        *tail = NULL;
    }
    popped->prev = NULL;
    popped->next = NULL;
}