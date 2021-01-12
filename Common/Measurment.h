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


//VISUAL STUDIO MI NE DA DISEM

const char* GetStringFromEnumHelper(Topic topic) {
	const char* strings[] = { "Analog", "Status" };
	return strings[topic];
}


char* GetStringFromEnumHelper(Type type) {
	char* retVal = (char *)malloc(50);
	switch (type)
	{
	case SWG:
		strcpy(retVal, "SWG");
		break;
	case CRB:
		strcpy(retVal, "CRB");
		break;
	case MER:
		strcpy(retVal, "MER");
		break;
	default:
		strcpy(retVal, "StringFromEnumHelper not updated for this value");
		break;
	}
	return retVal;
}