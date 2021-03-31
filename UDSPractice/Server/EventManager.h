#pragma once

void InitEventManager();
void loop_ManageEvent(void);
void PushEvent(const uint8_t* pData, uint16_t len);
