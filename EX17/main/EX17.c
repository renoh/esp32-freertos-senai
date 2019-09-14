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

/* The three sender task. */
void vSenderTask1( void *pvParameters );
void vSenderTask2( void *pvParameters );

/* The receiver task.  The receiver blocks on the queue set to received data
from both sender task. */
void vReceiverTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* Declare two variables of type QueueHandle_t.  Both queues are added to the
same queue set. */
static QueueHandle_t xQueueUint32 = NULL, xQueueCharPointer = NULL;

/* Declare a variable of type QueueSetHandle_t.  This is the queue set to which
the two queues are added. */
static QueueSetHandle_t xQueueSet = NULL;

void app_main( void ) 
{
    
	/* Create the two queues.  Each queue sends character pointers.  The
	priority of the receiving task is above the priority of the sending tasks so
	the queues will never have more than one item in them at any one time. */
    xQueueUint32 = xQueueCreate( 1, sizeof( uint32_t) );
	xQueueCharPointer = xQueueCreate( 1, sizeof( char * ) );

	/* Create the queue set.  There are two queues both of which can contain
	1 item, so the maximum number of queue handle the queue set will ever have
	to hold is 2 (1 item multiplied by 2 sets). */
	xQueueSet = xQueueCreateSet( 1 * 2 );

	/* Add the two queues to the set. */
	xQueueAddToSet( xQueueUint32, xQueueSet );
	xQueueAddToSet( xQueueCharPointer, xQueueSet );

	/* Create the tasks that send to the queues. */
	xTaskCreate( vSenderTask1, "Sender1", 2048, NULL, 1, NULL );
	xTaskCreate( vSenderTask2, "Sender2", 2048, NULL, 1, NULL );

	/* Create the receiver task. */
	xTaskCreate( vReceiverTask, "Receiver", 2048, NULL, 2, NULL );

}
/*-----------------------------------------------------------*/

void vSenderTask1( void *pvParameters )
{
    BaseType_t xStringNumber = 0;
    const TickType_t xBlockTime = pdMS_TO_TICKS( 100 );

	if( DEBUG )
		ESP_LOGI( TAG, "vSenderTask1 run.\r\n" );	
	
	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{
		/* Block for 100ms. */
		vTaskDelay( xBlockTime );

        xStringNumber++;

		xQueueSend( xQueueUint32, &xStringNumber, 0 );
	}
}
/*-----------------------------------------------------------*/

void vSenderTask2( void *pvParameters )
{
    const TickType_t xBlockTime = pdMS_TO_TICKS( 200 );
    const char * const pcMessage = "Message from vSenderTask2\r\n";

	if( DEBUG )
		ESP_LOGI( TAG, "vSenderTask2 run.\r\n" );	
	
	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{
		/* Block for 200ms. */
		vTaskDelay( xBlockTime );

		/* Send this task's string to xQueueCharPointer. It is not necessary to use a
		block time, even though the queue can only hold one item.  This is
		because the priority of the task that reads from the queue is higher
		than the priority of this task; as soon as this task writes to the queue
		it will be pre-empted by the task that reads from the queue, so the
		queue will already be empty again by the time the call to xQueueSend()
		returns.  The block time is set to 0. */
		xQueueSend( xQueueCharPointer, &pcMessage, 0 );
	}
}
/*-----------------------------------------------------------*/

void vReceiverTask( void *pvParameters )
{
    char *pcReceivedString;
    uint32_t num;
    QueueSetMemberHandle_t xHandle;
    const TickType_t xDelay100ms = pdMS_TO_TICKS(100);

	if( DEBUG )
		ESP_LOGI( TAG, "vReceiverTask run.\r\n" );	

	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{

        xHandle =  xQueueSelectFromSet( xQueueSet, xDelay100ms );
        
        if(xHandle != NULL)
        {
            
            if(xHandle == (QueueSetMemberHandle_t) xQueueCharPointer)
            {
                xQueueReceive( xQueueCharPointer, &pcReceivedString, 0 );
                if( DEBUG )
					ESP_LOGI( TAG, "%s", pcReceivedString );
            }
            
            if(xHandle == (QueueSetMemberHandle_t) xQueueUint32)
            {
                xQueueReceive( xQueueUint32, &num, 0 );
                
				if( DEBUG )
					ESP_LOGI( TAG, "Num = %d", num );
            }
                           
            
        }
	}
}