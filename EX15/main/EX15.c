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
/**
 * Logs
 */
#include "esp_system.h"
#include "esp_log.h"

/**
 * Definições
 */
#define DEBUG 1

/* Definitions for the event bits in the event group. */
#define mainFIRST_TASK_BIT	( 1UL << 0UL ) /* Event bit 0, which is set by the first task. */ //0x01
#define mainSECOND_TASK_BIT	( 1UL << 1UL ) /* Event bit 1, which is set by the second task. *///0x02
#define mainTHIRD_TASK_BIT	( 1UL << 2UL ) /* Event bit 2, which is set by the third task. */ //0x03

/**
 * Protótipos
 */
void task_sync( void *pvParameters );

/**
 * Variáveis
 */
static const char *TAG = "main: ";
EventGroupHandle_t xEventGroup;


void task_sync( void *pvParameters )
{

	if( DEBUG )
		ESP_LOGI( TAG, "task_sync run.\r\n" );	

	const EventBits_t uxAllSyncBits = ( mainFIRST_TASK_BIT | mainSECOND_TASK_BIT | mainTHIRD_TASK_BIT );
	const TickType_t xMaxDelay = pdMS_TO_TICKS( 4000UL );
	const TickType_t xMinDelay = pdMS_TO_TICKS( 200UL );

	TickType_t xDelayTime;
	EventBits_t uxThisTasksSyncBit;

	/* Three instances of this task are created - each task uses a different
	event bit in the synchronization.  The event bit to use by this task
	instance is passed into the task using the task's parameter.  Store it in
	the uxThisTasksSyncBit variable. */
	uxThisTasksSyncBit = ( EventBits_t ) pvParameters;

	for( ;; )
	{
		/* Simulate this task taking some time to perform an action by delaying
		for a pseudo random time.  This prevents all three instances of this
		task from reaching the synchronization point at the same time, and
		allows the example's behavior to be observed more easily. */
		xDelayTime = ( esp_random() % xMaxDelay ) + xMinDelay;
		vTaskDelay( xDelayTime );

		/* Print out a message to show this task has reached its synchronization
		point.  pcTaskGetTaskName() is an API function that returns the name
		assigned to the task when the task was created. */
		if( DEBUG )
			ESP_LOGI( TAG, "%s reached sync point", pcTaskGetTaskName( NULL ) );

		/* Wait for all the tasks to have reached their respective
		synchronization points. */
		xEventGroupSync( /* The event group used to synchronize. */
						 xEventGroup,

						 /* The bit set by this task to indicate it has reached
						 the synchronization point. */
						 uxThisTasksSyncBit,

						 /* The bits to wait for, one bit for each task taking
						 part in the synchronization. */
						 uxAllSyncBits,

						 /* Wait indefinitely for all three tasks to reach the
						 synchronization point. */
						 portMAX_DELAY );

		/* Print out a message to show this task has passed its synchronization
		point.  As an indefinite delay was used the following line will only be
		reached after all the tasks reached their respective synchronization
		points. */
		if( DEBUG )
			ESP_LOGI( TAG, "%s exited sync point", pcTaskGetTaskName( NULL ) );

	}
}

/**
 * main da aplicação;
 */
void app_main( void ) 
{

	if( DEBUG )
		ESP_LOGI( TAG, "app_main run.\r\n" );	

	xEventGroup = xEventGroupCreate();

	xTaskCreate( task_sync, "Task 1", configMINIMAL_STACK_SIZE + 2048, ( void * ) mainFIRST_TASK_BIT, 1, NULL );
	xTaskCreate( task_sync, "Task 2", configMINIMAL_STACK_SIZE + 2048, ( void * ) mainSECOND_TASK_BIT, 1, NULL );
	xTaskCreate( task_sync, "Task 3", configMINIMAL_STACK_SIZE + 2048, ( void * ) mainTHIRD_TASK_BIT, 1, NULL );

}


