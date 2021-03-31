#include "ServiceDataUnit.h"
#include "ProtocolDataUnit.h"
#include "ServicePrimitiveBase.h"
#include "RequestConfirmAndResponseConfirmPrimitive.h"

 RequestConfirmAndResponseConfirmPrimitive::RequestConfirmAndResponseConfirmPrimitive(A_Result r):
    ServicePrimitiveBase(msg_type_diagnostics, DEFAULT_SOURCE_ADDRESS, DEFAULT_TARGET_ADDRESS, targetaddress_type_physical),
      result(r) {
  printf("RequestConfirmAndResponseConfirmPrimitive constructed\n");
}

 RequestConfirmAndResponseConfirmPrimitive::
    ~RequestConfirmAndResponseConfirmPrimitive() {}

void RequestConfirmAndResponseConfirmPrimitive::execute() {
   printf(
      "the data passed in the service request primitive successfully sent or "
       "the data passed in the service response primitive successfully sent\n");
 }
