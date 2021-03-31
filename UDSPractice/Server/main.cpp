#include <stdint.h>
#include "DataTransport.h"

//#include "BehaviourBase.h"
//#include "EventManager.h"
//#include <map>

//#include "config.h"
//#include "event_fifo.h"
//#include "fifo/xif_safe.h"



// std::map<uint8_t, RequestAndIndicationPrimitive> sp_map = {{1, sp_}, {10,
// sp_}}; std::map<uint8_t, char> ch_map = {{1, 'e'}, {3, 'r'}};
//xif_event event;

static void* loop_fifo(void* arg);


int main() {
  InitDataTransport();
    
  // Behaviour *behavr = new Behaviour();
  // behavr->ReturnNegativeResponseAndConfirmation(0x31);

  while (true) {
  }
  return 0;
}

