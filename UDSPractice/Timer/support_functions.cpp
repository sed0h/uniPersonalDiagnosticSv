#include <cstdint>
#include <stdio.h>
#include "support_functions.h"
#include "portmacro.h"


void vPrintString(const char* pcString) {
}

void vPrintStringAndNumber(const char* pcString, uint32_t ulValue) {
	portENTER_CRITICAL(); {
		printf("%s %lu\r\n", pcString, ulValue);
		fflush(stdout);
	}
	portEXIT_CRITICAL();
}

void vPrintTwoStrings(const char* pcString1, const char* pcString2) {

}
