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

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "WorkerStruct.h"


#include "../Common/GenericList.h";
#include "../Common/Measurment.h";
#include "../TCPLib/TCPLib.h";

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma comment(lib,"WS2_32")

#define DEFAULT_BUFLEN 1000
#define DEFAULT_PORT "27016"
#define MAX_CLIENTS 100
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
DWORD WINAPI DoWork(LPVOID);
DWORD WINAPI InitWorkerThreads(LPVOID);
bool Work(int);

fd_set readfds;
SOCKET listenSocket = INVALID_SOCKET;
SOCKET acceptedSockets[MAX_CLIENTS];
addrinfo* resultingAddress = NULL;
timeval timeVal;

CRITICAL_SECTION CSAnalogData;
CRITICAL_SECTION CSStatusData;
CRITICAL_SECTION CSAnalogSubs;
CRITICAL_SECTION CSStatusSubs;
CRITICAL_SECTION CSWorkerTasks;


NODE *publisherList = NULL;
NODE *subscriberList = NULL;
NODE *statusData = NULL;
NODE *analogData = NULL;
NODE *statusSubscribers = NULL;
NODE *analogSubscribers = NULL;
NODE *workerTasks = NULL;


HANDLE listenHandle;
HANDLE workerManagerHandle;
HANDLE workerHandles[MAX_THREADS];

int main()
{
 
    DWORD listenID;
    DWORD workerID;

    int result = Init();
    if (result) {
        printf("ERROR CODE %d, press any key to exit\n", result);
        getchar();
        return result;
    }

    printf("Server live and ready to listen\n");
    
    listenHandle        = CreateThread(NULL, 0, &Listen, (LPVOID)0, 0, &listenID);
    workerManagerHandle = CreateThread(NULL, 0, &InitWorkerThreads, (LPVOID)0, 0, &workerID);

    printf("Service running, press 'q' for shutdown\n");
    while (true) {
        char key = getchar();
        if (key == 'q') {
            Shutdown();
            break;
        }
    }
    return 0;
}


/*
* Initialises the service and setups listen and accepted sockets.
*/
int Init() {
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
* Initializes MAX_THREADS instances of worker threads. 
* No argument needed.
*/
DWORD WINAPI InitWorkerThreads(LPVOID params) {
    for (int i = 0; i < MAX_THREADS; ++i) {
        workerHandles[i] = CreateThread(NULL, 0, &DoWork, (LPVOID)0, 0, NULL);
        if (workerHandles[i] == 0) {
            printf("WorkerManager failed at creating a worker thread.");
            return false;
        }
    }

    if (WaitForMultipleObjects(MAX_THREADS, workerHandles, TRUE, INFINITE) != WAIT_OBJECT_0 + MAX_THREADS - 1) { //TODO make sure math is correct 
        //all worker threads succesfully finished, close them
        printf("[DEBUG] Workers finished\n");
    }

    return true;
}


/*
* Calls InitializeCriticalSection(...) for every CRITICAL_SECTION handle.
*/
bool InitCriticalSections() {
    InitializeCriticalSection(&CSAnalogData);
    InitializeCriticalSection(&CSStatusData);
    InitializeCriticalSection(&CSAnalogSubs);
    InitializeCriticalSection(&CSStatusSubs);
    InitializeCriticalSection(&CSWorkerTasks);

    return true;
}

/*
* After the service is initialised, enter the listening state.
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
            //pass...
        }
        else if (value == SOCKET_ERROR) {
            //printf("[DEBUG] select failed with error: %d\ncontinueing...\n", WSAGetLastError());
            continue;
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
                            return 0;
                        }

                        break;
                    }
                }
                if (i == MAX_CLIENTS) {
                   // printf("Nema mesta za jos jednu konekciju.\n");
                }
                
            }
            ProcessMessages();
        }
    }

}

/*
* Checks accepted sockets that raised an event and 
* calls TCPLib.TCPReceiveMeasurment to processes the message. 
* Fills publishers and subrscribers list for introducment messages and calls
* ProccessMeasurment for processing data.
* i = index of the accpetedSocket that raised the event flag.
*/
bool Work(int i) {
    char* data = (char*)malloc(sizeof(Measurment));
    int succes = TCPReceive(acceptedSockets[i], data, sizeof(Measurment));

    if (succes == 0) { //always close this socket?
        EnterCriticalSection(&CSAnalogSubs);
        DeleteNode(&analogSubscribers, &acceptedSockets[i], sizeof(SOCKET));
        LeaveCriticalSection(&CSAnalogSubs);

        EnterCriticalSection(&CSStatusSubs);
        DeleteNode(&statusSubscribers, &acceptedSockets[i], sizeof(SOCKET));
        LeaveCriticalSection(&CSStatusSubs);

        closesocket(acceptedSockets[i]);
        acceptedSockets[i] = INVALID_SOCKET;
        free(data);
        return false;
    }
    else if (succes == 2) {
        //nepotreban poziv, nikom nista, WSAWOULDBLOCK
        free(data);
        return false;
    }

    SOCKET* ptr = &acceptedSockets[i];
    //proveri da li je poruka predstavljanja
    if (data[0] == 'p') {
        //GenericListPushAtStart(&publisherList, ptr, sizeof(SOCKET));
        printf("[DEBUG] Connected client: publisher\n");
    }
    else if (data[0] == 'd') {
        //GenericListPushAtStart(&subscriberList, ptr, sizeof(SOCKET));
        printf("[DEBUG] Connected client: subscriber\n");
    }
    else if (data[0] == 'a') {
        EnterCriticalSection(&CSAnalogSubs);
        GenericListPushAtStart(&analogSubscribers, ptr, sizeof(SOCKET));
        SendToNewSubscriber(acceptedSockets[i], analogData);
        LeaveCriticalSection(&CSAnalogSubs);
        printf("[DEBUG] Client %d subscribed to Analog\n", i);
    }
    else if (data[0] == 's') {
        EnterCriticalSection(&CSStatusSubs);
        GenericListPushAtStart(&statusSubscribers, ptr, sizeof(SOCKET));
        SendToNewSubscriber(acceptedSockets[i], statusData);
        LeaveCriticalSection(&CSStatusSubs);
        printf("[DEBUG] Client %d subscribed to Status\n", i);
    }
    else {
        //else message is Measurment data
        Measurment* newMeasurment = (Measurment*)malloc(sizeof(Measurment));
        memcpy(newMeasurment, data, sizeof(Measurment));
        //free(data);
        ProcessMeasurment(newMeasurment);
        free(newMeasurment);
    }

    free(data);
    return true;
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
* Creates a instance of WorkerData and fills it in the workerTasks list.
* Task is then to be done by a worker thread.
*/
void ProcessMessages() {
    //char* data = (char*)malloc(sizeof(Measurment));
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (FD_ISSET(acceptedSockets[i], &readfds)) {
           
            
            WorkerData* taskData = (WorkerData*)malloc(sizeof(WorkerData));
            taskData->i = i;
            //taskData->data = data;

            EnterCriticalSection(&CSWorkerTasks);
            //printf("[DEBUG] Pravim novi task za i = %d\n", i);
            GenericListPushAtStart(&workerTasks, taskData, sizeof(WorkerData));
            LeaveCriticalSection(&CSWorkerTasks);

            free(taskData);
        }
    }
    //free(data);
}

/*
* Save the measurment in the list corresponding to its type.
* m = Measurment to be saved (by reference)
*/
void ProcessMeasurment(Measurment *m) {
    //printf("[DEBUG] Service received: ");
    //PrintMeasurment(m);

    switch (m->topic)
    {
    case Analog:
        EnterCriticalSection(&CSAnalogData);
        GenericListPushAtStart(&analogData, m, sizeof(Measurment));
        LeaveCriticalSection(&CSAnalogData);
        UpdateSubscribers(m, analogSubscribers);
        break;
    case Status:
        EnterCriticalSection(&CSStatusData);
        GenericListPushAtStart(&statusData, m, sizeof(Measurment));
        LeaveCriticalSection(&CSStatusData);
        UpdateSubscribers(m, statusSubscribers);

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
    printf("[INFO] Shuting down..\n");

    printf("[INFO] Closing all sockets..\n");
    closesocket(listenSocket);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (acceptedSockets[i] != INVALID_SOCKET) {
            closesocket(acceptedSockets[i]);
        }
    }
    WSACleanup();
    printf("[INFO] Done\n");



    printf("[INFO] Closing all handles..\n");
    if (!TerminateThread(listenHandle, NULL)) {
        printf("[DEBUG] Cannot terminate listenHandle\n");
    }
    //if (!TerminateThread(workerManagerHandle, NULL)) {
    //    printf("[DEBUG] Cannot terminate workerManagerHandle\n");
    //}

    //create special tasks that worker recognize as termination signals
    WorkerData* terminate = (WorkerData*)malloc(sizeof(WorkerData));
    terminate->i = WORKER_TERMINATE;
    for (int i = 0; i < MAX_THREADS; ++i) {
        GenericListPushAtStart(&workerTasks, terminate, sizeof(WorkerData));
    }
    free(terminate);

    if (workerManagerHandle) {
        WaitForSingleObject(workerManagerHandle, INFINITE);
    }

    SAFE_DELETE_HANDLE(listenHandle);
    SAFE_DELETE_HANDLE(workerManagerHandle);
    for (int i = 0; i < MAX_THREADS; ++i) {
        //if (TerminateThread(workerHandles[i], NULL)) {
        //    printf("[DEBUG] Cannot terminate workerHandles[%d]\n", i);
        //}
        SAFE_DELETE_HANDLE(workerHandles[i]);
    }
    printf("[INFO] Done\n");

    printf("[INFO] Deleting all critical sections.. \n");
    DeleteCriticalSection(&CSStatusData);
    DeleteCriticalSection(&CSAnalogData);
    DeleteCriticalSection(&CSStatusSubs);
    DeleteCriticalSection(&CSAnalogSubs);
    DeleteCriticalSection(&CSWorkerTasks);
    printf("[INFO] Done\n");

    printf("[INFO] Deleting all data.. \n");
    FreeGenericList(&publisherList);
    FreeGenericList(&subscriberList);
    FreeGenericList(&statusData);
    FreeGenericList(&analogData);
    FreeGenericList(&statusSubscribers);
    FreeGenericList(&analogSubscribers);
    FreeGenericList(&workerTasks);
    printf("[INFO] Done\n");

    printf("[INFO] Service freed all memory.\n");
    getchar();
    getchar();
    return;
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

/*
* Infinite loop performed by worker threads. Work is initiated through creation of worker tasks.
* No argument needed.
* Returns true if properly terminated.
*/
DWORD WINAPI DoWork(LPVOID params) {
    bool execute = false;
    while (true) {
        if (workerTasks == NULL) {
            Sleep(10);
        }
        else {
            WorkerData* wData = (WorkerData*)malloc(sizeof(WorkerData));
            EnterCriticalSection(&CSWorkerTasks);
            if (workerTasks != NULL) {
                memcpy(wData, workerTasks->data, sizeof(WorkerData));
                DeleteNode(&workerTasks, workerTasks->data, sizeof(WorkerData));
                execute = true;
            }
            LeaveCriticalSection(&CSWorkerTasks);
            if (execute) {
                //printf("Obradjujem zahtev sa i = %d \ni char *data = %s\n", wData->i, wData->data);
                if (wData->i == WORKER_TERMINATE) { //special code for terminating the worker thread
                    free(wData);
                    return true;
                }
                Work(wData->i);
                execute = false;
            }
            free(wData);
        }

    }
    return false; //should never return here
}