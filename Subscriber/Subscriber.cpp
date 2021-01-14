#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "../Common/Measurment.h";
#include "../TCPLib/TCPLib.cpp";

#pragma comment(lib,"WS2_32")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016
#define ADDRESS "127.0.0.1"
#define TYPE_STRING_LENGHT 10
#define DEMOTESTCOUNT 20

int Init();
bool InitializeWindowsSockets();
bool CreateSocket();
bool Connect();
bool IntroduceMyself();
void Subscribe(MeasurmentTopic);
void Recieve();
void SendTopic();

SOCKET connectSocket = INVALID_SOCKET;
sockaddr_in serverAddress;


int main()
{
    int result = Init();
    if (result) {
        printf("ERROR CODE %d, press any key to exit\n", result);
        getchar();
        return result;
    }

    printf("Client initialised.\n");

    SendTopic();

    Recieve();

    getchar();

}

void SendTopic() {
    printf("Enter the topic you want to subscribe to (analog,status): \n");
    char topic[7];
    scanf("%s", topic);
    if (strcmp(topic, "status") == 0) {
        TCPSend(connectSocket, (char*)"s");
    }
    else if (strcmp(topic, "analog") == 0) {
        TCPSend(connectSocket, (char*)"a");
    }
    else {
        printf("Please, enter a valid topic");
    }
}

//primi i ispisi
void Recieve() {
    char* measurment_bytes = TCPReceive(connectSocket, sizeof(Measurment));
    Measurment* m = (Measurment*)malloc(sizeof(Measurment));
    memcpy(m, measurment_bytes, sizeof(Measurment));
    PrintMeasurment(&m);
}

void Subscribe(MeasurmentTopic topic) {
    //TODO
}

int Init() {

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }
    if (CreateSocket() == false) {
        return 2;
    }
    if (Connect() == false) {
        return 3;
    }
    if (IntroduceMyself() == false) {
        return 4;
    }
    return 0;
}

bool IntroduceMyself() {
    //subs se predstavi servisu
    char introducment[2] = "d";
    return TCPSend(connectSocket, introducment);
}

bool Connect() {
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ADDRESS);
    serverAddress.sin_port = htons(DEFAULT_PORT);

    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return false;
    }
    return true;
}

bool CreateSocket() {
    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return false;
    }
    return true;
}

bool InitializeWindowsSockets() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}
