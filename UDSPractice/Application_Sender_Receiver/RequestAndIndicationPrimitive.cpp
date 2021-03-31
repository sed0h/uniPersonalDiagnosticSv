#include "ServiceDataUnit.h"
#include "ProtocolDataUnit.h"
#include "ServicePrimitiveBase.h"
#include "RequestAndIndicationPrimitive.h"

RequestAndIndicationPrimitive::RequestAndIndicationPrimitive() {
}

 RequestAndIndicationPrimitive::RequestAndIndicationPrimitive(A_Length bytes, A_Data data):
    ServicePrimitiveBase(msg_type_diagnostics, DEFAULT_SOURCE_ADDRESS,DEFAULT_TARGET_ADDRESS, targetaddress_type_physical), 
     bytes_number_of_data(bytes), app_data(data) {
  printf("RequestAndIndicationPrimitive constructed\n");
}
 RequestAndIndicationPrimitive::~RequestAndIndicationPrimitive() {}
void RequestAndIndicationPrimitive::execute() {
  printf(
      "diagnostic tester application start to pass data to the diagnostics application layer in server, or"
     " pass data to the server function of the ECU diagnostic application \n");
 }

extern "C" void CallCFuncExecute(RequestAndIndicationPrimitive* p) {
  p->execute();
}
