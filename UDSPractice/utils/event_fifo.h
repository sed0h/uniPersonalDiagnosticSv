/* this file is for event use ,data use fifo files directly */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	Service_Request = 0,
	Service_Request_Confirm,
	Service_Indication,
	Service_Response,
	Service_Response_Confirm,
	Service_Confirm,
	
} Event_Type;

typedef struct {
	Event_Type service_event_type;
	uint8_t event_id;
        union {
          uint8_t* _ctx;
          uint8_t buffer[UDS_FIFO_DATAUNIT_MAX_SIZE];
		} data;
} xif_event;


#ifdef __cplusplus
}
#endif