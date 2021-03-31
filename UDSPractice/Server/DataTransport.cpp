#include <stdint.h>
#include "Config.h"
#include "fifo/xif_safe.h"
#include <vector>
#include <thread>
#include "DataTransport.h"
#include "EventManager.h"

#if UDS_ON_DOIP

#else
#include <stdio.h>
#include "udppipe.h"
#endif// UDS_ON_DOIP


void ExecutesServicePrimitive(const uint8_t* pData, uint16_t len);

void InitDataTransport() {
  UDP_PIPE_InitA();  // A,server B,tester
  
  InitEventManager();

  UDP_PIPE_SetStreamCallback(ExecutesServicePrimitive);
  
}

void ExecutesServicePrimitive(const uint8_t* pData, uint16_t len) {
  if (pData) {
    PushEvent(pData, len);
  }
}


