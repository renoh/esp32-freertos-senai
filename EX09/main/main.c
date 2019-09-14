/*
 * Lib C
 */
#include <stdio.h>
#include <stdint.h>  
#include <string.h>

/*
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/*
 * Drivers
 */
#include "driver/gpio.h"

/*
 * logs
 */
#include "esp_log.h"

/*
 * Definições Gerais
 */
#define DEBUG 1

/*
 * Variável Global. 
 */
static const char * TAG = "MAIN: ";
TaskHandle_t xTask2Handle;

/**
 * Task Responsável pelo incremento e envio de count_1 na fila;
 */
void task_1( void * pvParameter )
{
	uint32_t count_1 = 0;
    UBaseType_t uxPriority;

	if( DEBUG )
		ESP_LOGI( TAG, "\n\n task_1 run..." );

	uxPriority = uxTaskPriorityGet( NULL );

    for(;;)
	{
		
		if( DEBUG )
			ESP_LOGI( TAG, "\n\n task_1 count = %d.", count_1 );
		
		count_1++; 

		vTaskPrioritySet( xTask2Handle, ( uxPriority + 1 ) );

    }

	/**
	 * Este comando não deve ser executado...
	 */
    vTaskDelete(NULL);
}

/**
 * Task Responsável pelo incremento e envio de count_2 na fila;
 */
void task_2( void * pvParameter )
{
	uint32_t count_2 = 0;
	UBaseType_t uxPriority;

	if( DEBUG )
		ESP_LOGI( TAG, "\n\n task_2 run..." );

	uxPriority = uxTaskPriorityGet( NULL );

    for(;;)
	{
		
		if( DEBUG )
			ESP_LOGI( TAG, "\n\n task_2 count = %d.", count_2 );
		
		count_2++; 

		vTaskPrioritySet( NULL, ( uxPriority - 2 ) );
    }

	/**
	 * Este comando não deve ser executado...
	 */
    vTaskDelete(NULL);
}


/**
 * Inicio da Aplicação;
 */
void app_main( void )
{

	/**
	 * Task responsável pelo incremento de count_2;
	 */
    if( ( xTaskCreate( task_1, "task_1", 4048, NULL, 5, NULL ) ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task_1.\n" );	
		return;		
	}
	/**
	 * Task responsável pelo incremento de count_2;
	 */
	if( ( xTaskCreate( task_2, "task_2", 4048, NULL, 5, &xTask2Handle ) ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task_2.\n" );	
		return;		
	}

	
	
}





