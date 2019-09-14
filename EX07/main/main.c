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
QueueHandle_t xQueue; 

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
void task_1( void * pvParameter )
{
	xData_t count_1 = {
		.value = 0,
		.task_id = 1, 
		.task_name = "task_1"
	};

	if( DEBUG )
		ESP_LOGI( TAG, "\n\n task_1 run..." );

    for(;;)
	{
		/**
		 * Envia uma cópia do valor de count_1 na fila. 
		 * Caso a fila esteja cheia, esta task será bloqueada.
		 */
		xQueueSend( xQueue, &count_1, portMAX_DELAY ); 
		
		if( DEBUG )
			ESP_LOGI( TAG, "\n\n count_1 (%d) enviado na Queue.", count_1.value );
		
		count_1.value++; 
		vTaskDelay( 1000/portTICK_PERIOD_MS );

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
	xData_t count_2 = {
		.value = 0,
		.task_id = 2, 
		.task_name = "task_2"
	};

	if( DEBUG )
		ESP_LOGI( TAG, "\n\n task_2 run..." );

    for(;;)
	{
		xQueueSend( xQueue, &count_2, portMAX_DELAY ); 
		
		if( DEBUG )
			ESP_LOGI( TAG, "\n\n count_2 (%d) enviado na Queue.", count_2.value );
		
		count_2.value++; 
		vTaskDelay( 1000/portTICK_PERIOD_MS );

    }

	/**
	 * Este comando não deve ser executado...
	 */
    vTaskDelete(NULL);
}

void task_print( void * pvParameter )
{
	xData_t count; 

	if( DEBUG )
		ESP_LOGI( TAG, "\n\n task_print run..." );
	
	for(;;)
	{
		/**
		 * Realiza a leitura da fila. Caso tenha algum valor a ser lido
		 * este será armazenado em count;
		 */
		xQueueReceive( xQueue, &count, portMAX_DELAY ); 	
		
		if( DEBUG ) {
			ESP_LOGI( TAG, "\n\nMsg Recebida. 	\
								task_id = %d. 	\
								task_name = %s. \
								value = %d.", count.task_id, count.task_name, count.value );
		}

		vTaskDelay( 10/portTICK_PERIOD_MS );
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
	if( ( xQueue = xQueueCreate( 5,  sizeof( xData_t ) ) ) == NULL )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar xQueue_button.\n" );
		return;
	} 
	
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
	if( ( xTaskCreate( task_2, "task_2", 4048, NULL, 5, NULL ) ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task_2.\n" );	
		return;		
	}

	/**
	 * Task Responsável pela Impressão;
	 */
	if( (xTaskCreate( task_print, "task_print", 4048, NULL, 5, NULL ) ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task_print.\n" );	
		return;		
	}

	
}





