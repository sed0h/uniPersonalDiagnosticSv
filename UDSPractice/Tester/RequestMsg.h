#pragma once
#include "ProtocolDataUnit.h"

typedef union {
  A_PDU msg;
  struct {
    MType message_type;
    SA source_address;
    TA target_address;
    TA_Type target_address_type = targetaddress_type_physical;
    RA remote_address;
    P_Data protocol_data;
    Length bytes_number_of_data;
  } m;

} PhysicalAddressedReqMsg;

typedef union {
  A_PDU msg;
  struct {
    MType message_type;
    SA source_address;
    TA target_address;
    TA_Type target_address_type = targetaddress_type_functional;
    RA remote_address;
    P_Data protocol_data;
    Length bytes_number_of_data;
  } m;

} FunctionalAddressedReqMsg;