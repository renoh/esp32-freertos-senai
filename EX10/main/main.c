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
#define BUTTON_2	18 
#define GPIO_INPUT_PIN_SEL  ((1ULL<<BUTTON_1) | (1ULL<<BUTTON_2))

/*
 * Variável Global. 
 */
static const char * TAG = "MAIN: ";
volatile int cnt_1=0;
volatile int cnt_2=0;

/**
 * Protótipos
 */
void Task_LED( void *pvParameter ); 


/**
 * Função de callback chamada por uma ISR.
 */
static void IRAM_ATTR gpio_isr_handler( void * pvParameter)
{
    //Identifica qual o botão que foi pressionado

    if( BUTTON_1 == (uint32_t) pvParameter || BUTTON_2 == (uint32_t) pvParameter )
	{
		//Caso o botão 1 estiver pressionado, faz a leitura e o acionamento do led.
		if( gpio_get_level( BUTTON_1 ) == 0) 
		{
			gpio_set_level( LED_BUILDING, cnt_1%2 );	
			cnt_1++;	
		} 	

		if( gpio_get_level( BUTTON_2 ) == 0) 
		{
			gpio_set_level( LED_BUILDING, cnt_2%2 );
			cnt_2++;	
		} 	
	
	}   
		
}
 
void Task_LED( void *pvParameter )
{
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

    gpio_install_isr_service(0);
    gpio_isr_handler_add( BUTTON_1, gpio_isr_handler, (void*) BUTTON_1 ); 
    gpio_isr_handler_add( BUTTON_2, gpio_isr_handler, (void*) BUTTON_2 ); 
	
    printf("Pisca LED_1 e LED_2\n");
    
    for(;;) 
    {		  
	    
		vTaskDelay( 300/portTICK_PERIOD_MS );
    }
}
 
void app_main( void )
{	
    if( xTaskCreate( Task_LED, "Task_LED", 2048, NULL, 2, NULL )!= pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI( TAG, "error - nao foi possivel alocar Task_LED.\n" );	
		return;		
	}
}





