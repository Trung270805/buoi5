.syntax unified
.cpu cortex-m3
.thumb

.global Reset_Handler
.global Default_Handler

.extern main

/* FreeRTOS / Cortex-M3 handlers */
.extern SVC_Handler
.extern PendSV_Handler
.extern SysTick_Handler

.extern _estack
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss


/* =========================================================
 * Vector table
 * ========================================================= */

.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object

g_pfnVectors:

    /*
     * Initial stack pointer
     */
    .word _estack

    /*
     * Reset
     */
    .word Reset_Handler

    /*
     * Cortex-M3 core exceptions
     */
    .word Default_Handler      /* NMI */
    .word Default_Handler      /* HardFault */
    .word Default_Handler      /* MemManage */
    .word Default_Handler      /* BusFault */
    .word Default_Handler      /* UsageFault */

    .word 0
    .word 0
    .word 0
    .word 0

    /*
     * FreeRTOS handlers
     */
    .word SVC_Handler
    .word Default_Handler      /* Debug Monitor */
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /*
     * External interrupts
     */
    .rept 59
        .word Default_Handler
    .endr

.size g_pfnVectors, . - g_pfnVectors


/* =========================================================
 * Reset Handler
 * ========================================================= */

.section .text.Reset_Handler
.type Reset_Handler, %function

Reset_Handler:

    /*
     * -----------------------------------------------------
     * Copy .data from FLASH to RAM
     * -----------------------------------------------------
     */

    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

copy_data:

    cmp r1, r2
    bcc copy_data_word
    b clear_bss

copy_data_word:

    ldr r3, [r0]
    str r3, [r1]

    adds r0, r0, #4
    adds r1, r1, #4

    b copy_data


    /*
     * -----------------------------------------------------
     * Clear .bss
     * -----------------------------------------------------
     */

clear_bss:

    ldr r1, =_sbss
    ldr r2, =_ebss

    movs r3, #0

clear_bss_loop:

    cmp r1, r2
    bcc clear_bss_word
    b call_main

clear_bss_word:

    str r3, [r1]

    adds r1, r1, #4

    b clear_bss_loop


    /*
     * -----------------------------------------------------
     * Call main()
     * -----------------------------------------------------
     */

call_main:

    bl main


    /*
     * -----------------------------------------------------
     * main() should never return
     * -----------------------------------------------------
     */

hang:

    b hang

.size Reset_Handler, . - Reset_Handler


/* =========================================================
 * Default Handler
 * ========================================================= */

.section .text.Default_Handler
.type Default_Handler, %function

Default_Handler:

default_loop:

    b default_loop

.size Default_Handler, . - Default_Handler
