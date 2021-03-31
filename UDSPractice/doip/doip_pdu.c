#include "doip_pdu.h"

#if defined(WIN32)
#define __little_endian__
#pragma message("Warning: please sure the target is __little_endian__")

#elif defined(__linux__)
#define __little_endian__
#pragma message("Warning: please sure the target is __little_endian__")
#else
#pragma error("error: must assign machine endian!!!");
#endif

#if !defined(__big_endian__) && !defined(__little_endian__)
#error "ERROR: You must defined __big_endian__ or __little_endian__ first!!! ( Don't modify the C code, just use compiler option )"
#endif

#if defined(__big_endian__)
#define BigLittleSwap16(A)	(A)
#define BigLittleSwap32(A)	(A)
#endif

#if defined(__little_endian__)
#define BigLittleSwap16(A) \
	((((uint16_t)(A) & 0xff00) >> 8) | \
	(((uint16_t)(A) & 0x00ff) << 8))  

#define BigLittleSwap32(A) \
	((((uint32_t)(A) & 0xff000000) >> 24) | \
	(((uint32_t)(A) & 0x00ff0000) >> 8) | \
	(((uint32_t)(A) & 0x0000ff00) << 8) | \
	(((uint32_t)(A) & 0x000000ff) << 24))
#endif

#define HTONS(s) BigLittleSwap16(s)
#define HTONL(l) BigLittleSwap32(l)

#define INIT_HDR(id, type)	{ \
	(uint8_t)DOIP_PROTOCOL_VERSION,	\
	~(uint8_t)DOIP_PROTOCOL_VERSION,	\
	HTONS(id),	\
	HTONL(sizeof(type) - sizeof(doip_hdr_t))}	\

static generic_nack_t GENERIC_NACK = {
	INIT_HDR(DOIP_GENERIC_NACK, generic_nack_t),
	0, /*code*/
	/*end*/
};

static vehicle_identification_request_t VI_REQ = {
	INIT_HDR(DOIP_VI_REQUEST, vehicle_identification_request_t),
	/*end*/
};

static vehicle_identification_request_eid_t VI_REQ_EID = {
	INIT_HDR(DOIP_VI_REQUEST_BY_EID, vehicle_identification_request_eid_t),
	{ 1, 2, 3, 4, 5, 6 },
	/*end*/
};

static vehicle_identification_request_vin_t VI_REQ_VIN = {
	INIT_HDR(DOIP_VI_REQUEST_BY_VIN, vehicle_identification_request_vin_t),
	{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 },
	/*end*/
};

static vehicle_identification_response_t VI_RSP = {
	INIT_HDR(DOIP_VI_RESPONSE, vehicle_identification_response_t),
	{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 },
	{ 0, 0 },
	{ 1, 2, 3, 4, 5, 6 },
	{ 6, 5, 4, 3, 2, 1 },
	DOIP_VI_NO_FURTHER_ACTION_REQUIRED,
	DOIP_VI_NOT_SYNCHRONIZED,
	/*end*/
};

static routing_activation_request_t RA_REQ = {
	INIT_HDR(DOIP_ROUTING_ACTIVATION_REQUEST, routing_activation_request_t),
	{ 0, 0 },			/*Source Address*/
	{ 0 },				/*Activation Type*/
	{ 0, 0, 0, 0 },		/*rsv*/
	{ 0, 0, 0, 0 },		/*oem_specific*/
	/*end*/
};

static routing_activation_response_t RA_RSP = {
	INIT_HDR(DOIP_ROUTING_ACTIVATION_RESPONSE, routing_activation_response_t),
	{ 0, 0 },			/*Equipment Address*/
	{ 0, 0 },			/*Enity Address*/
	{ 0 },				/*Response Code*/
	{ 0, 0, 0, 0 },		/*rsv*/
	{ 0, 0, 0, 0 },		/*oem_specific*/
	/*end*/
};


static diagnostic_message_t DIAG_MSG = {
	INIT_HDR(DOIP_DIAGNOSTIC, diagnostic_message_t),
	{ 0, 0 },			/*Source Address*/
	{ 0, 0 },			/*Target Address*/
	/*variable: userdata*/
};

static diagnostic_nack_t DIAG_NACK = {
	INIT_HDR(DOIP_DIAGNOSTIC_NACK, diagnostic_nack_t),
	{ 0, 0 },			/*Source Address*/
	{ 0, 0 },			/*Target Address*/
	{ 0 }				/*NACK code */
	/*variable: Previous diagnostic message data */
};

static diagnostic_ack_t DIAG_ACK = {
	INIT_HDR(DOIP_DIAGNOSTIC_ACK, diagnostic_ack_t),
	{ 0, 0 },			/*Source Address*/
	{ 0, 0 },			/*Target Address*/
	{ 0 }				/*ACK code */
	/*variable: Previous diagnostic message data */
};

static alive_check_request_t AC_REQ = {
	 INIT_HDR(DOIP_ALIVE_CHECK_REQUEST, alive_check_request_t),
	 /*end*/
};

static alive_check_response_t AC_RSP = {
	INIT_HDR(DOIP_ALIVE_CHECK_RESPONSE, alive_check_response_t),
	{ 0, 0 },	/*Source Address*/
	/*end*/
};

static power_mode_request_t PM_REQ = {
	INIT_HDR(DOIP_POWER_MODE_REQUEST, power_mode_request_t),
	/*end*/
};

static power_mode_response_t PM_RSP = {
	INIT_HDR(DOIP_POWER_MODE_RESPONSE, power_mode_response_t),
	{ 0 },		/*Diagnostic power mode */
	/*end*/
};

static entity_status_request_t ES_REQ = {
	INIT_HDR(DOIP_ENTITY_STATUS_REQUEST, entity_status_request_t),
	/*end*/
};

static entity_status_response_t ES_RSP = {
	INIT_HDR(DOIP_ENTITY_STATUS_RESPONSE, entity_status_response_t),
	{ 0 },				/*Node type */
	{ 0 },				/*Max. concurrent TCP_DATA sockets : 1 ~ 255 */
	{ 0 },				/*Currently open TCP_DATA sockets : 0 ~ 255 */
	{ 0, 0, 0, 0 },		/*Max.data size(MDS) : 0 ~ 4GB */
	/*end*/
};

/*WARNING: Don't change the order, it maybe due to fatal error !!!! */
doip_message_t *DOIP_MSGS[IDX_MAX_NUM] = {
	(doip_hdr_t*)&GENERIC_NACK,	
	(doip_hdr_t*)&VI_REQ,
	(doip_hdr_t*)&VI_REQ_EID,
	(doip_hdr_t*)&VI_REQ_VIN,
	(doip_hdr_t*)&VI_RSP,
	(doip_hdr_t*)&RA_REQ,		/*TCP Data*/
	(doip_hdr_t*)&RA_RSP,		/*TCP Data*/
	(doip_hdr_t*)&DIAG_MSG,		/*TCP Data*/
	(doip_hdr_t*)&DIAG_NACK,	/*TCP Data*/
	(doip_hdr_t*)&DIAG_ACK,		/*TCP Data*/
	(doip_hdr_t*)&AC_REQ,		/*TCP Data*/
	(doip_hdr_t*)&AC_RSP,		/*TCP Data*/
	(doip_hdr_t*)&PM_REQ,
	(doip_hdr_t*)&PM_RSP,
	(doip_hdr_t*)&ES_REQ,
	(doip_hdr_t*)&ES_RSP,
};


