//---------------------------------------------------------------------------------------------------------------------
//
// E-PLANET CORPORATION CONFIDENTIAL
// ________________________________
//
// [2020] E-Planet Corporation
// All Rights Reserved.
//
// NOTICE: This is an unpublished work of authorship, which contains trade secrets.
// E-Planet Corporation owns all rights to this work and intends to maintain it in confidence to
// preserve its trade secret status. E-Planet Corporation reserves the right, under the copyright
// laws of the United States or those of any other country that may have jurisdiction, to protect
// this work as an unpublished work, in the event of an inadvertent or deliberate unauthorized
// publication. E-Planet Corporation also reserves its rights under all copyright laws to protect
// this work as a published work, when appropriate. Those having access to this work may not copy
// it, use it, modify it, or disclose the information contained in it without the written
// authorization of E-Planet Corporation.
//
//---------------------------------------------------------------------------------------------------------------------
#if !defined(_UDP_PIPE_H_)
#define _UDP_PIPE_H_

#if defined(__cplusplus)
extern "C" {
#endif

void UDP_PIPE_InitA(void);

void UDP_PIPE_InitB(void);

void UDP_PIPE_DeInit(void);

void UDP_PIPE_Send(const uint8_t *pData, uint16_t Length);

typedef void (*pfUDPPipeDataArrived)(uint8_t Data);

void UDP_PIPE_SetDataCallback(pfUDPPipeDataArrived fCallback);

typedef void (*pfUDPPipeStreamArrived)(const uint8_t *pData, uint16_t Length);

void UDP_PIPE_SetStreamCallback(pfUDPPipeStreamArrived fCallback);

#if defined(__cplusplus)
}
#endif

#endif // _UDP_PIPE_H_
