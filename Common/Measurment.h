// Measurment.cpp : Defines the Measurment structure thats used for service-client communication.
//

#include "pch.h"
#include "framework.h"


#pragma once


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


const char* GetStringFromEnumHelper(Type type);


/*
* Print measurment struct, specifically to be used by the generic list.
*/
void PrintMeasurment(Measurment* m);

