ENTRY(entry16)
OUTPUT_FORMAT("binary")
phys = 0x1800;

SECTIONS
{
    . = phys;
    .entry              : { __entry_start = .;      *(.entry)   }
    .text               : { __text_start = .;       *(.text)    }
    .data               : { __data_start = .;       *(.data)    }
    .rodata             : { __rodata_start = .;     *(.rodata)  }
    .bss                : { __bss_start = .;        *(.bss)     }
    .stack ALIGN(16) (NOLOAD) :
    {
        __stack_start = .;
        *(.stack)
        . += 0x8000;
        __stack_end = .;
    }

    __end = .;
}