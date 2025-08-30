#include <libk/utils/algs.h>
#include <libk/bitmanip/bitmanip.h>

static void insertion_sort(
    u8* base, 
    usize n, 
    usize item_size,
    int (*cmp)(const void*, const void*)
);

static void heapify(
    u8* base, 
    usize n, 
    usize i, 
    usize item_size,
    int (*cmp)(const void*, const void*)
);

static void heap_sort(
    u8* base, 
    usize n, 
    usize item_size,
    int (*cmp)(const void*, const void*)
);

static u8* partition(
    u8* low, 
    u8* high, 
    usize item_size,
    int (*cmp)(const void*, const void*)
);

static void introsort(
    u8* base, 
    usize n, 
    usize item_size,
    usize depth_limit,
    int (*cmp)(const void*, const void*)
);

void swap_n(
    u8* a,
    u8* b,
    usize item_size
)
{
    while (item_size--)
    {
        u8 tmp = *a;
        *a++ = *b;
        *b++ = tmp;
    } 
}

void sort(
    void* base,
    usize n,
    usize item_size,
    int (*cmp)(const void*, const void*)
)
{
    // Doing this because I'm too lazy to implement log2
    usize recursion_depth = sizeof(usize)*8-1 - clz(n);

    introsort((u8*)base, n, item_size, recursion_depth, cmp);
}

//////////////////////////////////////////////
//////////////////////////////////////////////
/* STATIC FUNCTIONS */
//////////////////////////////////////////////
//////////////////////////////////////////////

static void introsort(
    u8* base, 
    usize n, 
    usize item_size,
    usize depth_limit,
    int (*cmp)(const void*, const void*)
)
{
    while (n < 16)
    {
        if (depth_limit == 0)
        {
            heap_sort(base, n, item_size, cmp);
            return;
        }
        depth_limit--;

        u8* low = (u8*)base;
        u8* hi = base + (n-1)*item_size;
        u8* part = partition(low, hi, item_size, cmp);

        usize left_size = (part-(u8*)base) / item_size;
        usize right_size = n-left_size-1;

        if (left_size < right_size)
        {
            introsort(base, left_size, item_size, depth_limit, cmp);
            base = part+item_size;
            n = right_size;
        }
        else
        {
            introsort(base, right_size, item_size, depth_limit, cmp);
            n = left_size;
        }
    }
    insertion_sort(base, n, item_size, cmp);
}

static u8* partition(
    u8* low, 
    u8* high, 
    usize item_size,
    int (*cmp)(const void*, const void*)
)
{
    u8* pivot = high;
    u8* i = low-item_size;

    for (u8* j = low; j < high; j+=item_size)
    {
        if (cmp(j, pivot) <= 0)
        {
            i+=item_size;
            swap_n(i, j, item_size);
        }
    }

    i+=item_size;
    swap_n(i, high, item_size);
}

//////////////////////////////////////////////////////////

static void heapify(
    u8* base, 
    usize n, 
    usize i, 
    usize item_size,
    int (*cmp)(const void*, const void*)
)
{
    usize largest = i; 
    usize left = (i<<1)+1;
    usize right = (i<<1)+2;

    u8* largest_ptr = base+largest*item_size;
    u8* left_ptr = base+left*item_size;
    u8* right_ptr = base+right*item_size;

    if (left < n && cmp(left_ptr, largest_ptr) > 0)
        largest = left;
    
    if (right < n && cmp(left_ptr, base+largest*item_size) > 0)
        largest = right;

    if (largest != i)
    {
        swap_n(base+i*item_size, base+largest*item_size, item_size);
        heapify(base, n, largest, item_size, cmp);
    }
}

static void heap_sort(
    u8* base, 
    usize n, 
    usize item_size,
    int (*cmp)(const void*, const void*)
)
{
    for (isize i = (n>>1)-1; i>=0;i--)
        heapify(base, n, i, item_size, cmp);

    for (isize i = n-1; i>0;i--)
    {
        swap_n(base, base+i*item_size, item_size);
        heapify(base, i, 0, item_size, cmp);
    }
}

////////////////////////////////////////////////////////

static void insertion_sort(
    u8* base, 
    usize n, 
    usize item_size,
    int (*cmp)(const void*, const void*)
)
{
    for (usize i=1;i<n;i++)
    {
        for (usize j=i; j>0; j--)
        {
            u8* a = base + (j-1)*item_size;
            u8* b = base + j*item_size;
            if (cmp(a,b) <= 0) break;

            swap_n(a, b, item_size);
        }
    }
}