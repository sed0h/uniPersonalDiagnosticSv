#pragma once

class RequestAndIndicationPrimitive : public ServicePrimitiveBase {
public:
  RequestAndIndicationPrimitive();
  RequestAndIndicationPrimitive(A_Length bytes, A_Data data);
  ~RequestAndIndicationPrimitive();
  void execute();

 private:
  A_Length bytes_number_of_data;
  A_Data app_data;
};
