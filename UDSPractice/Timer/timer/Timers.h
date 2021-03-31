#pragma once
#include <cstdint>
#include <string>
#include <windows.h>
#include "queue.h"
#include "List.h"
#include "portmacro.h"

typedef void * TimerHandle_t;
typedef void (*TimerCallbackFunction)(TimerHandle_t timer);

using namespace std;

typedef struct TimerControl {
	string name;
	ListItem_t list_item;			/*<< Standard linked list item as used by all kernel features for event management. */
	uint32_t period_in_ticks;
	uint32_t auto_reload;
	void* timer_id;
	TimerCallbackFunction callback_function;
	uint32_t timer_number;			/*<< An ID assigned by trace tools such as FreeRTOS+Trace */

} Timer;
typedef Timer Timer_t;

typedef struct TimerParameters {
	uint32_t message_value;
	Timer_t timer;

} TimerParameter_t;

typedef struct TimerQueueMessage
{
	uint32_t			xMessageID;			/*<< The command being sent to the timer service task. */
	union
	{
		TimerParameter_t xTimerParameters;

		/* Don't include xCallbackParameters if it is not going to be used as
		it makes the structure (and therefore the timer queue) larger. */
#if ( INCLUDE_xTimerPendFunctionCall == 1 )
		CallbackParameters_t xCallbackParameters;
#endif /* INCLUDE_xTimerPendFunctionCall */
	} u;
} DaemonTaskMessage_t;

using namespace std;

class ListInterface;
class QueueInterface;
class Timers {
private:
	uint32_t _start_tick;
	QueueHandle_t _timer_queue;
	QueueInterface* _px_queue_interface;
	ListInterface* _px_list_interface;
	List_t _active_timer_list1, _active_timer_list2, *_px_current_timer_list, *_px_overflow_timer_list;
	
	void InitialiseNewTimer(string timer_name, const uint32_t period_in_ticks, const uint32_t auto_reload, void* const px_timer_id, TimerCallbackFunction callback_function, Timer_t* px_new_timer);
	void CheckForValidListAndQueue(void);
	TickType_t prvGetNextExpireTime(uint32_t* const pxListWasEmpty);
	void prvProcessTimerOrBlockTask(const TickType_t xNextExpireTime, uint32_t xListWasEmpty);
	TickType_t prvSampleTimeNowSinceConstruct(uint32_t* const pxTimerListsWereSwitched);
	void prvProcessExpiredTimer(const TickType_t xNextExpireTime, const TickType_t xTimeNow);
	uint32_t prvInsertTimerInActiveList(Timer_t* const pxTimer, const TickType_t xNextExpiryTime, const TickType_t xTimeNow, const TickType_t xCommandTime);
	void prvSwitchTimerLists(void);

public:
	Timers();
	TimerHandle_t Create(string timer_name, const uint32_t period_in_ticks, const uint32_t auto_reload, void* const px_timer_id, TimerCallbackFunction callback_function);
	uint32_t StartTimersProcessTask(void);
	void StartTimer(TimerHandle_t timer, const TickType_t xTicksToWait, const TickType_t xNextExpireTime = 0);
	void TimerProcessLoop();

};
