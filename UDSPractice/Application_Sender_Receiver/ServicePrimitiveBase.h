#pragma once

#include <stdio.h>

#define service_name_request(A_MType, A_SA, A_TA, A_TA_type, A_AE, A_Length, \
                             A_Data)                                         \
  printf(                                                                    \
      "A_MType,				A_SA,				"                                                \
      "		   A_TA,						"                                                     \
      "	")

#define DEFAULT_SOURCE_ADDRESS 0
#define DEFAULT_TARGET_ADDRESS 0

typedef enum ServicePrimitiveType {
  service_request_and_service_indication = 0,

  service_response_and_service_confirm = 1,

  service_request_confirm_and_service_response_confirm = 2,

} ServicePrimitiveType_t;

class ServicePrimitiveBase {
 public:
  ServicePrimitiveBase();
  ServicePrimitiveBase(
      A_MType message_type, A_SA source_address, A_TA target_address, A_TA_Type target_address_type /*, [A_AE], A_Length, A_Data[, parameter 1, ...]*/);
  virtual void execute();// = 0;
};

