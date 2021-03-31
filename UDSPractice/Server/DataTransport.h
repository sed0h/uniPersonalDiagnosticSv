
#ifndef __DATA_TRANSPORT__
#define __DATA_TRANSPORT__

typedef void (*pfDataTransportCallback)(const uint8_t* arg1, uint16_t arg2);

void InitDataTransport(/*pfDataTransportCallback fCallback*/);

#endif  // !__DATA_TRANSPORT__

