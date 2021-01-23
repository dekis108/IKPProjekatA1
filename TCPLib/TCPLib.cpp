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


#define WOULDBLOCKWAIT 500

#pragma comment(lib,"WS2_32")


bool TCPSend(SOCKET connectSocket, Measurment measurment) {
    while (true) {
        char* data = (char*)malloc(sizeof(Measurment));
        memcpy(data, (const void*)&measurment, sizeof(Measurment));
        int iResult = send(connectSocket, data, sizeof(Measurment), 0);
        free(data); //zasto puca?
        //free(&measurment);
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            printf("[DEBUG] TCPSendMeasurment  WSAEWOULDBLOCK\n");
            //Sleep(WOULDBLOCKWAIT);
            continue;
        }
        else if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            //closesocket(connectSocket);
            //WSACleanup();
            return false;
        }
        else {
            return true;
        }

    }

}


bool TCPSend(SOCKET connectSocket, char *key) {
    while (true) {
        int iResult = send(connectSocket, (const char*)key, sizeof(char), 0);
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            printf("[DEBUG] TCPSendChar WSAEWOULDBLOCK\n");
            //Sleep(WOULDBLOCKWAIT);
            continue;
        }
        else if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            //closesocket(connectSocket);
            //WSACleanup();
            return false;
        }
        else {
            return true;
        }
    }

}



int TCPReceive(SOCKET connectSocket, char* recvbuf, size_t len) {
    //Measurment* recvbuf = (Measurment*)malloc(sizeof(Measurment));
    //char* recvbuf = (char*)malloc(sizeof(len));
    //Measurment* data = (Measurment*)malloc(sizeof(Measurment));

    int iResult = recv(connectSocket, (char*)recvbuf, len, 0);
    if (iResult > 0)
    {
        //printf("Msg value: %d\n", recvbuf->value);

        //memcpy(data, recvbuf, sizeof(Measurment));
        return 1;
    }
    else if (WSAGetLastError() == WSAEWOULDBLOCK) {
        //printf("[DEBUG] TCPReceive WSAEWOULDBLOCK\n");
        //Sleep(WOULDBLOCKWAIT);
        return 2;
    }
    else if (iResult == SOCKET_ERROR)
    {
        // there was an error during recv
        //printf("recv failed with error: %d\n", WSAGetLastError());
        return 0;
    }
   
}