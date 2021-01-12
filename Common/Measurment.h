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