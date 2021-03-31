#include "BehaviourBase.h"

bool Behaviour::IsServerBusy() { return false; }

bool Behaviour::ManufactureSpecificFailureDetected() { return false; }

bool Behaviour::IsSIDSupported() { return false; }

bool Behaviour::IsSIDSupportedInActiveSessiion() { return false; }

bool Behaviour::GetSIDSecurityCheckResult() { return false; }

bool Behaviour::SupplierSpecificFailureDetected() { return false; }

bool Behaviour::IsServiceWithSubFuncExceptX31() { return false; }

void Behaviour::ReturnNegativeResponseOrConfirmation(DATA_Type nrc_value) {
  // negative_resp_confirm_instance->
}
