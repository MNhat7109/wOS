#include <libk/mmu/buddy_allocator.h>
#include <libk/mmu/mmu_utils.h>
#include <libk/mmu/mmu_map.h>

#include <libk/bitmap/bitmap.h>
#include <libk/string.h>
#include <libk/stdio.h>
#include <stdbool.h>

#define MAX_BUDDY_LEVEL 64

#define BUDDY_META_LVL_INVALID (0xFF)

static usize buddy_find_larger_power_of_two(usize number, usize granularity);
static usize buddy_find_levels(usize total_size, usize granularity);
static usize buddy_find_total_bit_count(usize level_count);

static bool buddy_add_to_metadata(buddy_alloc_zone_t* zone, usize ptr_offset, u8 level);
static bool buddy_remove_from_metadata(buddy_alloc_zone_t* zone, usize ptr_offset);

static usize buddy_access_parent_node_idx(usize ptr_index, usize level);
static usize buddy_access_child_node_idx(usize ptr_index, usize level);

static u8 buddy_meta_get_level(buddy_alloc_zone_t* zone, usize ptr_offset);
static bool buddy_meta_is_valid(buddy_alloc_zone_t* zone, usize ptr_offset);

void buddy_alloc_init(buddy_alloc_zone_t* zone, void* address,
    usize length, usize granularity)
{
    if (length == 0) return;

    length = mmu_find_prev_po2(length);

    kprintf("Buddy: Initializing at address 0x%x, max size: %u\n", 
        address, length);

    zone->buddy_address = address;
    zone->buddy_total_size = length;
    zone->buddy_granularity = granularity;
    
    zone->meta_item_count = length/granularity;

    usize total_levels = buddy_find_levels(length, granularity);
    if (total_levels >= MAX_BUDDY_LEVEL) total_levels = MAX_BUDDY_LEVEL-1;
    usize bmp_bit_count = buddy_find_total_bit_count(total_levels);
    usize bmp_size = (bmp_bit_count+7)>>3;

    zone->buddy_max_level = total_levels;
    
    
    mmu_map_create_region(0, 
        zone->meta_item_count*sizeof(buddy_metadata_t)+bmp_size,
        MEMORY_TYPE_SYS_RESERVED
    );

    u8* data;
    // Metadata allocation
    zone->meta_start = (buddy_metadata_t*)data;

    // Bitmap allocation
    bitmap_init(&zone->buddy_bmp, bmp_bit_count, 
        data+zone->meta_item_count*sizeof(buddy_metadata_t)
    );
}

void buddy_alloc_destroy(buddy_alloc_zone_t* zone)
{
    kprintf("Buddy: Destroying buddy...\n");
    if (!zone->buddy_address) return;
    if (!zone->meta_start) return;
    if (!zone->buddy_bmp.buffer) return;
    
    memset(mmu_ptov(zone->meta_start), 0, zone->meta_item_count*sizeof(buddy_metadata_t));
    memset(mmu_ptov(zone->buddy_bmp.buffer), 0, zone->buddy_bmp.size);
    memset(mmu_ptov(zone->buddy_address), 0, zone->buddy_total_size);
    zone->buddy_address = NULL;
    zone->buddy_bmp.buffer = NULL;
    zone->meta_start = NULL;
}

void* buddy_alloc_request(buddy_alloc_zone_t* zone, usize size)
{
    if (size == 0) return NULL;

    usize fixed_size = buddy_find_larger_power_of_two(size, zone->buddy_granularity);
    usize desired_level = buddy_find_levels(zone->buddy_total_size, fixed_size)-1;
    usize current_size = zone->buddy_total_size;
    usize current_level = 0; // Start at root
    usize current_index = 0; // Start at root
    
    u8* buddy_virt = __va(u8*, zone->buddy_address);

    if (desired_level >= zone->buddy_max_level)
    {
        kprintf("Buddy: Buddy max level exceeded, cannot allocate further\n");
        return NULL;
    }

    while (current_size > fixed_size)
    {
        // Compute child index
        usize child_index = buddy_access_child_node_idx(current_index, current_level);
        usize left_index = child_index-1;
        usize right_index = child_index;

        // Is the left child free? Yes, then go to left. 
        // No, then is the right child used? Yes, then mark the parent. No, go to right.
        if (!bitmap_get_bits(&zone->buddy_bmp, left_index))
            current_index = left_index;
        else if (!bitmap_get_bits(&zone->buddy_bmp, right_index))
            current_index = right_index;
        else
        {
            // Mark the current, then backtrack to the parent.
            bitmap_set_bits(&zone->buddy_bmp, current_index);

            // If the root is used, yeah, out of memory.
            if (current_level == 0) break;

            // Backtrack to the parent node
            current_index = buddy_access_parent_node_idx(current_index, current_level);

            current_level--;
            current_size<<=1;
        }

        current_level++;
        current_size>>=1; 
    }

    if (fixed_size == current_size)
    {
        // Set the block as "used"
        bitmap_set_bits(&zone->buddy_bmp, current_index);

        // If the block is of the correct size, we return the pointer
        // Calculate the pointer offset

        // If we are to allocate at the root (level==0) for some reason
        // (The scenario where some jackasses try to allocate an object
        // that is the same size as their moms)
        // then the pointer offset must be 0.

        usize ptr_offset = (current_index-((1<<current_level)-1))*current_size;

        // Notify the client that we've successfully allocated
        // a pointer

        kprintf("Buddy: Successfully allocated object of size: %u bytes,"
            " at offset=0x%x\n", fixed_size, buddy_virt+ptr_offset);

        // Use the pointer's offset as the index in the meta region
        buddy_add_to_metadata(zone, ptr_offset, current_level);

        return (void*)(buddy_virt+ptr_offset);
    }
    
    // If we get to this point: Congrats, you're out of memory
    // To the jackasses mentioned above: yeah, your mom is that big.
    // In that case, we will show an error, and simply returns NULL. 

    kprintf("Buddy: Out of memory. Check if you've hogged up memory somewhere in your code.\n");
    return NULL;
}

void buddy_alloc_free(buddy_alloc_zone_t* zone, void* ptr)
{
    // Check if the object's pointer is in range from the buddy start address
    // If not, maybe its owner is not the buddy, but either the slab, or
    // a stray pointer

    u8* buddy_virt = __va(u8*, zone->buddy_address);

    if (ptr < buddy_virt)
    {
        kprintf("Buddy: Attempted to free a stray pointer. Check the heap API for bugs.\n");
        return;
    }

    // Compute the pointer's offset in the user's data
    usize ptr_offset = (u8*)ptr - buddy_virt;

    // Now check if the checksum is there
    // If it's not, it's likely to have been wiped out in the last call
    if (!buddy_meta_is_valid(zone, ptr_offset))
    {
        kprintf("Buddy: Double free detected\n");
        return;
    }

    // Get the level
    u8 ptr_level = buddy_meta_get_level(zone, ptr_offset);
    if (ptr_level == BUDDY_META_LVL_INVALID)
    {
        kprintf("Buddy: Metadata for this pointer is corrupted\n");
        return;
    }

    // Get the block size
    usize ptr_size = zone->buddy_total_size/(1<<ptr_level);

    // As you have read in the buddy_alloc_request(),
    // ptr_offset = (index - ((1<<level) - 1)) * block_size
    // Because the bits in a level in our bitmap always start
    // bit offset ((1<<level)-1) (zero-based bitmap)

    // Therefore, with level and block_size obtained, we can easily
    // calculate the pointer's bit offset.
    usize ptr_bit_index = ptr_offset/ptr_size + (1<<ptr_level) - 1;
    
    // Check once again, if the bit has already been cleared.
    // If so, the pointer has, once again, been double-freed.
    if (!bitmap_get_bits(&zone->buddy_bmp, ptr_bit_index))
    {
        kprintf("Buddy: Double free detected\n");
        return;
    }

    // If the mentioned check have been passed, the bit may now be
    // cleared
    bitmap_clear_bits(&zone->buddy_bmp, ptr_bit_index);

    // Now, remove any metadata that were previously tied to our object.
    // This will remove the "allocated" checkmark that we previously
    // have to check in line 178, ensuring that double-free pointers
    // can't get past that.

    buddy_remove_from_metadata(zone, ptr_offset);

    // Now, we come to the hard part:
    // Navigate to the pointer's parent for merging

    // For example: 
    // total size = 8192, min item size = 1024
    // Allocated two 1024-blocks
    // Before: [L0: 0] [L1: 0 0] [L2: 1 0 0 0] [L3: 1 1 0 0 0 0 0 0]
    // Free 1 1024-block
    // After: [L0: 0] [L1: 0 0] [L2: 0 0 0 0] [L3: 1 0 0 0 0 0 0 0]

    // If either one of the children is free, the parent node bit must be cleared, too.

    while (ptr_level)
    {
        // Access the peer node
        usize peer_bit_index;

        // The left node
        if (ptr_bit_index & 1) peer_bit_index = ptr_bit_index+1;
        // The right node
        else peer_bit_index = ptr_bit_index-1; 

        // Now check if either one of them is free, then clear the parent bit.
        usize parent_bit_index = buddy_access_parent_node_idx(ptr_bit_index, ptr_level);

        u8 peer_status = bitmap_get_bits(&zone->buddy_bmp, peer_bit_index);
        u8 ptr_status = bitmap_get_bits(&zone->buddy_bmp, ptr_bit_index);

        if (!peer_status || !ptr_status)
            bitmap_clear_bits(&zone->buddy_bmp, parent_bit_index);

        ptr_bit_index = parent_bit_index;
        ptr_level--;
    }
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

static usize buddy_find_levels(usize total_size, usize granularity)
{
    // To find the level of a 2^x buddy allocator,
    // Consider this question:
    // John has one long birch log of (total_size) meters.
    // Now, he chops that log as follows:
    // - Take 1: He chop the log in two halves.
    // - Take 2: He took each half and chop it to another new two halves.
    // - Take 3, 4, 5, etc.: same.
    
    // Question: How many takes does John need in order to get pieces
    // of MIN_ITEM_SIZE-meter of birch log?
    
    return mmu_log2(total_size/granularity)+1;
}

static usize buddy_find_total_bit_count(usize level_count)
{
    if (level_count >= MAX_BUDDY_LEVEL)
    {
        return 0;
    }

    // This is a bitmap based 2^x buddy allocator.
    // There will be a large bitmap and each buddy
    // (except for the root buddy, it uses only one bit) 
    // will use double the bits compared to its parents. 
    
    // For example:
    // Total size = 16384 bytes (16K)
    // Minimum allocable items = 1024 bytes (1K)

    // Level 0 (root) : 16384 bytes each => 1 bit
    // Level 1        : 8192 bytes each  => 2 bits
    // Level 2        : 4096 bytes each  => 4 bits
    // Level 3        : 2048 bytes each  => 8 bits
    // Level 4        : 1024 bytes each  => 16 bits

    // => Total bits used after 5 levels (0-4) = 1+2+4+8+16 = 31 bits
    // => Total bits used after k+1 levels (0-k) [S] = 1+2+4+8+...+2^k (bits)
    // => S = 1+1*2+1*2^2+1*2^3+...+1*2^k

    // We realize that this is a geometric sequence 
    // (see https://en.wikipedia.org/wiki/Geometric_progression), 
    // and we can retrieve:
    // u1 (first number in the series) = 1
    // q (the multiplier of the series) = 2 
    // => We can calculate the sum of bits as a sum of a geometric sequence
    // which has such attributes above: 
    // S(k) (sum of bits after k levels) = u1*(q^k-1)/(q-1)
    // => S(k) = 1*(2^k-1)/(2-1) = 2^k-1 (Our total bit count!)
    // Bitwise! : S(k) = (1<<k)-1

    return (1<<level_count)-1;
}

static usize buddy_find_larger_power_of_two(usize number, usize granularity)
{
    if (number < granularity) return granularity;

    return mmu_find_next_po2(number);
}

static bool buddy_add_to_metadata(buddy_alloc_zone_t* zone, usize ptr_offset, u8 level)
{
    u8* buddy_virt = __va(u8*, zone->buddy_address);
    buddy_metadata_t* meta_virt = __va(buddy_metadata_t*, zone->meta_start);
    
    // Divide the offset by granularity, in order to get the location
    // in our metadata
    // This metadata will behave like a map of our user data
    
    usize index = ptr_offset / zone->buddy_granularity;
    usize meta_entry_count = zone->meta_item_count;

    // Also, bound check!
    if (index >= zone->meta_item_count)
    {
        kprintf("Buddy: Metadata index of pointer 0x%x out of bound\n",
             buddy_virt+ptr_offset);
        return false;
    }
    
    // Set the level and the check
    meta_virt[index].buddy_info.level = level;
    meta_virt[index].buddy_info.check = 1;
    return true;
}

static bool buddy_remove_from_metadata(buddy_alloc_zone_t* zone, usize ptr_offset)
{
    u8* buddy_virt = __va(u8*, zone->buddy_address);
    buddy_metadata_t* meta_virt = __va(buddy_metadata_t*, zone->meta_start);
    
    // Divide the offset by granularity, in order to get the location
    // in our metadata
    // This metadata will behave like a map of our user data
    
    usize index = ptr_offset / zone->buddy_granularity;
    usize meta_entry_count = zone->meta_item_count;

    // Also, bound check!
    if (index >= meta_entry_count)
    {
        kprintf("Buddy: Metadata index of pointer 0x%x out of bound\n",
             buddy_virt+ptr_offset);
        return false;
    }
    
    meta_virt[index].buddy_info.level = 0;
    meta_virt[index].buddy_info.check = 0;
    return true;
}

static u8 buddy_meta_get_level(buddy_alloc_zone_t* zone, usize ptr_offset)
{
    u8* buddy_virt = __va(u8*, zone->buddy_address);
    buddy_metadata_t* meta_virt = __va(buddy_metadata_t*, zone->meta_start);
    
    // Divide the offset by granularity, in order to get the location
    // in our metadata
    // This metadata will behave like a map of our user data
    
    usize index = ptr_offset / zone->buddy_granularity;
    usize meta_entry_count = zone->meta_item_count;

    // Also, bound check!
    if (index >= meta_entry_count)
    {
        kprintf("Buddy: Metadata index of pointer 0x%x out of bound\n",
             buddy_virt+ptr_offset);
        return BUDDY_META_LVL_INVALID;
    }

    return meta_virt[index].buddy_info.level;
}

static bool buddy_meta_is_valid(buddy_alloc_zone_t* zone, usize ptr_offset)
{
    u8* buddy_virt = __va(u8*, zone->buddy_address);
    buddy_metadata_t* meta_virt = __va(buddy_metadata_t*, zone->meta_start);
    
    // Divide the offset by granularity, in order to get the location
    // in our metadata
    // This metadata will behave like a map of our user data
    
    usize index = ptr_offset / zone->buddy_granularity;
    usize meta_entry_count = zone->meta_item_count;

    // Also, bound check!
    if (index >= meta_entry_count)
    {
        kprintf("Buddy: Metadata index of pointer 0x%x out of bound\n",
             buddy_virt+ptr_offset);
        return BUDDY_META_LVL_INVALID;
    }

    return meta_virt[index].buddy_info.check;
}

static usize buddy_access_child_node_idx(usize ptr_index, usize level)
{
    // Here, because of the special structure of the bitmap (to be exact, 
    // a binary tree stored in a bitmap) that goes:
    // - L0 (level 0) :
    // -- Bit 0 (Zero-based bitmap, so subtract by 1: (1<<0)-1 = 0)
    // - L1:
    // -- Bit index left (parent bit 0) = ((0 >> 0) << 1) + (1 << 1) - 1 = 1
    // -- bit index right (parent bit 0) = ((0 >> 0) << 1) + (1 << 1)    = 2
    // - L2: 
    // -- Bit index left (parent bit 1) = ((1 >> 1) << 1) + (1 << 2) - 1 = 3
    // -- bit index right (parent bit 1) = ((1 >> 1) << 1) + (1 << 2)    = 4
    // -- Bit index left (parent bit 2) = ((2 >> 1) << 1) + (1 << 2) - 1 = 5
    // -- bit index right (parent bit 2) = ((2 >> 1) << 1) + (1 << 2)    = 6
    // And so on....

    // We can generalize the formula to find the exact bit location of our object
    // at level k as:
    // loffset(k) = ((poffset>>(k-1))<<1) + (1<<k) - 1
    // And:
    // roffset(k) = ((poffset>>(k-1))<<1) + (1<<k)
    // Where:
    // - k is the current level of the children 
    // - poffset, loffset, roffset are respectively the bit offset of
    // the parent node, its left child node and its right child node

    // (Gee, this thing takes me like 5 hours and a lunch nap to figure out)

    return ((ptr_index>>level)<<1)+(1<<(level+1));
}

static usize buddy_access_parent_node_idx(usize ptr_index, usize level)
{
    // Here, you can see in our buddy binary tree, (aside from the root)
    // every left child node's bit index is odd and every right child node's
    // is even, so we will first check if our current index is a left or a right.
    // If it's left (bit index being an odd number), we will increase that by 1
    // to get to the right node's index.
    // Only in the right node's index, will you be able to backtrack the index to
    // the parent's.

    if (ptr_index&1) ptr_index++;

    // Here, because roffset = ((poffset>>plevel)<<1) + (1<<(plevel+1)) where
    // poffset and roffset are respectively the parent node and its right child's
    // bit offsets, plevel is the parent's level, and you guessed it,
    // (plevel+1) is the right child's level,
    // we can flip the logic and calculate the parent's bit, given the current bit offset
    // and the current level:
    // poffset = ((roffset - (1<<(current_level)))>>1)<<(current_level-1)

    return ((ptr_index-(1<<level))>>1)<<(level-1);
}