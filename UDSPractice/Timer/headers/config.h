#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#define configTIMER_QUEUE_LENGTH				2
#define PRINTF_ON								0

extern void vAssertCalled(uint32_t ulLine, const char* const pcFileName);
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __LINE__, __FILE__ )

#ifdef __cplusplus
}
#endif