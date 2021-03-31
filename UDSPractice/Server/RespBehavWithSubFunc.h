#pragma once
#include "BehaviourBase.h"
class RespBehavWithSubFunc : public Behaviour {
 public:
  //IsServerBusy
  bool CheckLengthBeyondMinum();//NRC x13
  //IsSIDSupported
  bool IsSubFuncSupportedForOneSID();//NRC x12
  //IsSIDSupportedInActiveSessiion
  bool IsSubFuncSupportedInActiveSessiionForOneSID();//NRC x7E
  //GetSIDSecurityCheckResult
  bool CheckRequestSequenceRespectedForThisSubFun();//NRC x24
};
