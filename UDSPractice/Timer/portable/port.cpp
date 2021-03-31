
#include <cstdint>
#include <Windows.h>
#include "projdefs.h"
#include "portmacro.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif // _cplusplus

	#define portNO_CRITICAL_NESTING 		( ( uint32_t ) 0 )
	
	static volatile uint32_t ulPendingInterrupts = 0UL;
	static void* pvInterruptEvent = NULL;
	static void* pvInterruptEventMutex = NULL;
	static uint32_t ulCriticalNesting = 9999UL;

	static uint32_t xPortRunning = pdFALSE;

	void vPortEnterCritical(void)
	{
		if (xPortRunning == pdTRUE)
		{
			/* The interrupt event mutex is held for the entire critical section,
			effectively disabling (simulated) interrupts. */
			WaitForSingleObject(pvInterruptEventMutex, INFINITE);
			ulCriticalNesting++;
		}
		else
		{
			ulCriticalNesting++;
		}
	}
	/*-----------------------------------------------------------*/

	void vPortExitCritical(void)
	{
		int32_t lMutexNeedsReleasing;

		/* The interrupt event mutex should already be held by this thread as it was
		obtained on entry to the critical section. */

		lMutexNeedsReleasing = pdTRUE;

		if (ulCriticalNesting > portNO_CRITICAL_NESTING)
		{
			if (ulCriticalNesting == (portNO_CRITICAL_NESTING + 1))
			{
				ulCriticalNesting--;

				/* Were any interrupts set to pending while interrupts were
				(simulated) disabled? */
				if (ulPendingInterrupts != 0UL)
				{
					configASSERT(xPortRunning);
					SetEvent(pvInterruptEvent);

					/* Mutex will be released now, so does not require releasing
					on function exit. */
					lMutexNeedsReleasing = pdFALSE;
					ReleaseMutex(pvInterruptEventMutex);
				}
			}
			else
			{
				/* Tick interrupts will still not be processed as the critical
				nesting depth will not be zero. */
				ulCriticalNesting--;
			}
		}

		if (pvInterruptEventMutex != NULL)
		{
			if (lMutexNeedsReleasing == pdTRUE)
			{
				configASSERT(xPortRunning);
				ReleaseMutex(pvInterruptEventMutex);
			}
		}
	}



	void vAssertCalled(uint32_t ulLine, const char* const pcFile)
	{
		/* The following two variables are just to ensure the parameters are not
		optimised away and therefore unavailable when viewed in the debugger. */
		volatile uint32_t ulLineNumber = ulLine, ulSetNonZeroInDebuggerToReturn = 0;
		volatile const char* const pcFileName = pcFile;

		portENTER_CRITICAL();
		{
			while (ulSetNonZeroInDebuggerToReturn == 0)
			{
				/* If you want to set out of this function in the debugger to see
				the	assert() location then set ulSetNonZeroInDebuggerToReturn to a
				non-zero value. */
			}
		}
		portEXIT_CRITICAL();

		/* Remove the potential for compiler warnings issued because the variables
		are set but not subsequently referenced. */
		(void)pcFileName;
		(void)ulLineNumber;
	}

#ifdef __cplusplus
}
#endif // _cplusplus