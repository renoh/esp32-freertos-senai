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
#define LED_BUILDING	2    
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<LED_BUILDING)

#define BUTTON_1	17 
#define GPIO_INPUT_PIN_SEL  (1ULL<<BUTTON_1)

/*
 * Variável Global. 
 */
static const char * TAG = "MAIN: ";
QueueHandle_t xQueue;

/**
 * Protótipos
 */
void task_led( void *pvParameter ); 


/**
 * Função de callback chamada por uma ISR.
 */
static void IRAM_ATTR gpio_isr_handler( void * pvParameter)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	static int cnt_1 = 0;

    if( BUTTON_1 == (uint32_t) pvParameter  )
	{

		if( gpio_get_level( BUTTON_1 ) == 0 ) 
		{
			cnt_1++;	

			xQueueSendFromISR( xQueue, &cnt_1, &xHigherPriorityTaskWoken );

		    if( xHigherPriorityTaskWoken == pdTRUE )
		    {
		        portYIELD_FROM_ISR();
		    }

		} 	
	
	}   
		
}


 
void task_led( void *pvParameter )
{
	uint32_t cnt_1 = 0;

	if( DEBUG )
		ESP_LOGI( TAG, "task_led run...\n" );
	/**
	 * Led Building
	 */
	gpio_config_t io_conf = {
		.intr_type = GPIO_PIN_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = GPIO_OUTPUT_PIN_SEL
	};	
	gpio_config(&io_conf);  
	
	/**
	 * Button (por interrupção externa)
	 */
	gpio_config_t io_conf2 = {
		.intr_type = GPIO_INTR_NEGEDGE,
		.mode = GPIO_MODE_INPUT,
		.pin_bit_mask = GPIO_INPUT_PIN_SEL,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE
	};
    gpio_config(&io_conf2); 

    /**
     * Habilita a interrupção externa das GPIOs;
     */
    gpio_install_isr_service(0);
    /**
     * Registra o pino que irá gerar acionar a interrupção por borda de descida 
     * e informa qual a função de callback que será chamada. 
     * O número da GPIOs será passado no parametro da função de callback.
     */
    gpio_isr_handler_add( BUTTON_1, gpio_isr_handler, (void*) BUTTON_1 ); 

	
    
    for(;;) 
    {		  

		xQueueReceive( xQueue, &cnt_1, portMAX_DELAY ); 	

	    gpio_set_level( LED_BUILDING, cnt_1%2 );	

		if( DEBUG ) 
			ESP_LOGI( TAG, "Recebe de ISR cnt_1 = %d.\r\n", cnt_1 );


		vTaskDelay( 10/portTICK_PERIOD_MS );
    }
}
 
void app_main( void )
{	

	/**
	 * Fila responsável em Armazenar o valor de count_1 e count_2;
	 */
	if( ( xQueue = xQueueCreate( 5,  sizeof( uint32_t ) ) ) == NULL )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar xQueue.\n" );
		return;
	} 

    if( xTaskCreate( task_led, "task_led", 2048, NULL, 2, NULL )!= pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar task_led.\n" );	
		return;		
	}

}





