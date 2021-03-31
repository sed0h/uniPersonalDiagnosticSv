
#if defined(_WIN32)
#include <WS2tcpip.h>
#include <WinInet.h>
#include <WinSock2.h>
#include <io.h>

#pragma comment(lib, "ws2_32.lib")
#elif defined(__linux__)
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bzero(a, b) memset(a, 0, b)
#if defined(_WIN32)
#define usleep(us) Sleep(us / 1000)
#define close closesocket
#define get_error GetLastError()
#elif defined(__linux__)
#define get_error errno
#endif

#include "doip_client.h"
#include "doip_pdu.h"
#include <pthread.h>

#define BUFFER_SIZE 4095

typedef struct sockaddr_s
{
    char ip[20];
    uint16_t port;
} sockaddr_t;

diagnostic_message_t *gdoip_msg = 0;
static int gsock = -1;
static sockaddr_t local_addr = {
    .ip = "127.0.0.1",
    .port = 7808,
};
static sockaddr_t remote_addr = {
    .ip = "127.0.0.1",
    .port = 13400,
};

static uint16_t src_addr = 0x0E82;
static uint16_t dst_addr = 0x0102;

static uint8_t recv_buf[BUFFER_SIZE];

static bool doip_sa_set = false;
static bool remote_unlink = false;
static struct sockaddr_in remote_address = {0};
static doip_recv_cb gRecvCb = NULL;
static pthread_t tid_rx;

static unsigned long get_tick()
{
    unsigned long tick = 0;

#if defined(_WIN32)
    tick = GetTickCount(); // about 15ms
#elif defined(__linux__)
    // Todo
#endif

    return tick;
}

static void Log(bool isOutput, const uint8_t *pData, size_t Length)
{
#if defined(_WIN32)
    printf("[%ld] %s (%zd) :", get_tick(), ((isOutput) ? ">>>" : "<<<"), Length);
#else
    printf("[%ld] %s (%ld) :", get_tick(), ((isOutput) ? ">>>" : "<<<"), Length);
#endif

    while (Length--)
    {
        printf(" %02X", *(pData++));
    }
    printf("\n");
}

static void recv_loop(void)
{
    int rc = -1;
    struct sockaddr_in SenderAddr;
    unsigned int SenderAddrSize = sizeof(SenderAddr);
    int remain_len;
    int total_len;
    int offset = 0;
    for (;;)
    {
        int recvLen = recv(gsock, &recv_buf[offset], BUFFER_SIZE, 0);
        if (recvLen > 0)
        {

            printf("[debug], doip recv data\n");
            if (recvLen <= 12)
            {
                printf("message too small!\n");
                offset += recvLen;
                continue;
            }
            offset += recvLen;
            diagnostic_message_t *msg = (diagnostic_message_t *)recv_buf;
            total_len = ntohl(msg->hdr.PayloadLength) + 8;
            if (offset < total_len)
            {
                printf("recv more message.\n");
                continue;
            }
            if (ntohs(msg->hdr.PayloadType) != 0x8001)
            {
                printf("[debug], hdr.PayloadType != 0x8001\n");
                offset -= total_len;
                continue;
            }
            int data_len = ntohl(msg->hdr.PayloadLength) - 4;
            if (gRecvCb)
            {
                Log(false, (const uint8_t *)recv_buf, recvLen);
                gRecvCb(msg->userdata, data_len);
            }
            offset -= total_len;
        }
        else if (recvLen == 0)
        {
            printf("[error], recv len = 0\n");
            continue;
        }
        else
        {
            printf("[error], recv len < 0, errno : %d\n", get_error);
            //printf("reconnect condition, gsock: %d, remote_unlink = %d\n", gsock, remote_unlink);
            if (remote_unlink)
            {
                if (0 == gsock)
                {
                    gsock = socket(PF_INET, SOCK_STREAM, 0);
                    if (gsock < 0)
                    {
                        printf("create socket failed in reconnect, errno : %d\n", get_error);
                        return;
                    }
                }

                rc = connect(gsock, (struct sockaddr *)&remote_address, sizeof(remote_address));
                if (0 == rc)
                {
                    printf("reconnect... successful!\n");
                    remote_unlink = false;
                    doip_routing_active();
                }
                else
                {
                    //printf("reconnect failed, errno : %d\n", get_error);
                    // 1s reconnect
                    usleep(1 * 1000 * 1000);
                }
            }
            else
            {
                rc = close(gsock);
                if (0 == rc)
                {
                    printf("remote server unlink...\n");
                    gsock = 0;
                    remote_unlink = true;
                }
            }
        }
    }
}

void set_ipaddr(char *local, char *remote)
{
    int len1 = strlen(local);
    int len2 = strlen(remote);

    memcpy(local_addr.ip, local, len1);
    memcpy(remote_addr.ip, remote, len2);
}

int doip_client_init(void)
{
    int ret = -1;
    int opt = 1;
    struct sockaddr_in address;

    // if (!local_addr || !remote_addr)
    // {
    //     printf("error: local_ip or uds_payload is null!\n");
    //     return -1;
    // }
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, local_addr.ip, &address.sin_addr);
    address.sin_port = htons(local_addr.port);

    gsock = socket(PF_INET, SOCK_STREAM, 0);
    if (gsock < 0)
    {
        printf("creat socket failed, errno : %d\n", get_error);
        return -1;
    }
    // ret = setsockopt(gsock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
    // if (ret < 0)
    // {
    //     printf("setsockopt failed, errno : %d\n", get_error);
    //     return -1;
    // }
    // ret = bind(gsock, (struct sockaddr *)&address, sizeof(address));
    // if (ret < 0)
    // {
    //     printf("bind failed, errno : %d\n", get_error);
    //     return -1;
    // }

    bzero(&remote_address, sizeof(remote_address));
    remote_address.sin_family = AF_INET;
    inet_pton(AF_INET, remote_addr.ip, &remote_address.sin_addr);
    remote_address.sin_port = htons(remote_addr.port);

    ret = connect(gsock, (struct sockaddr *)&remote_address, sizeof(remote_address));
    if (ret < 0)
    {
        printf("connect failed, errno : %d\n", get_error);
        remote_unlink = true;
        goto __exit;
    }

    doiphdr_create();

__exit:
    pthread_create(&tid_rx, NULL, (void *)recv_loop, NULL);

    return 0;
}

int doiphdr_create(void)
{
    if (NULL == gdoip_msg)
    {
        gdoip_msg = (diagnostic_message_t *)malloc(65535);
        if (!gdoip_msg)
        {
            printf("malloc failed!\n");
            return -1;
        }
    }

    if (gdoip_msg)
    {
        gdoip_msg->hdr.ProtocolVersion = 0x02;
        gdoip_msg->hdr.InverseProtocolVersion = 0xFD;
        gdoip_msg->hdr.PayloadType = 0x8001;
        gdoip_msg->hdr.PayloadLength = 4;
        gdoip_msg->sa[1] = htons(src_addr) >> 8;
        gdoip_msg->sa[0] = (uint8_t)htons(src_addr);
        gdoip_msg->ta[1] = htons(dst_addr) >> 8;
        gdoip_msg->ta[0] = (uint8_t)htons(dst_addr);

        doip_sa_set = true;
    }

    return 0;
}

int doip_routing_active()
{
    routing_activation_request_t rout_req;

    if (remote_unlink)
    {
        return -1;
    }

    // for first init is interrupt exit ( connect fail for no server ), so must be set the sa address info;
    if (!doip_sa_set)
    {
        doiphdr_create();
    }

    rout_req.hdr.ProtocolVersion = 0x02;
    rout_req.hdr.InverseProtocolVersion = 0xFD;
    rout_req.hdr.PayloadType = htons(0x0005);
    rout_req.hdr.PayloadLength = htonl(0x0b);
    rout_req.ActivationType = 0xE0;
    memset(&rout_req.iso_rsv, 0, sizeof(rout_req.iso_rsv));
    memset(&rout_req.oem_specific, 0, sizeof(rout_req.oem_specific));
    rout_req.SourceAddress[1] = gdoip_msg->sa[1];
    rout_req.SourceAddress[0] = gdoip_msg->sa[0];

    int ret = send(gsock, (const uint8_t *)&rout_req, sizeof(rout_req), 0);
    if (ret < 0)
    {
        printf("send failed, errno : %d\n", get_error);
        return -1;
    }
    else
    {
        printf("send sucess, send %d bytes!\n", ret);
        return ret;
    }
}

int doip_client_send(uint8_t *payload, uint32_t len)
{
    if (!payload || !gdoip_msg)
    {
        printf("error: payload or gdoip_msg is null!");
        return -1;
    }

    uint32_t send_len = 0;
    memcpy(gdoip_msg->userdata, payload, len);
    gdoip_msg->hdr.PayloadLength += len;
    send_len = gdoip_msg->hdr.PayloadLength + 8;
    gdoip_msg->hdr.PayloadLength = htonl(gdoip_msg->hdr.PayloadLength);
    gdoip_msg->hdr.PayloadType = htons(gdoip_msg->hdr.PayloadType);

    Log(true, (const uint8_t *)gdoip_msg, 14);

    int ret = send(gsock, (const uint8_t *)gdoip_msg, send_len, 0);
    if (ret < 0)
    {
        printf("send failed, errno : %d\n", get_error);
        return -1;
    }
    else
    {
        printf("send sucess, send %d bytes!\n", ret);
        gdoip_msg->hdr.PayloadLength = 4;
        gdoip_msg->hdr.PayloadType = 0x8001;
        return ret;
    }
}

void doip_register_cb(doip_recv_cb cb) { gRecvCb = cb; }

// int doip_client_recv(uint8_t *recv_buf, uint32_t buf_len, uint8_t *payload)
// {
//     if (buf_len <= 12)
//     {
//         printf("message too small!\n");
//         return -1;
//     }
//     diagnostic_message_t *msg = (diagnostic_message_t *)recv_buf;
//     int recv_len = ntohl(msg->hdr.PayloadLength) - 4;
//     memcpy(payload, msg->userdata, recv_len);
//     return recv_len;
// }

// int main(int argc, char *argv[])
// {

// doip_client_init(&local, &remote, 0x1234, 0x4567);
//     doip_routing_active();
//     uint8_t payload[] = {0x10, 0x03};
//     doip_client_send(payload, sizeof(payload));
// }
