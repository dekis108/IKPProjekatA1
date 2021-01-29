#pragma once

#define WORKER_TERMINATE -1

/*
* Struct to be used as parameter for worker threads.
*/
typedef struct worker_data {
	int i;
}WorkerData;


