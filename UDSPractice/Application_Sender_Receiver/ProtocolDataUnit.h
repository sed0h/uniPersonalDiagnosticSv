#pragma once

typedef uint8_t SI;
typedef uint8_t NRC;

typedef struct {
	//const uint8_t NR_SI = 0x7F;
	SI Service_identifier;
} A_PCI;

typedef A_MType MType;
typedef A_SA SA;
typedef A_TA TA;
typedef A_TA_Type TA_Type;
typedef A_AE RA;

typedef struct {
	A_PCI protocol_control_information;
	std::vector<DATA_Type> parameters;
} P_Data;
typedef A_Length Length;

typedef struct {
	MType message_type;
	SA source_address;
	TA target_address;
	TA_Type target_address_type;
	RA remote_address;
	P_Data protocol_data;
	Length bytes_number_of_data;
} A_PDU;

