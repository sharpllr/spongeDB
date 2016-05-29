/*
 * ut_clt_mirror.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */
#ifdef _UT_CLI_
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include "anet.h"
#include "sponge.h"
#include "ut.h"
#include "util/spg_util.h"

struct mr_clt {
    char* srvIp;
    int32_t srvPort;
    int32_t srvSock;
};

int32_t conToSrv(struct mr_clt *clt, char* ipd, int32_t srvPort)
{
    int32_t ret;
    char _port[6];
    struct addrinfo hints, *srvAddrInfo;

    snprintf(_port, sizeof(_port), "%d", srvPort);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    //For connect
    ret = getaddrinfo(clt->srvIp, _port, &hints, &srvAddrInfo);
    if (ret) {
        dbg_assert(0 == ret);
        logErr("Get address infor failed.");
        return SPG_ERR;
    }

    clt->srvSock = socket(srvAddrInfo->ai_family, srvAddrInfo->ai_socktype,
        srvAddrInfo->ai_protocol);

    anetKeepAlive(NULL, clt->srvSock, 1);
    anetNonBlock(NULL, clt->srvSock);
    anetEnableTcpNoDelay(NULL, clt->srvSock);
    uint32_t times = 10;
    while (times) {
        ret = connect(clt->srvSock, srvAddrInfo->ai_addr,
            srvAddrInfo->ai_addrlen);
        if (-1 == ret) {
            if (errno == EHOSTUNREACH) {
                logEasy("Host unreachable.");
                return SPG_ERR;
            }
            //When socket is unblock,errno maybe EINPROGRESS,need handle.
            times--;
            continue;
        }
        break;
    }

    if (times == 0) {
        return SPG_ERR;
    }

    logEasy("Connect to server success.");
    return SPG_OK;
}

void readFromSrv(struct mr_clt *clt)
{
    char buf[128];
    int32_t rdLen;
    rdLen = anetRead(clt->srvSock, buf, sizeof(buf));
    logEasy("Read %d data:%s.", rdLen, buf);
}

void sendToSrv(struct mr_clt *clt, char *data, uint32_t len)
{
    int32_t wrLen;
    wrLen = anetWrite(clt->srvSock, data, len);
    logEasy("Write %d byte data to server.", wrLen);
}


struct mr_clt g_mr_clt;
void launch_client(int argc, char** argv)
{
    char buf[128];
    int32_t ret;
    if (argc < 3) {
        logEasy("Need IP and port of server.");
        return;
    }

    g_mr_clt.srvIp   = strdup(argv[1]);
    g_mr_clt.srvPort = strtol(argv[2], NULL, 0);

    ret = conToSrv(&g_mr_clt, g_mr_clt.srvIp, g_mr_clt.srvPort);
    if (ret) {
        logErr("Connection to server failed.");
        dbg_assert(0 == ret);
    }

    while(1) {
        printf("Input data:\n");
        scanf("%s", buf);
        sendToSrv(&g_mr_clt, buf, strlen(buf));
//        readFromSrv(&g_mr_clt);
    }
    zfree(g_mr_clt.srvIp);
    return;
}

#endif
