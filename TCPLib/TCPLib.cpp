// TCPLib.cpp : Defines the functions for TCP communication.
//
#include "pch.h"
#include "framework.h"

#include "TCPLib.h"
#include "../Common/Measurment.h"

/*
* Uses wsa send(...) to transmit data of type Measurment to the target Socket.
* connectSocket = target socket
* measurment = data to be sent
*/
bool TCPSend(SOCKET connectSocket, Measurment measurment) {
    while (true) {
        char* data = (char*)malloc(sizeof(Measurment));
        memcpy(data, (const void*)&measurment, sizeof(Measurment));
        int iResult = send(connectSocket, data, sizeof(Measurment), 0);
        free(data); 
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            continue;
        }
        else if (iResult == SOCKET_ERROR)
        {
            return false;
        }
        else {
            return true;
        }

    }

}

/*
* Uses wsa send(...) to transmit a string of data to the target socket.
* connectSocket = target socket
* key = data to be sent
*/
bool TCPSend(SOCKET connectSocket, char *key) {
    while (true) {
        int iResult = send(connectSocket, (const char*)key, sizeof(char), 0);
        if (WSAGetLastError() == WSAEWOULDBLOCK) {

            continue;
        }
        else if (iResult == SOCKET_ERROR)
        {
            return false;
        }
        else {
            return true;
        }
    }

}



/*
* Uses wsa recv to receive data from the target socket.
* connectSocket = target socket (sender)
* *recvbuf = data pointer where the data will be writen
* len = size of the data that will be read
*/
int TCPReceive(SOCKET connectSocket, char* recvbuf, size_t len) {
    int iResult = recv(connectSocket, (char*)recvbuf, len, 0);
    if (iResult > 0)
    {
        return 1;
    }
    else if (WSAGetLastError() == WSAEWOULDBLOCK) {
        //printf("[DEBUG] TCPReceive WSAEWOULDBLOCK\n");
        return 2;
    }
    else if (iResult == SOCKET_ERROR)
    {
        return 0;
    }
    else {
        return 0;
    }
   
}
