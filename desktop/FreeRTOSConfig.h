#ifndef M_DESKTOP_FREERTOS_CONFIG_H_
#define M_DESKTOP_FREERTOS_CONFIG_H_

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     1
#define configCPU_CLOCK_HZ                      1000000
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                1024
#define configTOTAL_HEAP_SIZE                   (1024 * 1024)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_TIMERS                        1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0

#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1

#define configKERNEL_INTERRUPT_PRIORITY         255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    191

#define configUSE_TICK_HOOK 1

#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS

#define configTIMER_TASK_PRIORITY        2
#define configTIMER_QUEUE_LENGTH         10
#define configTIMER_TASK_STACK_DEPTH     2048

#define INCLUDE_vTaskDelay 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_xTaskDelayUntil 1

#define configUSE_RECURSIVE_MUTEXES 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

#endif
