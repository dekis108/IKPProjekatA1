#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "WorkerStruct.h"

//#include "../Common/LinkedList.h";
//#include "../Common/GenericList.h";
#include "../Common/ThreadSafeGenericList.h";
#include "../Common/Measurment.h";
#include "../TCPLib/TCPLib.cpp";

#pragma comment(lib,"WS2_32")

#define DEFAULT_BUFLEN 1000
#define DEFAULT_PORT "27016"
#define MAX_CLIENTS 10
#define MAX_THREADS 16
#define TIMEVAL_SEC 0
#define TIMEVAL_USEC 0

#define SAFE_DELETE_HANDLE(a)  if(a){CloseHandle(a);}

bool InitializeWindowsSockets();
bool InitializeListenSocket();
bool BindListenSocket();
void SetNonBlocking();
int Init();
DWORD WINAPI Listen(LPVOID);
void SetAcceptedSocketsInvalid();
void ProcessMessages();
void ProcessMeasurment(Measurment*);
void Shutdown();
void UpdateSubscribers(Measurment*, NODE *);
void SendToNewSubscriber(SOCKET, NODE*);
bool InitCriticalSections();
void CallWorkerTask(int, char*);
DWORD WINAPI Work(LPVOID);

fd_set readfds;
SOCKET listenSocket = INVALID_SOCKET;
SOCKET acceptedSockets[MAX_CLIENTS];
addrinfo* resultingAddress = NULL;
timeval timeVal;

CRITICAL_SECTION CSAnalogData;
CRITICAL_SECTION CSStatusData;
CRITICAL_SECTION CSAnalogSubs;
CRITICAL_SECTION CSStatusSubs;


NODE *publisherList = NULL;
NODE *subscriberList = NULL;
NODE *statusData = NULL;
NODE *analogData = NULL;
NODE *statusSubscribers = NULL;
NODE *analogSubscribers = NULL;


HANDLE listenHandle;
HANDLE workerHandles[MAX_THREADS];

int main()
{
 
    DWORD listenID;

    int result = Init();
    if (result) {
        printf("ERROR CODE %d, press any key to exit\n", result);
        getchar();
        return result;
    }

    printf("Server live and ready to listen\n");
    
    listenHandle = CreateThread(NULL, 0, &Listen, (LPVOID)0, 0, &listenID);
    //Listen();
    if (listenHandle) {
        WaitForSingleObject(listenHandle, INFINITE);
    }


    getchar();
    Shutdown();
}

void CallWorkerTask(int i, char *data) {

    while (true) {
        for (int k = 0; k < MAX_THREADS; ++k) {
            if (workerHandles[k] != 0) {
                continue;
            }

            WorkerData* wData = (WorkerData *)malloc(sizeof(WorkerData));
            wData->i = i;
            wData->data = data;

            workerHandles[i] = CreateThread(NULL, 0, &Work, wData, 0, NULL);
            if (workerHandles[i] == 0) {
                printf("Erorr while creating a worker thread. Shutting down..");
                Shutdown();
            }
            return;
        }
        printf("No avaliable thread for message proccessing, consider increasing thread count.Waiting..\n");
        Sleep(1000);
    }
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

    if (InitCriticalSections() == false) {
        return 4;
    }

    freeaddrinfo(resultingAddress);

    SetNonBlocking();

    SetAcceptedSocketsInvalid();

    return 0;
}

/*
* Calls InitializeCriticalSection(...) for every CRITICAL_SECTION handle.
*/
bool InitCriticalSections() {
    InitializeCriticalSection(&CSAnalogData);
    InitializeCriticalSection(&CSStatusData);
    InitializeCriticalSection(&CSAnalogSubs);
    InitializeCriticalSection(&CSStatusSubs);

    return true;
}

/*
* After the serice is initialised, enter the listening state.
*/
DWORD WINAPI Listen(LPVOID param) {


    int iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        Shutdown();
        return 0;
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
            //pass
        }
        else if (value == SOCKET_ERROR) {
            //Greska prilikom poziva funkcije, odbaci sokete sa greskom
            printf("select failed with error: %d\n", WSAGetLastError());
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (FD_ISSET(acceptedSockets[i], &readfds)) {
                    closesocket(acceptedSockets[i]);
                    acceptedSockets[i] = INVALID_SOCKET;
                }
            }
        }
        else { //accept event
            if (FD_ISSET(listenSocket, &readfds)) {
                int  i;
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (acceptedSockets[i] == INVALID_SOCKET) {
                        acceptedSockets[i] = accept(listenSocket, NULL, NULL);
                        if (acceptedSockets[i] == INVALID_SOCKET)
                        {
                            printf("accept failed with error: %d\n", WSAGetLastError());
                            closesocket(acceptedSockets[i]);
                            acceptedSockets[i] = INVALID_SOCKET;
                            //closesocket(listenSocket);
                            //WSACleanup();
                            return 0;
                        }

                        break;
                    }
                }
                //ako mu ne nadje mesto uradi nesto
                if (i == MAX_CLIENTS) {
                   // printf("Nema mesta za jos jednu konekciju.\n");
                }
                
            }
            //else { //servis prima poruku
            ProcessMessages();
            //}

        }
    }

}

DWORD WINAPI Work(LPVOID lparams) {
    WorkerData *wData = (WorkerData*)lparams;

    int i = wData->i;
    char *data = wData->data;

    bool succes = TCPReceive(acceptedSockets[i], data, sizeof(Measurment));

    if (!succes) { //always close this socket?
        EnterCriticalSection(&CSAnalogSubs);
        DeleteNode(&analogSubscribers, &acceptedSockets[i], sizeof(SOCKET));
        LeaveCriticalSection(&CSAnalogSubs);

        EnterCriticalSection(&CSStatusSubs);
        DeleteNode(&statusSubscribers, &acceptedSockets[i], sizeof(SOCKET));
        LeaveCriticalSection(&CSStatusSubs);

        closesocket(acceptedSockets[i]);
        acceptedSockets[i] = INVALID_SOCKET;
        //continue;
        return 0;
    }

    SOCKET* ptr = &acceptedSockets[i];
    //proveri da li je poruka predstavljanja
    if (data[0] == 'p') {
        //GenericListPushAtStart(&publisherList, ptr, sizeof(SOCKET));
        printf("Connected client: publisher\n");
    }
    else if (data[0] == 'd') {
        //GenericListPushAtStart(&subscriberList, ptr, sizeof(SOCKET));
        printf("Connected client: subscriber\n");
    }
    else if (data[0] == 'a') {
        EnterCriticalSection(&CSAnalogSubs);
        GenericListPushAtStart(&analogSubscribers, ptr, sizeof(SOCKET));
        SendToNewSubscriber(acceptedSockets[i], analogData);
        LeaveCriticalSection(&CSAnalogSubs);
    }
    else if (data[0] == 's') {
        EnterCriticalSection(&CSStatusSubs);
        GenericListPushAtStart(&statusSubscribers, ptr, sizeof(SOCKET));
        SendToNewSubscriber(acceptedSockets[i], statusData);
        LeaveCriticalSection(&CSStatusSubs);
    }
    else {
        //else message is Measurment data
        Measurment* newMeasurment = (Measurment*)malloc(sizeof(Measurment));
        memcpy(newMeasurment, data, sizeof(Measurment));
        //free(data);
        ProcessMeasurment(newMeasurment);
        free(newMeasurment);
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
    char* data = (char*)malloc(sizeof(Measurment));
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (FD_ISSET(acceptedSockets[i], &readfds)) {
           
            //TODO sad napravi thread da ovo obradi
            CallWorkerTask(i, data);


        }
    }
    free(data);
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
        EnterCriticalSection(&CSAnalogData);
        GenericListPushAtStart(&analogData, m, sizeof(Measurment));
        UpdateSubscribers(m, analogSubscribers);
        LeaveCriticalSection(&CSAnalogData);
        break;
    case Status:
        EnterCriticalSection(&CSStatusData);
        GenericListPushAtStart(&statusData, m, sizeof(Measurment));
        UpdateSubscribers(m, statusSubscribers);
        LeaveCriticalSection(&CSStatusData);
        break;
    default:
        printf("[ERROR] Topic %d not supported.", m->topic);
        break;
    }
}

/*
* Frees program memory, handles and performs WSA shutdown logic.
*/
void Shutdown() {
    closesocket(listenSocket);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        closesocket(acceptedSockets[i]);
    }
    WSACleanup();

    FreeGenericList(&publisherList);
    FreeGenericList(&subscriberList);
    FreeGenericList(&statusData);
    FreeGenericList(&analogData);
    FreeGenericList(&statusSubscribers);
    FreeGenericList(&analogSubscribers);

    SAFE_DELETE_HANDLE(listenHandle);
    for (int i = 0; i < MAX_THREADS; ++i) {
        SAFE_DELETE_HANDLE(workerHandles[i]);
    }
    
    DeleteCriticalSection(&CSStatusData);
    DeleteCriticalSection(&CSAnalogData);
    DeleteCriticalSection(&CSStatusSubs);
    DeleteCriticalSection(&CSAnalogSubs);



    printf("Service freed all memory.");
    getchar();
}


/*
* After a new measurment arrives, send it to all the subscribers that all subscribed to that topic.
* *m = Measurment to be sent
* *list = head of the list of subscribers that should receive the measurment
*/
void UpdateSubscribers(Measurment* m, Node *list) {
    Node* temp = list;
    if (temp == NULL) {
        return;
    }
    while (temp != NULL) {
        SOCKET s;
        memcpy(&s, temp->data, sizeof(SOCKET));
        TCPSend(s, *m);
        temp = temp->next;
    }
}


/*
* After a Subscriber Subscribes to a new topic, send all saved data of the said topic to him.
* sub = SOCKET of the subscriber
* *dataHead = head of the list with the correct topic data
*/
void SendToNewSubscriber(SOCKET sub, NODE *dataHead) {
    Node* temp = dataHead;
    if (temp == NULL) {
        return;
    }
    Measurment* data = (Measurment*)malloc(sizeof(Measurment));
    while (temp != NULL) {
        memcpy(data, temp->data, sizeof(Measurment));
        TCPSend(sub, *data);
        temp = temp->next;
    }
    free(data);
}
