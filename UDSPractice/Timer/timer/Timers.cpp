#include "Timers.h"
#include <stdio.h>
#include <thread>
#include "List.h"
#include "config.h"
#include "projdefs.h"
#include "support_functions.h"

static void EnterLoop(void* args);

Timers::Timers() {
	_px_list_interface = new ListInterface();
	_timer_queue = NULL;
	_px_queue_interface = new QueueInterface();
	_start_tick = GetTickCount();
	//InitializeCriticalSection(&_cs);
}

TimerHandle_t Timers::Create(string timer_name, const uint32_t period_in_ticks, const uint32_t auto_reload, 
	void* const px_timer_id, TimerCallbackFunction callback_function) {
	Timer_t* px_new_timer;
	px_new_timer = new Timer_t;

	if (px_new_timer != nullptr) {
		InitialiseNewTimer(timer_name, period_in_ticks, auto_reload, px_timer_id, callback_function, px_new_timer);

	}

	return (TimerHandle_t*) px_new_timer;
}

uint32_t Timers::StartTimersProcessTask(void) {
	CheckForValidListAndQueue();
	if (_timer_queue != NULL) {
		std::thread *t  = new std::thread(EnterLoop, this);
		t->join();
		if (t == NULL) {
			return pdFAIL;
		}
	}
	
	return pdPASS;
}

void Timers::StartTimer(TimerHandle_t timer, const TickType_t xTicksToWait, const TickType_t xNextExpireTime) {
	Timer_t* px_tmr = (Timer_t*)timer;
	TickType_t xTimeNow, xOptionalValue;
	uint32_t xTimerListsWereSwitched, xResult;
	
	xOptionalValue = prvSampleTimeNowSinceConstruct(&xTimerListsWereSwitched);

	//List_t * const px_list = (List_t *) 
	if (listIS_CONTAINED_WITHIN(NULL, &(px_tmr->list_item)) == pdFALSE) {
		_px_list_interface->RemoveItem(&(px_tmr->list_item));

	}
	xTimeNow = prvSampleTimeNowSinceConstruct(&xTimerListsWereSwitched);

	if (prvInsertTimerInActiveList(px_tmr, xOptionalValue + px_tmr->period_in_ticks, xTimeNow, xOptionalValue) != pdFALSE) {
		px_tmr->callback_function(timer);

		if (px_tmr->auto_reload == (uint32_t)pdTRUE) {
			//xResult = xTimerGenericCommand(pxTimer, tmrCOMMAND_START_DONT_TRACE, xMessage.u.xTimerParameters.xMessageValue + pxTimer->xTimerPeriodInTicks, NULL, tmrNO_DELAY);
			
		}

	}
}

uint32_t Timers::prvInsertTimerInActiveList(Timer_t* const pxTimer, const TickType_t xNextExpiryTime, const TickType_t xTimeNow, const TickType_t xCommandTime) {
	uint32_t xProcessTimerNow = pdFALSE;

	listSET_LIST_ITEM_VALUE(&(pxTimer->list_item), xNextExpiryTime);
	listSET_LIST_ITEM_OWNER(&(pxTimer->list_item), pxTimer);

	if (xNextExpiryTime <= xTimeNow) {
		/* Has the expiry time elapsed between the command to start/reset a
		timer was issued, and the time the command was processed? */
		if (((TickType_t)(xTimeNow - xCommandTime)) >= pxTimer->period_in_ticks) /*lint !e961 MISRA exception as the casts are only redundant for some ports. */
		{
			/* The time between a command being issued and the command being
			processed actually exceeds the timers period.  */
			xProcessTimerNow = pdTRUE;
		} else {
			_px_list_interface->InsertItem(_px_overflow_timer_list, &(pxTimer->list_item));
		} 
	} else {
		if ((xTimeNow < xCommandTime) && (xNextExpiryTime >= xCommandTime)) {
			/* If, since the command was issued, the tick count has overflowed
			but the expiry time has not, then the timer must have already passed
			its expiry time and should be processed immediately. */
			xProcessTimerNow = pdTRUE;
		} else {
			_px_list_interface->InsertItem(_px_current_timer_list, &(pxTimer->list_item));
		}
	}
	
	return xProcessTimerNow;
}

void Timers::prvSwitchTimerLists(void) {
	TickType_t xNextExpireTime, xReloadTime;
	List_t* pxTemp;
	Timer_t *px_timer;

	/* The tick count has overflowed.  The timer lists must be switched.
	If there are any timers still referenced from the current timer list
	then they must have expired and should be processed before the lists
	are switched. */
	while ((listLIST_IS_EMPTY(_px_current_timer_list) == pdFALSE)) {
		xNextExpireTime = listGET_ITEM_VALUE_OF_HEAD_ENTRY(_px_current_timer_list);

		px_timer = (Timer_t *)listGET_OWNER_OF_HEAD_ENTRY(_px_current_timer_list);
		_px_list_interface->RemoveItem(&(px_timer->list_item));
		px_timer->callback_function((TimerHandle_t)px_timer);

		if (px_timer->auto_reload == (uint32_t)pdTRUE) {
			xReloadTime = xNextExpireTime + px_timer->period_in_ticks;
			if (xReloadTime > xNextExpireTime) {
				listSET_LIST_ITEM_VALUE(&(px_timer->list_item), xReloadTime);
				listSET_LIST_ITEM_OWNER(&(px_timer->list_item), px_timer);
				_px_list_interface->InsertItem(_px_current_timer_list, &(px_timer->list_item));
			} else {
				StartTimer((TimerHandle_t)px_timer, 0, xNextExpireTime);
			}
		}
	}

	pxTemp = _px_current_timer_list;
	_px_current_timer_list = _px_overflow_timer_list;
	_px_overflow_timer_list = pxTemp;
}

static void EnterLoop(void * args) {
	Timers *ts = (Timers*)args;
	ts->TimerProcessLoop();
}

void Timers::TimerProcessLoop() {
	TickType_t xNextExpireTime;
	uint32_t xListWasEmpty;

	for (;;) {
		xNextExpireTime = prvGetNextExpireTime(&xListWasEmpty);
		prvProcessTimerOrBlockTask(xNextExpireTime, xListWasEmpty);
		
	}
}

TickType_t Timers::prvGetNextExpireTime(uint32_t* const pxListWasEmpty) {
	TickType_t xNextExpireTime;

	/* Timers are listed in expiry time order, with the head of the list
	referencing the task that will expire first.  Obtain the time at which
	the timer with the nearest expiry time will expire.  If there are no
	active timers then just set the next expire time to 0.  That will cause
	this task to unblock when the tick count overflows, at which point the
	timer lists will be switched and the next expiry time can be
	re-assessed.  */
	*pxListWasEmpty = listLIST_IS_EMPTY(_px_current_timer_list);
	if (*pxListWasEmpty == pdFALSE)
	{
		xNextExpireTime = listGET_ITEM_VALUE_OF_HEAD_ENTRY(_px_current_timer_list);
	} else {
		/* Ensure the task unblocks when the tick count rolls over. */
		xNextExpireTime = (TickType_t)0U;
	}

	return xNextExpireTime;
}

void Timers::prvProcessTimerOrBlockTask(const TickType_t xNextExpireTime, uint32_t xListWasEmpty) {
	TickType_t xTimeNow;
	uint32_t xTimerListsWereSwitched;

	//vTaskSuspendAll();//++uxSchedulerSuspended;
	portENTER_CRITICAL();
	{
		xTimeNow = prvSampleTimeNowSinceConstruct(&xTimerListsWereSwitched);
		
		if (xTimerListsWereSwitched == pdFALSE) {
			if ((xListWasEmpty == pdFALSE) && (xNextExpireTime <= xTimeNow)) {
				//(void)xTaskResumeAll(); need to be implemented...
				//temporary use next line: portEXIT_CRITICAL instead
				portEXIT_CRITICAL();
				prvProcessExpiredTimer(xNextExpireTime, xTimeNow);
			} else {
				portEXIT_CRITICAL();
			}
		} else {
			//(void)xTaskResumeAll();
			portEXIT_CRITICAL();
		}

	}
}

TickType_t Timers::prvSampleTimeNowSinceConstruct(uint32_t* const pxTimerListsWereSwitched) {
	TickType_t time_now = GetTickCount();
	static TickType_t xLastTime = (TickType_t)0U;
	time_now -= _start_tick;

	if (time_now < xLastTime) {
		/*
		 * The tick count has overflowed.  Switch the timer lists after ensuring the
		 * current timer list does not still reference some timers.
		 */
		 prvSwitchTimerLists();//current timer_list and overflow timer_list
		*pxTimerListsWereSwitched = pdTRUE;
	} else {
		*pxTimerListsWereSwitched = pdFALSE;
	}

	xLastTime = time_now;
#if PRINTF_ON 
	vPrintStringAndNumber("timer now:", time_now);
#endif // PRINTF

	return time_now;
}

void Timers::prvProcessExpiredTimer(const TickType_t xNextExpireTime, const TickType_t xTimeNow) {
	Timer_t* const pxTimer = (Timer_t*)listGET_OWNER_OF_HEAD_ENTRY(_px_current_timer_list);
	_px_list_interface->RemoveItem(&(pxTimer->list_item));
	if (pxTimer->auto_reload == (uint32_t)pdTRUE) {
		//to be implemented...
		if (prvInsertTimerInActiveList(pxTimer, xNextExpireTime + pxTimer->period_in_ticks, xTimeNow, xNextExpireTime) != pdFALSE) {
			StartTimer((TimerHandle_t*)pxTimer, 0, xNextExpireTime);
		}
	}
	pxTimer->callback_function((TimerHandle_t)pxTimer);
}

void Timers::InitialiseNewTimer(string timer_name, const uint32_t period_in_ticks, const uint32_t auto_reload,
	void* const px_timer_id, TimerCallbackFunction callback_function, Timer_t* px_new_timer) {
	_ASSERT((period_in_ticks > 0));
	if (px_new_timer != nullptr) {
		CheckForValidListAndQueue();
		px_new_timer->name = timer_name;
		px_new_timer->period_in_ticks = period_in_ticks;
		px_new_timer->auto_reload = auto_reload;
		px_new_timer->timer_id = px_timer_id;
		px_new_timer->callback_function = callback_function;
		_px_list_interface->InitialiseListItem(&px_new_timer->list_item);
			
	}
}

void Timers::CheckForValidListAndQueue(void) {
	portENTER_CRITICAL();
	if (_timer_queue == NULL) {
		_px_list_interface->InitialiseList(&_active_timer_list1);
		_px_list_interface->InitialiseList(&_active_timer_list2);
		_px_current_timer_list = &_active_timer_list1;
		_px_overflow_timer_list = &_active_timer_list2;

		#if( configSUPPORT_STATIC_ALLOCATION == 1 ) 
		{
		} 
		#else
		{
			_timer_queue = _px_queue_interface->Create((uint32_t)configTIMER_QUEUE_LENGTH, sizeof(DaemonTaskMessage_t), queueQUEUE_TYPE_BASE);
		}
		#endif

		//#if ( configQUEUE_REGISTRY_SIZE > 0 ) {
		//}
		//#endif

	} else {
	
	}

	portEXIT_CRITICAL();
}
