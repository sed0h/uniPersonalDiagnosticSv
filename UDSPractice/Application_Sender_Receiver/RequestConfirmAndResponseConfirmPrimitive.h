#pragma once

 class RequestConfirmAndResponseConfirmPrimitive : ServicePrimitiveBase {
 public:
  RequestConfirmAndResponseConfirmPrimitive(A_Result r);
  ~RequestConfirmAndResponseConfirmPrimitive();
  void execute();
 private:
  A_Result result;
};
