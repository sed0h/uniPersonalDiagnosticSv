#include "ServiceDataUnit.h"
#include "ProtocolDataUnit.h"
#include "ServicePrimitiveBase.h"
#include "ResponseAndConfirmPrimitive.h"

ResponseAndConfirmPrimitive::ResponseAndConfirmPrimitive(A_Length bytes, A_Data data):
    ServicePrimitiveBase(msg_type_diagnostics, DEFAULT_SOURCE_ADDRESS, DEFAULT_TARGET_ADDRESS, targetaddress_type_physical),
      bytes_number_of_data(bytes), app_data(data) {
  printf("ResponseAndConfirmPrimitive constructed\n");
}
ResponseAndConfirmPrimitive::~ResponseAndConfirmPrimitive() {}

void ResponseAndConfirmPrimitive::execute() {
  printf(
      "srart to pass response data to the diagnostics application layer or "
  "the data passed in the service response primitive is successfully sent\n");
}
