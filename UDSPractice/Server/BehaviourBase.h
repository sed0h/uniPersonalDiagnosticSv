#pragma once
//#include "ProtocolDataUnit"
#include "ServiceDataUnit.h"

class NegativeResponseAndConfirmPrimitive;

class Behaviour {
 private:
  NegativeResponseAndConfirmPrimitive *instance_negative_resp_confirm_primitive;

 public:
  bool IsServerBusy();//NRC x21
  bool ManufactureSpecificFailureDetected();  // event?
  bool IsSIDSupported();//NRC x11
  bool IsSIDSupportedInActiveSessiion();//NRC x7F
  bool GetSIDSecurityCheckResult();//NRC x33
  bool SupplierSpecificFailureDetected();  // event?
  bool IsServiceWithSubFuncExceptX31();

  void ReturnNegativeResponseOrConfirmation(DATA_Type nrc_value);
};
