#pragma once

class ResponseAndConfirmPrimitive : ServicePrimitiveBase {
 public:
  ResponseAndConfirmPrimitive(A_Length bytes, A_Data data);
  ~ResponseAndConfirmPrimitive();
  void execute();
 private:
  A_Length bytes_number_of_data;
  A_Data app_data;
};
