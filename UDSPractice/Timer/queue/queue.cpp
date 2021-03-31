#include <cstdint>
#include "queue.h"
#include "config.h"
#include "portmacro.h"
#include "projdefs.h"

QueueInterface::QueueInterface() {
}

QueueHandle_t QueueInterface::Create(const uint32_t uxQueueLength, const uint32_t uxItemSize, const uint8_t ucQueueType) {
	Queue_t* px_new_queue;
	size_t queue_size_in_bytes;
	uint8_t* px_queue_storage;
	
	configASSERT(uxQueueLength > 0);
	if (uxItemSize == 0) {
		queue_size_in_bytes = (size_t)0;
	}else {
		queue_size_in_bytes = (size_t)(uxQueueLength + uxItemSize);
	}
	px_new_queue = new Queue_t[sizeof(Queue_t) + queue_size_in_bytes];
	if (px_new_queue != NULL) {
		px_queue_storage = ((uint8_t*)px_new_queue) + sizeof(Queue_t);

		InitialiseNewQueue(uxQueueLength, uxItemSize, px_queue_storage, ucQueueType, px_new_queue);
	}
	return px_new_queue;
}

void QueueInterface::InitialiseNewQueue(const uint32_t uxQueueLength, const uint32_t uxItemSize, uint8_t* pucQueueStorage, const uint8_t ucQueueType, Queue_t* pxNewQueue) {
	(void)ucQueueType;

	if (uxItemSize == (uint32_t)0)
	{
		/* No RAM was allocated for the queue storage area, but PC head cannot
		be set to NULL because NULL is used as a key to say the queue is used as
		a mutex.  Therefore just set pcHead to point to the queue as a benign
		value that is known to be within the memory map. */
		pxNewQueue->pcHead = (int8_t*)pxNewQueue;
	}
	else
	{
		/* Set the head to the start of the queue storage area. */
		pxNewQueue->pcHead = (int8_t*)pucQueueStorage;
	}

	/* Initialise the queue members as described where the queue type is
	defined. */
	pxNewQueue->uxLength = uxQueueLength;
	pxNewQueue->uxItemSize = uxItemSize;
	(void)xQueueGenericReset(pxNewQueue, pdTRUE);

#if ( configUSE_TRACE_FACILITY == 1 )
	{
		pxNewQueue->ucQueueType = ucQueueType;
	}
#endif /* configUSE_TRACE_FACILITY */

#if( configUSE_QUEUE_SETS == 1 )
	{
		pxNewQueue->pxQueueSetContainer = NULL;
	}
#endif /* configUSE_QUEUE_SETS */
}

uint32_t QueueInterface::xQueueGenericReset(QueueHandle_t xQueue, uint32_t xNewQueue)
{
	Queue_t* const pxQueue = (Queue_t*)xQueue;

	configASSERT(pxQueue);

	portENTER_CRITICAL();
	{
		//pxQueue->pcTail = pxQueue->pcHead + (pxQueue->uxLength * pxQueue->uxItemSize);
		//pxQueue->uxMessagesWaiting = (UBaseType_t)0U;
		//pxQueue->pcWriteTo = pxQueue->pcHead;
		//pxQueue->u.pcReadFrom = pxQueue->pcHead + ((pxQueue->uxLength - (UBaseType_t)1U) * pxQueue->uxItemSize);
		//pxQueue->cRxLock = queueUNLOCKED;
		//pxQueue->cTxLock = queueUNLOCKED;

		//if (xNewQueue == pdFALSE)
		//{
		//	/* If there are tasks blocked waiting to read from the queue, then
		//	the tasks will remain blocked as after this function exits the queue
		//	will still be empty.  If there are tasks blocked waiting to write to
		//	the queue, then one should be unblocked as after this function exits
		//	it will be possible to write to it. */
		//	if (listLIST_IS_EMPTY(&(pxQueue->xTasksWaitingToSend)) == pdFALSE)
		//	{
		//		if (xTaskRemoveFromEventList(&(pxQueue->xTasksWaitingToSend)) != pdFALSE)
		//		{
		//			queueYIELD_IF_USING_PREEMPTION();
		//		}
		//		else
		//		{
		//			mtCOVERAGE_TEST_MARKER();
		//		}
		//	}
		//	else
		//	{
		//		mtCOVERAGE_TEST_MARKER();
		//	}
		//}
		//else
		//{
		//	/* Ensure the event queues start in the correct state. */
		//	vListInitialise(&(pxQueue->xTasksWaitingToSend));
		//	vListInitialise(&(pxQueue->xTasksWaitingToReceive));
		//}
	}
	portEXIT_CRITICAL();

	/* A value is returned for calling semantic consistency with previous
	versions. */
	return pdPASS;
}