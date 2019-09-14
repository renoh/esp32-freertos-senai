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
#define mainSECOND_TASK_BIT	( 1UL << 1UL ) /* Event bit 1, which is set by the second task. */ //0x02

const EventBits_t uxAllSyncBits = ( mainFIRST_TASK_BIT | mainSECOND_TASK_BIT ); //0x03

/**
 * Protótipos
 */
void vTask1( void * pvParameters );
void vTask2( void * pvParameters );
void vTask3( void * pvParameters );
void app_main( void );

/**
 * Variáveis
 */
static const char *TAG = "main: ";
EventGroupHandle_t xEventGroup;

/**
 * task mainFIRST_TASK_BIT
 */
void vTask1( void * pvParameters )
{
	if( DEBUG )
		ESP_LOGI( TAG, "vTask1 run.\r\n" );	

    for(;;)
    {
        xEventGroupSetBits( xEventGroup, mainFIRST_TASK_BIT );
        vTaskDelay( 250/portTICK_PERIOD_MS );
    }
}


/**
 * task mainSECOND_TASK_BIT
 */
void vTask2( void * pvParameters )
{
	if( DEBUG )
		ESP_LOGI( TAG, "vTask2 run.\r\n" );	

    for(;;)
    {
         xEventGroupSetBits( xEventGroup, mainSECOND_TASK_BIT );
         vTaskDelay( 250/portTICK_PERIOD_MS );
    }
}

/**
 * task mainSECOND_TASK_BIT
 */
void vTask3( void * pvParameters )
{
	BaseType_t  result;
    const TickType_t xDelay2000ms = pdMS_TO_TICKS( 2000UL );

	if( DEBUG )
		ESP_LOGI( TAG, "vTask3 run.\r\n" );	
    
    for(;;)
    {
        result = xEventGroupWaitBits(   xEventGroup,
                                        uxAllSyncBits, //0x03
                                        pdTRUE,
                                        pdFALSE, 
                                        xDelay2000ms );
        
        if( result == uxAllSyncBits )
        {

			if( DEBUG )
				ESP_LOGI( TAG, "Tasks Sincronizadas.\r\n" );	
            
        } else 

        if( ( result & mainFIRST_TASK_BIT ) )
        {
			if( DEBUG )
				ESP_LOGI( TAG, "mainFIRST_TASK_BIT Sincronizada.\r\n" );
        }
        if( ( result & mainSECOND_TASK_BIT ) )
        {
			if( DEBUG )
				ESP_LOGI( TAG, "mainSECOND_TASK_BIT Sincronizada.\r\n" );
        }
        
        
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

	xTaskCreate( vTask1, "Task 1", 2048, NULL, 1, NULL );
	xTaskCreate( vTask2, "Task 2", 2048, NULL, 1, NULL );
	xTaskCreate( vTask3, "Task 3", 4096, NULL, 2, NULL );

}


