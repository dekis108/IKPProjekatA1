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
#include <time.h>

#include "../Common/Measurment.h";
#include "../Common/Measurment.cpp";
#include "../TCPLib/TCPLib.h";
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
Measurment *GenerateMeasurment();
Measurment *CreateMeasurment();
bool IntroduceMyself();
void SendMeasurment(int publishingType, int interval);

SOCKET connectSocket = INVALID_SOCKET;
sockaddr_in serverAddress;


int main()
{
    srand(time(NULL));
    int result = Init();
    if (result) {
        printf("ERROR CODE %d, press any key to exit\n", result);
        getchar();
        return result;
    }
    printf("Client initialised.");
    int picked = 0;
    int interval = 1000; //default interval
    while (true)
    {
        printf(" Pick a way of publishing:\n1)Randomly generated message\n2)User created message\n");
        scanf("%d", &picked);
        if (picked != 1 && picked != 2) {
            printf("Please pick a valid publishing type.");
        }
        else if (picked == 1) {
            printf("Please pick an interval of publishing in ms: \n");
            scanf("%d", &interval);
            break;
        }
        else {
            break;
        }
    }

    getchar();
    
    SendMeasurment(picked, interval);
    
    getchar();

}

/// <summary>
/// Function that sends randomly generated Measurment every 1001 ms by using a sending function from TCPLib.
/// </summary>
void SendMeasurment(int publishingType, int interval) {
    while (true) {
        printf("Sending...\n");
        Measurment* m = (Measurment *)malloc(sizeof(Measurment));
        switch (publishingType)
        {
        case 1:
             m = GenerateMeasurment();
             break;
        case 2:
             m = CreateMeasurment();
             break;
        default:
            break;
        }
        //Measurment* m = GenerateMeasurment();
        //Measurment* m = CreateMeasurment();
        if (TCPSend(connectSocket, *m)) {
            printf("Sent: %s %s %d \n", GetStringFromEnumHelper(m->topic), GetStringFromEnumHelper(m->type), m->value);
        }
        else {
            printf("An error occured\n");
        }
        free(m);
        Sleep(interval);
    }
}

/// <summary>
/// Generates a randomly valued Measurment structure.
/// </summary>
/// <returns>Returns a pointer to the generated Measurment.</returns>
Measurment * GenerateMeasurment() {
    Measurment* msg = (Measurment*)malloc(sizeof(Measurment));
    enum MeasurmentTopic a = Analog;
    msg->value = (rand() % 100) + 1; 
    int topic = (rand() % 2);
    int type = (rand() % 2);
    switch (topic) {
        case 0: 
            msg->topic = Analog;
            msg->value = (rand() % 100);
            break;
    
        case 1: 
            msg->topic = Status;
            msg->type = MER;
            msg->value = (rand() % 5);
            break;
    }
    if (msg->topic == Analog) {
        switch (type)
        {
        case 0:
            msg->type = SWG;
            break;
        case 1:
            msg->type = CRB;
            break;
        }
    }
    return msg;
}

/// <summary>
/// Creates a Measurment structure by users input.
/// </summary>
/// <returns>Returns a Measurment.</returns>
Measurment *CreateMeasurment() {
    Measurment* msg = (Measurment*)malloc(sizeof(Measurment));
    
    printf("Enter topic\n");
    char topic[20];
    scanf("%s", topic);
    printf("Enter type\n");
    char type[20];
    scanf("%s", type);
    printf("Enter value\n");
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

    return msg;
}

/// <summary>
/// Initialises the client with functions InitializeWindowsSockets(), CreateSocket() and Connect(). 
/// Calls IntroduceMyself() to tell PubSubEngine what type of a client it is.
/// </summary>
/// <returns>Returns error number.</returns>
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

/// <summary>
/// Function that uses a sending function from TCPLib to introduce himself as a Subscriber.
/// </summary>
/// <returns>Returns true if send function is successfull, otherwise false.</returns>
bool IntroduceMyself() {
    //publisher se predstavi servisu
    char introducment[2] = "p";
    return TCPSend(connectSocket,introducment);
}

/// <summary>
/// Connects the client socket to the server.
/// </summary>
/// <returns>True if connect function was successful.</returns>
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

/// <summary>
/// Creates and inits the clients socket.
/// </summary>
/// <returns>True if socket creation was successful.</returns>
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

/// <summary>
/// Initiates use of the Winsock DLL by a process.
/// </summary>
/// <returns>Returns true if init was successful.</returns>
bool InitializeWindowsSockets() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}
