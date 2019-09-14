/*
 * Lib C
 */
#include <stdio.h>
#include <stdint.h>  

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
#define BLINK_GPIO 2 //led building
static const char * TAG = "MAIN: ";

/*
 * Variável Global. 
 */
QueueHandle_t xQueue; 


/*
 * Task Responsável pelo incremento e envio da variável count
 * na fila.
 */
void task_adc( void * pvParameter )
{
	/* uint32_t é um typedef padrão definido em stdint.h */
	uint32_t count = 0; 
	uint32_t status;
	
	/* As Tasks normalmente possuem um loop sem saída */
	for(;;)
	{
		/*
		 * Envia na fila o valor da variável count;
		 * Caso a fila esteja cheia será retornado imediatamente;
		 * Sendo assim, 
		 */
		
		/* status_return = xQueueSend( fila_name, variável_address, timeout) */
		status = xQueueSend(xQueue, &count, 0);
		if(status == pdPASS)
		{
			ESP_LOGI( TAG, "O VALOR %d DE COUNT FOI ENVIADO NA QUEUE.\n", count );
			count++;
		}
		/* Bloqueia esta task por 1s */
		vTaskDelay(1000 / portTICK_PERIOD_MS);
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
	uint32_t count; 
	uint32_t status;
	
	for(;;)
	{
		/* status_return = xQueueReceive( fila_name, variável_address, timeout) */
		status = xQueueReceive(xQueue, &count, 0); 
		if(status == pdPASS)
		{		
			ESP_LOGI( TAG, "COUNT RECEBIDO. VALOR = %d\n", count );	
		}
		
		vTaskDelay(1000 / portTICK_PERIOD_MS);	
	}
    vTaskDelete(NULL);
}

/*
 * Task responsável em inverter o estado lógico do led building
 * do kit;
*/
void blink_task(void *pvParameter)
{
	/*
	 * Configura a GPIO2 do ESP32 como GPIO-OUTPUT;
	 * Sobre o Led Building do ESP32; 
	 * Ligado 		-> GPIO2 - Nível 0
	 * Desligado 	-> GPIO2 - Nível 1
	*/
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
	
    for(;;)
	{
        /* Liga Led */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Delisga Led */
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
	/*
	 * Realiza a alocação dinâmica da fila na memória RAM do ESP32;
	 * Neste caso foi criado um fila com 5 elementos do tipo unsigned int 32 bits.
	 */
	if( (xQueue = xQueueCreate( 5, sizeof(uint32_t)) ) == NULL )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar xQueue.\n" );
		/* app_main será finalizada e o programa finalizado */
		return;
	} 
	
	/* configMINIMAL_STACK_SIZE é um #define que informa para o FreeRTOS qual o tamanho mínimo
	necessário para rodar uma task. Por meio do menuconfig é possível saber o tamanho de configMINIMAL_STACK_SIZE; 
	*/
    if( (xTaskCreate( blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL) ) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar blink_task.\n" );	
		return;		
	}
	
	/*
	 * Task_print e task_adc contém a função printf (ESP_LOGI). Portanto, foi aumentado o stack
	 * para 2k;
	 */
	if( (xTaskCreate( task_print, "task_print", 4048, NULL, 5, NULL)) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar task_print.\n" );	
		return;		
	}
	if( (xTaskCreate( task_adc, "task_adc", 4048, NULL, 5, NULL)) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar task_adc.\n" );	
		return;		
	}
	
}





