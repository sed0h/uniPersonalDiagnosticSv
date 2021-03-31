#include "ServiceDataUnit.h"
#include "ProtocolDataUnit.h"
#include "ServicePrimitiveBase.h"

ServicePrimitiveBase::ServicePrimitiveBase(){}

ServicePrimitiveBase::ServicePrimitiveBase(A_MType message_type,
                                           A_SA source_address,
                                           A_TA target_address,
                                           A_TA_Type target_address_type) {
  printf("ServicePrimitiveBase consturcted\n");
}

