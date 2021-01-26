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

#include "../Common/Measurment.h";
#include "../TCPLib/TCPLib.h";

#pragma comment(lib,"WS2_32")

/// <summary>
/// Sends a measurment packet trough the socket using TCP protocol.
/// </summary>
/// <param name="connectSocket">Socket trought which the data will be sent.</param>
/// <param name="measurment">Measurment packet that will be sent.</param>
/// <returns>True if sending was successful.</returns>
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

/// <summary>
/// Used for sending keys of introduction or any kind of string data trough the socket using TCP protocol.
/// </summary>
/// <param name="connectSocket">Socket trought which the data will be sent.</param>
/// <param name="key">String that will be sent.</param>
/// <returns>True if sending was successful.</returns>
bool TCPSend(SOCKET connectSocket, char *key) {
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


/// <summary>
/// Function for receiving packets from a specified socket.
/// </summary>
/// <param name="connectSocket">Socket trough which the packets will be sent.</param>
/// <param name="recvbuf">Buffer that will store recieved data.</param>
/// <param name="len">Lenght of data to be recieved.</param>
/// <returns>True if recieving is successful.</returns>
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
        //printf("recv failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}