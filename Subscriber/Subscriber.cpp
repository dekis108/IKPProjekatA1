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
bool Validate(Measurment *);

SOCKET connectSocket = INVALID_SOCKET;
sockaddr_in serverAddress;

/// <summary>
/// Recieves Measurment structure from the PubSubEngine. Uses a function from TCPLib to recieve a packet.
/// </summary>
/// <param name="param">Needed to construct a thread.</param>
/// <returns>Will never return since there is an infinite loop.</returns>
DWORD WINAPI Receive(LPVOID param) {
    char* data = (char*)malloc(sizeof(Measurment));
    while (true) {
        TCPReceive(connectSocket, data, sizeof(Measurment));
        Measurment* newMeasurment = (Measurment*)malloc(sizeof(Measurment));
        memcpy(newMeasurment, data, sizeof(Measurment));
        if (Validate(newMeasurment)) {
            printf("VALID: ");
        }
        else {
            printf("INVALID: ");
        }
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

    printf("Client subscribed.\n");
    
    StartRecieveThread();

    getchar();

}

bool Validate(Measurment *m) {
    if (m->topic == Analog && m->value >= 0) {
        return true;
    }
    else if (m->topic == Status && (m->value == 0 || m->value == 1)) {
        return true;
    }
    return false;
}


/// <summary>
/// Creates and starts the thread for the Receive function.
/// </summary>
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

/// <summary>
/// Gives user the menu for picking subscription topics. Based on users input, uses a sending function from TCPLib to 
/// tell PubSubEngine what topic client has subscribed to.
/// </summary>
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
        Sleep(100);
        TCPSend(connectSocket, t2);
        break;
    default:
        printf("\nBad input.\n");
        Subscribe();
    }
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
    //subs se predstavi servisu
    char introducment[2] = "d";
    return TCPSend(connectSocket, introducment);
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
