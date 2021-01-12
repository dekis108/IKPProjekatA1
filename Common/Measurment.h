// Measurment.cpp : Defines the Measurment structure thats used for service-client communication.
//

#include "pch.h"
#include "framework.h"


#pragma once


enum MeasurmentTopic {Analog = 0, Status};
enum MeasurmentType {SWG = 0, CRB, MER};

typedef struct _msgFormat {
    MeasurmentTopic topic;
    MeasurmentType type;
    int value;

}Measurment;


//VISUAL STUDIO MI NE DA DISEM
/*
char* GetStringFromEnumHelper(MeasurmentTopic topic) {
	//const char* strings[] = { "Analog", "Status" };
	//return "Deja";
	//return strings[topic];
}

*/

/*
char* GetStringFromEnumHelper(MeasurmentType type) {
	switch (type)
	{
	case SWG:
		char returnValue1[] = "SWG";
		return returnValue1;
	case CRB:
		char returnValue2[] = "CRB";
		return returnValue2;
	case MER:
		char returnValue3[] = "MER";
		return returnValue3;
	default:
		char returnValue4[] = "StringFromEnumHelper not updated for this value";
		return returnValue4;
	}
}*/