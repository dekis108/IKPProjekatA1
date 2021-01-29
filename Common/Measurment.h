// Measurment.cpp : Defines the Measurment structure thats used for service-client communication.
//
#ifndef MEASURMENT
#define MEASURMENT

#pragma once

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "pch.h"



typedef enum MeasurmentTopic {Analog = 0, Status}Topic;
typedef enum MeasurmentType {SWG = 0, CRB, MER}Type;

typedef struct _msgFormat {
    Topic topic;
    Type type;
    int value;

}Measurment;


/*
* Get string representation from enum object, can't do it better in c.
*/
const char* GetStringFromEnumHelper(Topic topic);


/*
* Get string representation from enum object, can't do it better in c.
*/
const char* GetStringFromEnumHelper(Type type);


/*
* Print measurment struct.
* *m = pointer to the Measurment to be printed
*/
void PrintMeasurment(Measurment* m);

#endif // !MEASURMENT
