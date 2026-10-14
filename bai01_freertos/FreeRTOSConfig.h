#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

/*-----------------------------------------------------------
 * CPU
 *----------------------------------------------------------*/

#define configCPU_CLOCK_HZ                    ((uint32_t)8000000)

/*-----------------------------------------------------------
 * Scheduler
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION                  1
#define configUSE_IDLE_HOOK                   0
#define configUSE_TICK_HOOK                   0

#define configTICK_RATE_HZ                    ((TickType_t)1000)

#define configMAX_PRIORITIES                  5
#define configMINIMAL_STACK_SIZE              ((uint16_t)128)
#define configMAX_TASK_NAME_LEN               16

#define configUSE_16_BIT_TICKS                0
#define configIDLE_SHOULD_YIELD               1

/*-----------------------------------------------------------
 * Memory allocation
 *----------------------------------------------------------*/

#define configSUPPORT_DYNAMIC_ALLOCATION      1
#define configSUPPORT_STATIC_ALLOCATION       0

#define configTOTAL_HEAP_SIZE                 ((size_t)(8 * 1024))

/*-----------------------------------------------------------
 * Synchronization
 *----------------------------------------------------------*/

#define configUSE_MUTEXES                     1
#define configUSE_RECURSIVE_MUTEXES           0
#define configUSE_COUNTING_SEMAPHORES          1

/*-----------------------------------------------------------
 * Software timer
 * Không dùng trong bài này
 *----------------------------------------------------------*/

#define configUSE_TIMERS                      0

/*-----------------------------------------------------------
 * Hook functions
 *----------------------------------------------------------*/

#define configCHECK_FOR_STACK_OVERFLOW        0
#define configUSE_MALLOC_FAILED_HOOK          0

/*-----------------------------------------------------------
 * API functions used by the application
 *----------------------------------------------------------*/

#define INCLUDE_vTaskDelay                    1
#define INCLUDE_vTaskSuspend                  1
#define INCLUDE_vTaskDelete                   1
#define INCLUDE_xTaskGetSchedulerState        1

/*-----------------------------------------------------------
 * Cortex-M3 interrupt priority
 *----------------------------------------------------------*/

#define configPRIO_BITS                       4

#define configKERNEL_INTERRUPT_PRIORITY       \
    (15 << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY  \
    (5 << (8 - configPRIO_BITS))

/*-----------------------------------------------------------
 * Assert
 *----------------------------------------------------------*/

#define configASSERT(x)                       \
    if ((x) == 0)                             \
    {                                         \
        taskDISABLE_INTERRUPTS();             \
        for (;;)                              \
        {                                     \
        }                                     \
    }

/*-----------------------------------------------------------
 * FreeRTOS Cortex-M3 handlers
 *----------------------------------------------------------*/

#define vPortSVCHandler                       SVC_Handler
#define xPortPendSVHandler                    PendSV_Handler
#define xPortSysTickHandler                   SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
