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


bool TCPSend(SOCKET connectSocket, Measurment measurment) {
    char* data = (char*)malloc(sizeof(Measurment));
    memcpy(data, (const void *)&measurment, sizeof(Measurment));
    int iResult = send(connectSocket, data, sizeof(Measurment), 0);
    free(data); //zasto puca?
    //free(&measurment);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        //closesocket(connectSocket);
        //WSACleanup();
        return false;
    }
    return true;
}


bool TCPSend(SOCKET connectSocket, char key) {
    int iResult = send(connectSocket, (const char*)key, sizeof(char), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        //closesocket(connectSocket);
        //WSACleanup();
        return false;
    }
    return true;
}



bool TCPReceive(SOCKET connectSocket, char* recvbuf, size_t len) {
    //Measurment* recvbuf = (Measurment*)malloc(sizeof(Measurment));
    //char* recvbuf = (char*)malloc(sizeof(len));
    //Measurment* data = (Measurment*)malloc(sizeof(Measurment));

    int iResult = recv(connectSocket, (char*)recvbuf, len, 0);
    if (iResult > 0)
    {
        //printf("Msg value: %d\n", recvbuf->value);
        
        //memcpy(data, recvbuf, sizeof(Measurment));
    }
    else
    {
        // there was an error during recv
        printf("recv failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}