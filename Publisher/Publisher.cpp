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
Measurment GenerateMeasurment();
Measurment CreateMeasurment();
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

    if (TCPSendMeasurment(connectSocket,GenerateMeasurment())) {
    //if (TCPSendMeasurment(connectSocket,CreateMeasurment())) {
        printf("Done, stopping..\n");
    }
    else {
        printf("An error occured\n");
    }

    getchar();

}


Measurment GenerateMeasurment() {
    Measurment* msg = (Measurment*)malloc(sizeof(Measurment));
    enum MeasurmentTopic a = Analog;
    srand(time(NULL));
    msg->value = (rand() % 100) + 1; 
    int topic = (rand() % 2);
    int type = (rand() % 3);
    switch (topic) {
        case 0: 
            msg->topic = Analog;
            break;
    
        case 1: 
            msg->topic = Status;
            break;
    }
    switch (type) {
        case 0:
            msg->type = SWG;
            break;

        case 1:
            msg->type = CRB;
            break;

        case 2:
            msg->type = MER;
            break;
    }

    return *msg;
}

Measurment CreateMeasurment() {
    Measurment* msg = (Measurment*)malloc(sizeof(Measurment));
    /*
    
    printf("Enter topic");
    char topic[20];
    scanf("%s", topic);
    printf("Enter type");
    char type[20];
    scanf("%s", type);
    printf("Enter value");
    int val = 0;
    scanf("%d", &val);

    if (strcmp(topic, "Analog") == 0) {
        msg->topic = Analog;
    }
    else if (strcmp(topic, "Status") == 0) {
        msg->topic = Status;
    }

    if (strcmp(type, "SWG") == 0) {
        msg->type = SWG;
    }
    else if (strcmp(type, "CRB") == 0) {
        msg->type = CRB;
    }
    else if (strcmp(type, "MER") == 0) {
        msg->type = MER;
    }

    msg->value = val;
    */

    msg->topic = Analog;
    msg->type = CRB;
    msg->value = 15;

    return *msg;
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
