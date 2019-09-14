/**
 * Lib C
 */
#include <stdio.h>
/**
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
/**
 * Logs
 */
#include "esp_system.h"
#include "esp_log.h"

/**
 * Hooks
 */
#include "esp_freertos_hooks.h"

/**
 * Definições
 */
#define DEBUG 1

/**
 * Variáveis
 */
static const char *TAG = "main: ";
static uint32_t ulIdleHooks_core_0_CycleCount = 0UL;
static uint32_t ulIdleHooks_core_1_CycleCount = 0UL;
static uint32_t ulTickHooks_core_0_CycleCount = 0UL;
static uint32_t ulTickHooks_core_1_CycleCount = 0UL;

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
const char *pcTextForTask1 = "vTaskPrint_IdleHook";
const char *pcTextForTask2 = "vTaskPrint_TickHook";

/**
 * Protótipos
 */
static void vApplicationIdleHook_core_0( void );
static void vApplicationIdleHook_core_1( void );
static void vApplicationTickHook_core_0( void );
static void vApplicationTickHook_core_1( void );
void vTaskPrint_IdleHook( void *pvParameters );
void vTaskPrint_TickHook( void *pvParameters );


/*-----------------------------------------------------------*/

void vTaskPrint_IdleHook( void *pvParameters )
{
	char *pcTaskName;
	const TickType_t xDelay2250ms = pdMS_TO_TICKS( 2250UL );

	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = ( char * ) pvParameters;

	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Print out the name of this task AND the number of times ulIdleCycleCount
        has been incremented. */
		ESP_LOGI( TAG, "task_name=%s .CORE 0: IdleHookCycles=%d\n", pcTaskName, ulIdleHooks_core_0_CycleCount );
		ESP_LOGI( TAG, "task_name=%s .CORE 1: IdleHookCycles=%d\n", pcTaskName, ulIdleHooks_core_1_CycleCount );
		
		/* Delay for a period.  This time we use a call to vTaskDelay() which
		puts the task into the Blocked state until the delay period has expired.
		The delay period is specified in 'ticks'. */
		vTaskDelay( xDelay2250ms );
	}
}

/*-----------------------------------------------------------*/

void vTaskPrint_TickHook( void *pvParameters )
{
	char *pcTaskName;
	const TickType_t xDelay2250ms = pdMS_TO_TICKS( 2250UL );

	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = ( char * ) pvParameters;

	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Print out the name of this task AND the number of times ulIdleCycleCount
        has been incremented. */
		ESP_LOGI( TAG, "task_name=%s .CORE 0: TickHookCycles=%d\n", pcTaskName, ulTickHooks_core_0_CycleCount );
		ESP_LOGI( TAG, "task_name=%s .CORE 1: TickHookCycles=%d\n", pcTaskName, ulTickHooks_core_1_CycleCount );
		
		/* Delay for a period.  This time we use a call to vTaskDelay() which
		puts the task into the Blocked state until the delay period has expired.
		The delay period is specified in 'ticks'. */
		vTaskDelay( xDelay2250ms );
	}
}

/*-----------------------------------------------------------*/
/**
 * Função de Callback chamada pela Task Idle do core 0; 
 */
static void vApplicationIdleHook_core_0( void )
{
	ulIdleHooks_core_0_CycleCount++;
}

/**
 * Função de Callback chamada pela Task Idle do core 1; 
 */
static void vApplicationIdleHook_core_1( void )
{
	ulIdleHooks_core_1_CycleCount++;
}

/*-----------------------------------------------------------*/
/**
 * Atenção: TickHook é a função de callback de uma ISR de Timer!
 * Portanto, não devemos utilizar nenhuma função da API do FreeRTOS bloqueante;
 */
static void vApplicationTickHook_core_0( void )
{
	ulTickHooks_core_0_CycleCount++;
}

/*-----------------------------------------------------------*/
static void vApplicationTickHook_core_1( void )
{
	ulTickHooks_core_1_CycleCount++;
}


/**
 * Início da Aplicação
 */
void app_main( void ) 
{

    esp_register_freertos_idle_hook_for_cpu(&vApplicationIdleHook_core_0, 0);
	esp_register_freertos_idle_hook_for_cpu(&vApplicationIdleHook_core_1, 1);
	esp_register_freertos_tick_hook_for_cpu(&vApplicationTickHook_core_0, 0);
	esp_register_freertos_tick_hook_for_cpu(&vApplicationTickHook_core_1, 1);
	
	
	xTaskCreate( vTaskPrint_IdleHook, "Task 1", configMINIMAL_STACK_SIZE + 2048, (void*)pcTextForTask1, 1, NULL );
	xTaskCreate( vTaskPrint_TickHook, "Task 2", configMINIMAL_STACK_SIZE + 2048, (void*)pcTextForTask2, 1, NULL );

}

