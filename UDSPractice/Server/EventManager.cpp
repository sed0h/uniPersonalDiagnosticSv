#include <vector>
#include <thread>// <pthread.h>
#include <stdint.h>
#include "config.h"
#include "fifo/xif_safe.h"
#include "event_fifo.h"
#include "EventManager.h"

#include "RequestAndIndicationPrimitive.h"
#include "ServicePrimitiveBase.h"
std::vector<uint8_t> _rcv_buffer;
static std::thread* _thread_EventManager = NULL;
static std::thread* _thread_EventProcess = NULL;

ServicePrimitiveBase req_ind_spprocess[6];

static struct {
  Event_Type            _evt_typ;
  ServicePrimitiveBase  _sp_process;
} event_process[] = {
  {Service_Request,         req_ind_spprocess[0]},
  {Service_Request_Confirm, req_ind_spprocess[1]},
};


#define UDS_FIFO_BUFF_SIZE (UDS_FIFO_CAPACITY * sizeof(xif_event) + (64))
// 64 is estimated length (fifo_t)

xif_fifo_t* _fifo;
static void* evt_mngr;

static void loop_ManageEvent();
void InitEventManager() {

    if (evt_mngr != NULL) {
        printf("Event_Manager has Existed!!\n");
  }
  if (evt_mngr == NULL) {
    _rcv_buffer.resize(UDS_FIFO_BUFF_SIZE);
    xif_fifo_attr_t attr;
    attr.capacity = UDS_FIFO_CAPACITY;
    attr.unit = sizeof(xif_event);
    _fifo = xif_fifo_init(&(*_rcv_buffer.begin()), UDS_FIFO_BUFF_SIZE, attr);
    if (_fifo) {
      xif_fifo_setmode(_fifo, access_mimo);
    }

#if defined(_WIN32) || defined(__linux__)
    _thread_EventManager = new std::thread(loop_ManageEvent);
#endif  // defined(_WIN32) || defined(__linux__)
    if (NULL != _thread_EventManager) {
      printf("loop_fifo created succeed!\n");
    } else {
      printf("loop_fifo created failed!!\n");
    }
  }
}

RequestAndIndicationPrimitive sp_;
static void loop_ProcessService(void);
static void loop_ManageEvent() {
  xif_event event;
  while (true) {
    if (_fifo) {
      if (xif_fifo_pop(_fifo, (uint8_t*)&event)) {
        printf("%c event: %d, %s\n", event.service_event_type, event.event_id,
               event.data._ctx);
        // int ret_code = pthread_create(&_thread_id_process, NULL,
        //                               loop_process_service, (void*)&event);
        _thread_EventManager = new std::thread(loop_ProcessService);
        // if (NULL !=_thread_id_fifo ) {
        //   printf("start to process service!\n");
        // }
                
        // if (event.service_event_type == Service_Request_Confirm) {
        //     sp_.execute();
        // }
        req_ind_spprocess[event.service_event_type].execute();
      } 
    }
  }
}

static void loop_ProcessService(void) {

}

void PushEvent(const uint8_t* pData, uint16_t len) {
  xif_fifo_push(_fifo, (uint8_t*)pData);
  printf("pushed %d bytes to fifo\n", len);
}