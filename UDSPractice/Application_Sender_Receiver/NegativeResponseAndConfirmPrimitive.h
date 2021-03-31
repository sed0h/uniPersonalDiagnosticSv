#pragma once

class NegativeResponseAndConfirmPrimitive : ServicePrimitiveBase {
 public:
  NegativeResponseAndConfirmPrimitive(A_PCI pci_data, NRC nrc);
  ~NegativeResponseAndConfirmPrimitive();
  void execute();

 private:
  A_PCI pci_of_data;
  NRC nrc_value;
};