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
#define BLINK_GPIO 		2 //Led building azul do kit ESP32
#define BUTTON_GPIO  GPIO_NUM_17
static const char * TAG = "MAIN: ";

/*
 * Variável Global. 
 */
QueueHandle_t xQueue_button; 

struct AMessage
{
	char ucMessageID;
	char ucData[ 20 ];
};


/*
 * Task Responsável pelo incremento e envio da variável count
 * na fila.
 */
void task_button( void * pvParameter )
{
	/* uint32_t é um typedef padrão definido em stdint.h */
	uint32_t status = 0;

	/**
	 * Cria e inicializa a variável xMessage;
	 */
	struct AMessage xMessage = 
	{ 
		.ucMessageID = 1 , 
		.ucData = "SENAI-SP" 
	};
	
	/**
	 * Passa o endereço de xMessage para um Ponteiro;
	 */
	struct AMessage * pxMessage = &xMessage; 

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

				ESP_LOGI( TAG, "\n\n Button foi pressionado." );
				/**
				 * O endereço do ponteiro é enviado na fila;
				 */
				if( xQueueSend( xQueue_button, ( void * ) &pxMessage, (10/portTICK_PERIOD_MS) ) == pdPASS )
				{
					ESP_LOGI( TAG, "\n\n Mensagem enviada com sucesso!" );
					ESP_LOGI( TAG, "\n\n task_print alterou o ID de xMessage para %d.", xMessage.ucMessageID );
					
				}
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

/*
 * Esta task é responsável em receber o valor de count
 * por meio da leitura da fila e imprimir na saída do console.
*/
void task_print( void * pvParameter )
{
	/**
	 * A variável msg será responsável em armazenar o ponteiro da struct;
	 */
	struct AMessage * msg; 

	ESP_LOGI( TAG, "\n\n task_print run..." );
	
	for(;;)
	{
		/**
		 * Aguarda a chegada de algum valor na fila;
		 * Caso tenha algum valor na fila, este valor é o ponteiro da struct;
		 */
		xQueueReceive( xQueue_button, &msg, portMAX_DELAY ); 	
		ESP_LOGI( TAG, "\n\n Mensagem Recebida. ID = %d. Msg = %s", msg->ucMessageID, msg->ucData );
		
		msg->ucMessageID = 2;

		/* Bloqueia intencionalmente esta task por 10ms */
		vTaskDelay( 10/portTICK_PERIOD_MS );
	}
	/**
	 * Este comando não deve ser executado...
	 */
    vTaskDelete(NULL);
}

/*
 * Task responsável em inverter o estado lógico do led building
 * do kit;
*/
void task_led(void *pvParameter)
{

	ESP_LOGI( TAG, "\n\n task_led run..." );
	/*
	 * Configura a GPIO2 do ESP32 como GPIO-OUTPUT;
	 * Sobre o Led Building do ESP32; 
	 * Ligado 		-> GPIO2 - Nível 0
	 * Desligado 	-> GPIO2 - Nível 1
	*/
    gpio_pad_select_gpio( BLINK_GPIO );
    gpio_set_direction( BLINK_GPIO, GPIO_MODE_OUTPUT );

    for(;;)
	{
        /* Liga Led building */
        gpio_set_level( BLINK_GPIO, 0 );
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
        /* Desliga Led building */
        gpio_set_level( BLINK_GPIO, 1 );
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}

void app_main()
{
	/*
	 * Realiza a alocação dinâmica da fila na memória RAM do ESP32;
	 * Neste caso foi criado um fila com 5 elementos do tipo ponteiro de struct.
	 */
	if( (xQueue_button = xQueueCreate( 5,  sizeof( struct AMessage * )) ) == NULL )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar xQueue_button.\n" );
		/* app_main será finalizada e o programa finalizado */
		return;
	} 
	
	/* configMINIMAL_STACK_SIZE é um #define que informa para o FreeRTOS qual o tamanho mínimo
	necessário para rodar uma task. Por meio do menuconfig é possível saber o tamanho de configMINIMAL_STACK_SIZE; 
	*/
    if( ( xTaskCreate( task_led, "task_led", 4048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar task_led.\n" );	
		return;		
	}
	
	/*
	 * Task_print e task_adc contém a função printf (ESP_LOGI). Portanto, foi aumentado o stack
	 * para 2k;
	 */
	if( (xTaskCreate( task_print, "task_print", 4048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar task_print.\n" );	
		return;		
	}
	if( (xTaskCreate( task_button, "task_button", 4048, NULL, 5, NULL )) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar task_button.\n" );	
		return;		
	}
	
}





