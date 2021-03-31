#pragma once
#include <cstdint>
#include <vector>


#define DATA_Type uint8_t

typedef enum {
	msg_type_diagnostics = 0,			//A_SA, A_TA, A_TAtype.
	msg_type_remote_diagnostics = 1,	//A_SA, A_TA, A_TAtype and A_AE.
} A_MType;								//Application layer message type

typedef uint16_t A_SA;

typedef uint16_t A_TA;

typedef enum {
	targetaddress_type_physical = 0, 
	targetaddress_type_functional = 1,
} A_TA_Type;

typedef enum {
	result_ok = 0,
	result_error = 1,
} A_Result;

typedef uint32_t A_Length;

typedef std::vector<DATA_Type> A_Data;

typedef uint16_t A_AE;