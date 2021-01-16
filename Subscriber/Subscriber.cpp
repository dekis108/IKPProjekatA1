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
void Subscribe();
//DWORD WINAPI Receive();
void StartRecieveThread();

SOCKET connectSocket = INVALID_SOCKET;
sockaddr_in serverAddress;

//primi i ispisi
DWORD WINAPI Receive(LPVOID param) {
    //printf("Subscriber starting to receive:\n\n");
    char* data = (char*)malloc(sizeof(Measurment));
    while (true) {
        TCPReceive(connectSocket, data, sizeof(Measurment));
        Measurment* newMeasurment = (Measurment*)malloc(sizeof(Measurment));
        memcpy(newMeasurment, data, sizeof(Measurment));
        PrintMeasurment(newMeasurment);
        free(newMeasurment);
        Sleep(10);
    }
}


int main()
{
    int result = Init();
    if (result) {
        printf("ERROR CODE %d, press any key to exit\n", result);
        getchar();
        return result;
    }

    printf("Client initialised.\n");

    Subscribe();

   // Receive();
    //Thread za Recieve
    
    StartRecieveThread();
    getchar();

}

void StartRecieveThread() {
    DWORD id, param = 1;
    HANDLE handle;
    handle = CreateThread(
        NULL, // default security attributes
        0, // use default stack size
        Receive, // thread function
        &param, // argument to thread function
        0, // use default creation flags
        &id); // returns the thread identifier
    int liI = _getch();
    CloseHandle(handle);
}


void Subscribe() {
    printf("1) Status\n2) Analog\n3) Both\nSelect: ");
    char c = getchar();
    char t1[2] = "s";
    char t2[2] = "a";
    switch (c)
    {
    case '1':
        TCPSend(connectSocket, t1);
        break;
    case '2':
        TCPSend(connectSocket, t2);
        break;
    case '3':
        TCPSend(connectSocket, t1);
        TCPSend(connectSocket, t2);
        break;
    default:
        printf("\nBad input.\n");
        Subscribe();
    }
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
