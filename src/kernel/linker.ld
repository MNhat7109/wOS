ENTRY(start)
OUTPUT_FORMAT("elf32-i386")
phys = 0x100000;

SECTIONS
{
    . = phys; __low_start = .;
    .entry              : { __entry_start = .;      *(.entry)   }
    
    . += 0xC0000000; __start = .;
    .text ALIGN(4K)       : AT (ADDR(.text) - 0xC0000000)
    { 
        __text_start = .;       
        *(.text)    
        __text_end = .;       
    }
    .data ALIGN(4K)       : AT (ADDR(.data) - 0xC0000000)
    { __data_start = .;       *(.data)    }
    .rodata ALIGN(4K)     : AT (ADDR(.rodata) - 0xC0000000)
    { 
        __rodata_start = .;     
        *(.rodata)
        __rodata_end = .;     
    }
    .bss ALIGN(4K)        : AT (ADDR(.bss) - 0xC0000000)
    { 
        __bss_start = .;        
        *(.bss)     
        *(COMMON)
        __bss_end = .;
    }
    .stack ALIGN(16) (NOLOAD) :
    {
        __stack_start = .;
        . += 0x4000;
        __stack_end = .;
    }
    
    __end = .;
}