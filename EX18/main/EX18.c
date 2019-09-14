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
/**
 * Logs
 */
#include "esp_system.h"
#include "esp_log.h"

/**
 * Definições
 */
#define DEBUG 1

/**
 * Variáveis
 */
static const char *TAG = "main: ";


static void vTask1( void * pvParameters );
static void vTask2( void * pvParameters );

SemaphoreHandle_t xSemaphore;

void app_main( void ) 
{
    
    xSemaphore = xSemaphoreCreateBinary();
	
    if( xSemaphore != NULL )
    {    

        xTaskCreate( vTask1, "Task 1", 2048, NULL, 1, NULL );
        xTaskCreate( vTask2, "Task 2", 2048, NULL, 1, NULL );
		
    }

}
/*-----------------------------------------------------------*/

static void vTask1( void * pvParameters )
{

	if( DEBUG )
		ESP_LOGI( TAG, "vTask1 run.\n" );
			
    for(;;)
    {
         if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
			if( DEBUG )
				ESP_LOGI( TAG, "vTask1.\n" );
        }
        xSemaphoreGive(xSemaphore);
		
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}


static void vTask2( void * pvParameters )
{

	if( DEBUG )
		ESP_LOGI( TAG, "vTask1 run.\n" );
    for(;;)
    {
         if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
			if( DEBUG )
				ESP_LOGI( TAG, "vTask2.\n" );
        }
        xSemaphoreGive(xSemaphore);
		
		vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}