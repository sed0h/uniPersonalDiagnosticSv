#include "ServiceDataUnit.h"
#include "ProtocolDataUnit.h"
#include "ServicePrimitiveBase.h"
#include "NegativeResponseAndConfirmPrimitive.h"

NegativeResponseAndConfirmPrimitive::NegativeResponseAndConfirmPrimitive(A_PCI pci_data, NRC nrc) :
    ServicePrimitiveBase(msg_type_diagnostics, DEFAULT_SOURCE_ADDRESS, DEFAULT_TARGET_ADDRESS, targetaddress_type_physical),
      pci_of_data(pci_data), nrc_value(nrc) {
  printf("NegativeResponseAndConfirmPrimitive\n");
}
NegativeResponseAndConfirmPrimitive::~NegativeResponseAndConfirmPrimitive() {}

void NegativeResponseAndConfirmPrimitive::execute() { 
    printf(" "); 
}
