#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//#include "../Common/LinkedList.h";
#include "../Common/GenericList.h";
#include "../Common/Measurment.h";
#include "../TCPLib/TCPLib.cpp";

#pragma comment(lib,"WS2_32")

#define DEFAULT_BUFLEN 1000
#define DEFAULT_PORT "27016"
#define MAX_CLIENTS 10
#define TIMEVAL_SEC 0
#define TIMEVAL_USEC 0


bool InitializeWindowsSockets();
bool InitializeListenSocket();
bool BindListenSocket();
void SetNonBlocking();
int Init();
void Listen();
void SetAcceptedSocketsInvalid();
void ProcessMessages();
void ProcessMeasurment(Measurment*);

fd_set readfds;
SOCKET listenSocket = INVALID_SOCKET;
SOCKET acceptedSockets[MAX_CLIENTS];
addrinfo* resultingAddress = NULL;
timeval timeVal;

NODE *publisherList = NULL;
NODE *subscriberList = NULL;
NODE *statusData = NULL;
NODE *analogData = NULL;

int main()
{

    int result = Init();
    if (result) {
        printf("ERROR CODE %d, press any key to exit\n", result);
        getchar();
        return result;
    }

    printf("Server live and ready to listen\n");
    
    Listen();
    getchar();

}

/*
* Initialises the service and setups listen and accepted sockets.
*/
int Init() {
    //init_list(&listHead);

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }


    if (InitializeListenSocket() == false) {
        return 2;
    }

    if (BindListenSocket() == false) {
        return 3;
    }

    freeaddrinfo(resultingAddress);

    SetNonBlocking();

    SetAcceptedSocketsInvalid();

    return 0;
}


/*
* After the serice is initialised, enter the listening state.
*/
void Listen() {


    int iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }
    printf("Listening...\n");


    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(listenSocket, &readfds);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (acceptedSockets[i] != INVALID_SOCKET)
                FD_SET(acceptedSockets[i], &readfds);
        }

        int value = select(0, &readfds, NULL, NULL, &timeVal);

        if (value == 0) {
            //pass...
        }
        else if (value == SOCKET_ERROR) {
            //Greska prilikom poziva funkcije, odbaci sokete sa greskom
            printf("select failed with error: %d\n", WSAGetLastError());
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (FD_ISSET(acceptedSockets[i], &readfds)) {
                    closesocket(acceptedSockets[i]);
                    acceptedSockets[i] = INVALID_SOCKET;
                }
                else if (i == MAX_CLIENTS) {
                    closesocket(listenSocket);
                    WSACleanup();
                    return;
                }
            }
        }
        else { //accept event
            if (FD_ISSET(listenSocket, &readfds)) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (acceptedSockets[i] == INVALID_SOCKET) {
                        acceptedSockets[i] = accept(listenSocket, NULL, NULL);
                        if (acceptedSockets[i] == INVALID_SOCKET)
                        {
                            printf("accept failed with error: %d\n", WSAGetLastError());
                            closesocket(listenSocket);
                            WSACleanup();
                            return;
                        }

                        break;
                    }
                }
                //ako mu ne nadje mesto uradi nesto

            }
            else { //servis prima poruku
                ProcessMessages();
            }

        }
    }

}


/*
* Set up the socket to accept non-blocking TCP mode.
*/
void SetNonBlocking() {
    unsigned long mode = 1;

    int iResult = ioctlsocket(listenSocket, FIONBIO, &mode);

    FD_ZERO(&readfds);

    FD_SET(listenSocket, &readfds);

    timeVal.tv_sec = TIMEVAL_SEC;
    timeVal.tv_usec = TIMEVAL_USEC;
}

/*
* WSA initialiser.
*/
bool InitializeWindowsSockets()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

/*
* Sets up listening socket.
*/
bool InitializeListenSocket() {
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return false;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return false;
    }

    return true;
}

/*
* Binds listening socket with the configured address and port.
*/
bool BindListenSocket() {
    int iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }
    return true;
}

/*
* Sets up accepted sockets to default (empty) value.
*/
void SetAcceptedSocketsInvalid() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        acceptedSockets[i] = INVALID_SOCKET;
    }
}


/*
* Checks accepted sockets that raised an event and 
* calls TCPLib.TCPReceiveMeasurment to processes the message. 
* Fills publishers and subrscribers list for introducment messages and calls
* ProccessMeasurment for processing data.
*/
void ProcessMessages() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (FD_ISSET(acceptedSockets[i], &readfds)) {
            char* data = (char*)malloc(sizeof(Measurment));
            TCPReceive(acceptedSockets[i], data, sizeof(Measurment));

            //proveri da li je poruka predstavljanja
            if (data[0] == 'p') {
                GenericListPushAtStart(&publisherList, &acceptedSockets[i], sizeof(SOCKET));
                printf("Connected client: publisher\n");
                return;
            }
            else if (data[0] == 'd') {
                GenericListPushAtStart(&subscriberList, &acceptedSockets[i], sizeof(SOCKET));
                printf("Connected client: subscriber\n");
                return;
            }
            else if (data[0] == 'a') {
                //TODO subscriber subscribed to analog topic
            }
            else if (data[0] == 's') {
                //TODO subscriber subscribed to status topic
            }
            else {
                //else message is Measurment data
                Measurment* newMeasurment = (Measurment*)malloc(sizeof(Measurment));
                memcpy(newMeasurment, data, sizeof(Measurment));
                //data treba free?
                free(data); //zasto ovo puca?
                ProcessMeasurment(newMeasurment);
            }
        }
    }
}

/*
* Save the measurment in the list corresponding to its type.
* m = Measurment to be saved (by reference)
*/
void ProcessMeasurment(Measurment *m) {
    printf("[DEBUG] Service received: ");
    PrintMeasurment(m);

    switch (m->topic)
    {
    case Analog:
        GenericListPushAtStart(&analogData, m, sizeof(Measurment));
        break;
    case Status:
        GenericListPushAtStart(&statusData, m, sizeof(Measurment));
        break;
    default:
        printf("[ERROR] Topic %d not supported.", m->topic);
        break;
    }
}