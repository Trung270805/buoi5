ENTRY(Reset_Handler)

/* =========================================================
 * STM32F103C8T6
 *
 * Flash = 64 KB
 * RAM   = 20 KB
 * ========================================================= */

_estack = ORIGIN(RAM) + LENGTH(RAM);

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* =========================================================
 * Sections
 * ========================================================= */

SECTIONS
{
    /* -----------------------------------------------------
     * Vector table
     * ----------------------------------------------------- */

    .isr_vector :
    {
        . = ALIGN(4);

        KEEP(*(.isr_vector))

        . = ALIGN(4);
    } > FLASH


    /* -----------------------------------------------------
     * Program code + read-only data
     * ----------------------------------------------------- */

    .text :
    {
        . = ALIGN(4);

        *(.text)
        *(.text*)

        *(.rodata)
        *(.rodata*)

        . = ALIGN(4);

        _etext = .;
    } > FLASH


    /* -----------------------------------------------------
     * ARM init/fini arrays
     * ----------------------------------------------------- */

    .preinit_array :
    {
        . = ALIGN(4);

        PROVIDE_HIDDEN(__preinit_array_start = .);

        KEEP(*(.preinit_array*))

        PROVIDE_HIDDEN(__preinit_array_end = .);

        . = ALIGN(4);
    } > FLASH


    .init_array :
    {
        . = ALIGN(4);

        PROVIDE_HIDDEN(__init_array_start = .);

        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))

        PROVIDE_HIDDEN(__init_array_end = .);

        . = ALIGN(4);
    } > FLASH


    .fini_array :
    {
        . = ALIGN(4);

        PROVIDE_HIDDEN(__fini_array_start = .);

        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))

        PROVIDE_HIDDEN(__fini_array_end = .);

        . = ALIGN(4);
    } > FLASH


    /* -----------------------------------------------------
     * Initialized data
     *
     * Stored in FLASH
     * Copied to RAM at reset
     * ----------------------------------------------------- */

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);

        _sdata = .;

        *(.data)
        *(.data*)

        . = ALIGN(4);

        _edata = .;
    } > RAM AT > FLASH


    /* -----------------------------------------------------
     * Zero initialized data
     * ----------------------------------------------------- */

    .bss :
    {
        . = ALIGN(4);

        _sbss = .;

        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);

        _ebss = .;
    } > RAM


    /* -----------------------------------------------------
     * Heap / Stack area
     * ----------------------------------------------------- */

    ._user_heap_stack :
    {
        . = ALIGN(8);

        PROVIDE(end = .);
        PROVIDE(_end = .);

        . = . + 0x1000;

        . = . + 0x1000;

        . = ALIGN(8);
    } > RAM


    /* -----------------------------------------------------
     * Discard unused libc sections
     * ----------------------------------------------------- */

    /DISCARD/ :
    {
        libc.a(*)
        libm.a(*)
        libgcc.a(*)
    }
}
