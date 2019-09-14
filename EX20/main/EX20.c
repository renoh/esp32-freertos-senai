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


/* The task function. */
void vTaskFunction( void *pvParameters );
static void vApplicationIdleHook( void );

/* A variable that is incremented by the idle task hook function. */
static uint32_t ulIdleCycleCount = 0UL;

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
const char *pcTextForTask1 = "Task 1";
const char *pcTextForTask2 = "Task 2";

void app_main( void ) 
{
    /* registra a função de callback em idle_hook do CORE que executa app_main */
    esp_register_freertos_idle_hook(&vApplicationIdleHook_new);
	
	/* Create the first task at priority 1... */
	xTaskCreate( vTaskFunction, "Task 1", configMINIMAL_STACK_SIZE + 2048, (void*)pcTextForTask1, 1, NULL );

	/* ... and the second task at priority 2.  The priority is the second to
	last parameter. */
	xTaskCreate( vTaskFunction, "Task 2", configMINIMAL_STACK_SIZE + 2048, (void*)pcTextForTask2, 2, NULL );

}
/*-----------------------------------------------------------*/

void vTaskFunction( void *pvParameters )
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
		ESP_LOGI( TAG, "task_name=%s .Cycles=%d\n", pcTaskName, ulIdleCycleCount );

		/* Delay for a period.  This time we use a call to vTaskDelay() which
		puts the task into the Blocked state until the delay period has expired.
		The delay period is specified in 'ticks'. */
		vTaskDelay( xDelay2250ms );
	}
}
/*-----------------------------------------------------------*/

/* Idle hook functions MUST be called vApplicationIdleHook(), take no parameters,
and return void. */
static void vApplicationIdleHook_new( void )
{
	/* This hook function does nothing but increment a counter. */
	ulIdleCycleCount++;
}


