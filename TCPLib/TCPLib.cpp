// TCPLib.cpp : Defines the functions for TCP communication.
//

#pragma once

#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS




#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "pch.h"
#include "framework.h"

#include "../Common/LinkedList.h";
#include "../Common/Measurment.h";

#pragma comment(lib,"WS2_32")


void TCPSendMeasurment(SOCKET connectSocket, Measurment measurment) {
    int iResult = send(connectSocket, (const char*)&measurment, sizeof(Measurment), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        //closesocket(connectSocket);
        //WSACleanup();
    }
}


Measurment TCPReceiveMeasurment(SOCKET connectSocket, int len) {
    Measurment* recvbuf = (Measurment*)malloc(sizeof(Measurment));
    int iResult = recv(connectSocket, (char*)recvbuf, len, 0);
    if (iResult > 0)
    {
        //printf("Msg value: %d\n", recvbuf->value);
    }
    else
    {
        // there was an error during recv
        printf("recv failed with error: %d\n", WSAGetLastError());
    }
    return *recvbuf;
}