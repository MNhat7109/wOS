bits 32

global i686_panic
i686_panic:
    cli
    hlt

global i686_enable_interrupt
i686_enable_interrupt:
    sti
    ret

global i686_disable_interrupt
i686_disable_interrupt:
    cli
    ret

global i686_halt
i686_halt:
    hlt
    ret