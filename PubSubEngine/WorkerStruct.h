#pragma once

#define WORKER_TERMINATE -1

#include "../Common/GenericList.h";

/*
* Struct to be used as parameter for worker threads.
*/
typedef struct worker_data {
	int i;
}WorkerData;


typedef struct service_metadata {
	fd_set readfds;

	HANDLE listenHandle;
	HANDLE workerManagerHandle;
	HANDLE* workerHandles;
	SOCKET listenSocket;
	SOCKET* acceptedSockets;

	CRITICAL_SECTION CSStatusData;
	CRITICAL_SECTION CSAnalogData;
	CRITICAL_SECTION CSStatusSubs;
	CRITICAL_SECTION CSAnalogSubs;
	CRITICAL_SECTION CSWorkerTasks;

	NODE* statusData;
	NODE* analogData;
	NODE* statusSubscribers;
	NODE* analogSubscribers;
	NODE* workerTasks;

}METADATA;



