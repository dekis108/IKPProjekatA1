// TCPLib.cpp : Defines the functions for TCP communication.
//

#ifndef TCPLIB
#define TCPLIB

#pragma once

#include "pch.h"
#include "framework.h"


#include "../Common/Measurment.h"

/*
* Uses wsa send(...) to transmit data of type Measurment to the target Socket.
* connectSocket = target socket
* measurment = data to be sent
*/
bool TCPSend(SOCKET connectSocket, Measurment measurment);

/*
* Uses wsa send(...) to transmit a string of data to the target socket.
* connectSocket = target socket
* key = data to be sent
*/
bool TCPSend(SOCKET connectSocket, char* key);



/*
* Uses wsa recv to receive data from the target socket.
* connectSocket = target socket (sender)
* *recvbuf = data pointer where the data will be writen
* len = size of the data that will be read
*/
int TCPReceive(SOCKET connectSocket, char* recvbuf, size_t len);


#endif // !TCPLIB
