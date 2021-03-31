#pragma once

#define queueQUEUE_TYPE_BASE				( ( uint8_t ) 0U )

typedef void* QueueHandle_t;

struct QUEUE {
	int8_t* pcHead;

	uint32_t uxLength;
	uint32_t uxItemSize;
};
typedef QUEUE Queue_t;

class QueueInterface
{
public:
	QueueInterface();
	~QueueInterface();
	QueueHandle_t Create(const uint32_t uxQueueLength, const uint32_t uxItemSize, const uint8_t ucQueueType);
	uint32_t xQueueGenericReset(QueueHandle_t xQueue, uint32_t xNewQueue);

private:
	void InitialiseNewQueue(const uint32_t uxQueueLength, const uint32_t uxItemSize, uint8_t* pucQueueStorage, const uint8_t ucQueueType, Queue_t* pxNewQueue);

};
