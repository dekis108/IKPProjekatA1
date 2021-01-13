// TCPLib.cpp : Defines the functions for TCP communication.
//

//#pragma once
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "pch.h"
#include "framework.h"

#include "../Common/LinkedList.h";
#include "../Common/Measurment.h";

#pragma comment(lib,"WS2_32")


#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#define BUFFER 100

/*
* connectSocket = socket of the receiver of the message
* type = type of the message, will be concatinated in the message, consult MessageTypes in Common
* *msg = data to be sent
*/
bool TCPSendData(SOCKET connectSocket, int type, void *msg) {
    char data[BUFFER];
    data[0] = (char)type;
    strcpy(&data[1], (const char *)msg);


    int iResult = send(connectSocket, data, sizeof(data), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        //closesocket(connectSocket);
        //WSACleanup();
        return false;
    }
    return true;
}



char *TCPReceiveData(SOCKET connectSocket) {
    char recvbuf[BUFFER];
    int iResult = recv(connectSocket, (char*)recvbuf, sizeof(recvbuf), 0);
    if (iResult > 0)
    {
        //printf("Msg value: %d\n", recvbuf->value);
    }
    else
    {
        // there was an error during recv
        printf("recv failed with error: %d\n", WSAGetLastError());
    }
    return recvbuf;
}