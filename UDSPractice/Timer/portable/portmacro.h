#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t TickType_t;

#define portMAX_DELAY ( uint32_t ) 0xffffffffUL

/* Critical section handling. */
void vPortEnterCritical(void);
void vPortExitCritical(void);

#define portENTER_CRITICAL()		vPortEnterCritical()
#define portEXIT_CRITICAL()			vPortExitCritical()

#ifdef __cplusplus
}
#endif