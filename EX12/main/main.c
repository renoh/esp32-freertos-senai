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
QueueHandle_t xMailbox; 

/**
 * struct
 */
typedef struct t {
	uint32_t value; 
	char task_id; 
	char * task_name; 
} xData_t; 

/**
 * Task Responsável pelo incremento e envio de count_1 na fila;
 */
void task_sender( void * pvParameter )
{
	xData_t count_1 = {
		.value = 0,
		.task_id = 1, 
		.task_name = "task_1"
	};

	if( DEBUG )
		ESP_LOGI( TAG, "task_sender run...\r\n" );

    for(;;)
	{
		/**
		 * Envia uma cópia do valor de count_1 na fila. 
		 * Caso a fila esteja cheia, esta task será bloqueada.
		 */
		xQueueOverwrite( xMailbox, &count_1 ) ;
		
		if( DEBUG )
			ESP_LOGI( TAG, "\n\n count_1 (%d) enviado na Queue.", count_1.value );
		
		count_1.value++; 

		vTaskDelay( 3000/portTICK_PERIOD_MS );

    }
	/**
	 * Este comando não deve ser executado...
	 */
    vTaskDelete(NULL);
}

/**
 * Task Responsável pelo incremento e envio de count_2 na fila;
 */
void task1_print( void * pvParameter )
{
	xData_t count_1;
    const TickType_t xDelay500ms = pdMS_TO_TICKS(500);

	if( DEBUG )
		ESP_LOGI( TAG, "task1_print run...\r\n" );

    for(;;)
	{
		xQueuePeek( xMailbox, &count_1, xDelay500ms);
		
		if( DEBUG )
			ESP_LOGI( TAG, "task1_print count_1 = (%d)\r\n", count_1.value );

		vTaskDelay( 1000/portTICK_PERIOD_MS );

    }

	/**
	 * Este comando não deve ser executado...
	 */
    vTaskDelete(NULL);
}

void task2_print( void * pvParameter )
{
	xData_t count_2; 
    const TickType_t xDelay100ms = pdMS_TO_TICKS(100);

	if( DEBUG )
		ESP_LOGI( TAG, "task2_print run...\r\n" );

    for(;;)
	{
		xQueuePeek( xMailbox, &count_2, xDelay100ms);
		
		if( DEBUG )
			ESP_LOGI( TAG, "task2_print count_2 = (%d)\r\n", count_2.value );

		vTaskDelay( 1000/portTICK_PERIOD_MS );

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
	 * Fila responsável em Armazenar o valor de count_1 e count_2;
	 */
	if( ( xMailbox = xQueueCreate( 1,  sizeof( xData_t ) ) ) == NULL )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar xMailbox.\n" );
		return;
	} 
	
	/**
	 * Task responsável pelo incremento de count_2;
	 */
    if( ( xTaskCreate( task_sender, "task_sender", 4048, NULL, 5, NULL ) ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task_sender.\n" );	
		return;		
	}
	/**
	 * Task responsável pelo incremento de count_2;
	 */
	if( ( xTaskCreate( task1_print, "task1_print", 4048, NULL, 5, NULL ) ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task1_print.\n" );	
		return;		
	}

	/**
	 * Task Responsável pela Impressão;
	 */
	if( (xTaskCreate( task2_print, "task2_print", 4048, NULL, 5, NULL ) ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task2_print.\n" );	
		return;		
	}

	
}





