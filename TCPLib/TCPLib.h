#ifndef TCP_HEADER
#define TCP_HEADER "TCPLib.h"

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "../Common/Measurment.h";

bool TCPSend(SOCKET connectSocket, Measurment measurment);

bool TCPSend(SOCKET connectSocket, char* key);

#endif