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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#if defined( __STD_C11__ )
#include <thread>

static std::thread t;
#else
#include <pthread.h>

static pthread_t tid_rx;
#endif

#if defined(_WIN32)
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;

#else
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define WSACleanup()      NULL
#define WSAGetLastError() errno
#define closesocket(X)    close(X)
#define SOCKET_ERROR      (-1)

#include <time.h>
#endif

#include "udppipe.h"


#define MAKEIP(A, B, C, D)                                                                         \
    ((uint32_t)(((uint8_t)(A) << 24) | ((uint8_t)(B) << 16) | ((uint8_t)(C) << 8) | (uint8_t)(D)))


typedef struct _endpoint {
    uint32_t IP;
    uint16_t Port;
} endpoint;

struct {
    endpoint SizeA;
    endpoint SizeB;
} Param = {{MAKEIP(127, 0, 0, 1), 8000}, {MAKEIP(127, 0, 0, 1), 8001}};

#define BUFF_SIZE (1024)

unsigned long get_sys_tick() {
    unsigned long tick = 0;

#if defined(_WIN32)
    tick = GetTickCount();  // about 15ms
#elif defined(__linux__)
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    tick = ((time.tv_sec * 1000) + (time.tv_nsec / 1000000L));
#endif

    return tick;
}

static uint8_t                ReceiveBuf[BUFF_SIZE];
static pfUDPPipeDataArrived   gDataCallback   = NULL;
static pfUDPPipeStreamArrived gStreamCallback = NULL;

void UDP_PIPE_SetDataCallback(pfUDPPipeDataArrived fCallback) {
    gDataCallback = fCallback;
}

void UDP_PIPE_SetStreamCallback(pfUDPPipeStreamArrived fCallback) {
    gStreamCallback = fCallback;
}

static void Log(bool isOutput, const uint8_t *pData, size_t Length) {
#if defined(_WIN32)
    printf("[%ld] %s (%zd) :", get_sys_tick(), ((isOutput)?">>>":"<<<"), Length);
#else
    printf("[%ld] %s (%ld) :", get_sys_tick(), ((isOutput) ? ">>>" : "<<<"), Length);
#endif

    while (Length--) {
        printf(" %02X", *(pData++));
    }
    printf("\n");
}

static int ReceivingSocket;

void ReceiveThread(void) {
    struct sockaddr_in SenderAddr;
    socklen_t          SenderAddrSize = sizeof(SenderAddr);

    for (;;) {
        int ByteReceived = recvfrom(ReceivingSocket, (char *)ReceiveBuf, BUFF_SIZE, 0,
                                    (struct sockaddr *)&SenderAddr, &SenderAddrSize);
        if (ByteReceived > 0) {
            Log(false, &ReceiveBuf[0], ByteReceived);
            if (gStreamCallback) {
                gStreamCallback(&ReceiveBuf[0], ByteReceived);
            }
            if (gDataCallback) {
                for (int index = 0; index < ByteReceived; ++index) {
                    gDataCallback(ReceiveBuf[index]);
                }
            }
        } else {
            break;
        }
    }
}

static void UDP_PIPE_Init(endpoint thisSide) {
    struct sockaddr_in ReceiverAddr;

#if defined(WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Server : WSAStartup failed with error % ld\n", WSAGetLastError());
        return;
    } else {
        printf("Server : The Winsock DLL status is % s.\n", wsaData.szSystemStatus);
    }
#endif

    // Create a new socket to receive datagrams on.
    ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ReceivingSocket < 0) {
        printf("Server : Error at socket() : %d\n", WSAGetLastError());
        WSACleanup();
        return;
    } else {
        printf("Server : socket() is OK !\n");
    }
    ReceiverAddr.sin_family      = AF_INET;
    ReceiverAddr.sin_port        = htons(thisSide.Port);
    ReceiverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(ReceivingSocket, (struct sockaddr *)&ReceiverAddr, sizeof(ReceiverAddr)) ==
        SOCKET_ERROR) {
        printf("Server : bind() failed !Error : %d.\n", WSAGetLastError());
        closesocket(ReceivingSocket);
        WSACleanup();
        return;

    } else {
        printf("Server : bind() is OK !\n");
    }

#if defined(__STD_C11__)
    t = std::thread(ReceiveThread);
#else
	pthread_create(&tid_rx, NULL, (void*)ReceiveThread, NULL);
#endif
}

void UDP_PIPE_DeInit(void) {
    closesocket(ReceivingSocket);
#if defined(__STD_C11__)
    t.join();
#else
    pthread_join( tid_rx, NULL );
#endif
}

static endpoint DestPoint;

void UDP_PIPE_InitA(void) {
    DestPoint = Param.SizeB;
    UDP_PIPE_Init(Param.SizeA);
}

void UDP_PIPE_InitB(void) {
    DestPoint = Param.SizeA;
    UDP_PIPE_Init(Param.SizeB);
}

void UDP_PIPE_Send(const uint8_t *pData, uint16_t Length) {
    struct sockaddr_in SenderAddr;
    int                SenderAddrSize = sizeof(SenderAddr);

    SenderAddr.sin_family      = AF_INET;
    SenderAddr.sin_port        = htons(DestPoint.Port);
    SenderAddr.sin_addr.s_addr = htonl(DestPoint.IP);

    Log(true, pData, Length);

    sendto(ReceivingSocket, (const char *)pData, Length, 0, (const struct sockaddr *)&SenderAddr,
           SenderAddrSize);
}
