#include "Timers.h"
#include <iostream>
#include "support_functions.h"

static uint32_t start_tick = 0;
void callB(TimerHandle_t t);
void callC(TimerHandle_t t);

int main() {
	Timers tms;

	TimerHandle_t timerA = tms.Create("_1timer", 5000, false, 0, callB);
	TimerHandle_t timerB = tms.Create("_1timer", 500, false, 0, callB);
	TimerHandle_t timerC = tms.Create("_1timer", 1000, true, 0, callC);
	start_tick = GetTickCount();
	vPrintStringAndNumber("start tick", start_tick);
	tms.StartTimer(timerA, 0);
	tms.StartTimer(timerB, 0);
	tms.StartTimer(timerC, 0);

	tms.StartTimersProcessTask();
	return 0;
}

void callB(TimerHandle_t t) {
	uint32_t tick = GetTickCount();
	vPrintStringAndNumber("One-shot timer callback executing", tick - start_tick);
}

void callC(TimerHandle_t t) {
	uint32_t tick = GetTickCount();
	vPrintStringAndNumber("Auto-reload timer callback executing", tick - start_tick);
}