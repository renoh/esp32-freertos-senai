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
#define BUTTON_GPIO  GPIO_NUM_17
/*
 * Variável Global. 
 */
static const char * TAG = "MAIN: ";
QueueHandle_t xQueue; 
TaskHandle_t xTask1_Handle;

/**
 * struct
 */
typedef struct t {
	uint32_t value; 
	char task_id; 
	char * task_name; 
} xData_t; 

static xData_t count;
const char * task_state[] = { "Ready", "Running", "Blocked", "Suspended", "Deleted" };

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
			ESP_LOGI( TAG, "task_1 envia %d.\r\n", count_1.value );
		
		count_1.value++; 
		
		vTaskDelay( 1000/portTICK_PERIOD_MS );

    }
	/**
	 * Este comando não deve ser executado...
	 */
    vTaskDelete(NULL);
}



void task_button( void * pvParameter )
{
	/* uint32_t é um typedef padrão definido em stdint.h */
	uint32_t status = 0;
	char msg[300];

	if( DEBUG )
		ESP_LOGI( TAG, "\n\n task_button run..." );

    /**
     * Configuração da GPIO 17 como saída;
     */
    gpio_pad_select_gpio( BUTTON_GPIO );
    gpio_set_direction( BUTTON_GPIO, GPIO_MODE_INPUT );
    gpio_set_pull_mode( BUTTON_GPIO, GPIO_PULLUP_ONLY );	

	/* As Tasks normalmente possuem um loop sem saída */
	for(;;)
	{

		/**
		 * Botão pressionado?
		 */
		if( gpio_get_level(BUTTON_GPIO) == 0 && status == 0)
		{
			/**
			 * Delay necessário para não processar o bounce causado
			 * pelo acionamento do botão;
			 */
			vTaskDelay( 100 / portTICK_PERIOD_MS ); 

			/**
			 * O botão ainda está sendo pressionado? Sim, então...
			 */
			if( gpio_get_level(BUTTON_GPIO) == 0 && status == 0)
			{
				if( DEBUG )
					ESP_LOGI( TAG, "\n\n Button foi pressionado." );
		
				/**
				 * Para utilizar vTaskList é necessário habilitar 
				 * no menuconfig
				 */
				/**
				 * xTask1_Handle = Suspended
					Task---------State---Prio------Stack---Num
					task_button     R       5       2208    13
					IDLE1           R       0       1104    7
					IDLE0           R       0       1108    6
					Tmr Svc         B       1       1616    8
					esp_timer       B       22      3672    1
					ipc1            B       24      560     3
					task_1          S       5       2436    12
					task_print      B       5       2444    14
					ipc0            B       24      596     2
				 */	
				vTaskList(msg);
				printf("xTask1_Handle = %s\r\n", task_state[eTaskGetState(xTask1_Handle)]);
				printf( "\r\nTask---------State---Prio------Stack---Num" );	
				printf( "\r\n%s", msg );		
				


				status = 1; 				
			}

		}

		/**
		 * Botão Solto?
		 */
		if( gpio_get_level(BUTTON_GPIO) == 1 && status == 1)
		{
			/**
			 * Delay necessário para não processar o bounce causado
			 * pelo acionamento do botão;
			 */
			vTaskDelay( 100 / portTICK_PERIOD_MS ); 

			/**
			 * O botão ainda está realmente solto? Sim, então...
			 */
			if( gpio_get_level(BUTTON_GPIO) == 1 && status == 1)
			{
				ESP_LOGI( TAG, "\n\n Button foi solto." );
				status = 0; 
			}
		}
		
		/* Bloqueia esta task por 10ms */
		vTaskDelay( 10/portTICK_PERIOD_MS );
	}
	
	/* este comando não deveria ser executado... Caso algo estranho aconteça, 
	 * esta task será deletada; O parametro NULL informa que é para desalocar
	 * da memória ESTA task;
	*/
    vTaskDelete(NULL);	
}



void task_print( void * pvParameter )
{
	 

	if( DEBUG )
		ESP_LOGI( TAG, "\n\n task_print run..." );
	
	for(;;)
	{
		/**
		 * Realiza a leitura da fila. Caso tenha algum valor a ser lido
		 * este será armazenado em count;
		 */
		xQueueReceive( xQueue, &count, portMAX_DELAY ); 	

		if( count.task_id == 1 )
		{
			if( count.value < 10 )
			{
				if( DEBUG ) 
					ESP_LOGI( TAG, "recebe de task_1 = %d.\r\n", count.value );	

			} else {

				if( DEBUG )
					ESP_LOGI( TAG, "\n\nSuspende Task1 ");
				
				vTaskSuspend( xTask1_Handle );
			}
			
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
    if( ( xTaskCreate( task_1, "task_1", 4048, NULL, 5, &xTask1_Handle  ) ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task_1.\n" );	
		return;		
	}
	if( (xTaskCreate( task_button, "task_button", 4048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar task_button.\n" );	
		return;		
	}
	if( (xTaskCreate( task_print, "task_print", 4048, NULL, 5, NULL ) ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task_print.\n" );	
		return;		
	}

	
}





