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
bool Publish();
bool IntroduceMyself();

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

    printf("Client initialised. Press enter publish\n");
    getchar();
    printf("Sending...\n");

    if (Publish()) {
        printf("Done, stopping..\n");
    }
    else {
        printf("An error occured\n");
    }

    getchar();

}

bool  Publish() {
    Measurment* msg = (Measurment*)malloc(sizeof(Measurment));

    //TODO: generisanje vrednosti
    msg->value = 13;
    strcpy(msg->topic,"test1");
    strcpy(msg->type,"test2");

    int iResult = send(connectSocket, (const char*)msg, sizeof(Measurment), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return false;
    }
    //Sleep(10);
    return true;
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
    //publisher se predstavi servisu
    char introduction[11] = "publisher_";
    int iResult = send(connectSocket, (const char*)introduction, 11, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return false;
    }
    return true;
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
