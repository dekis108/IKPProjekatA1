// Measurment.cpp : Defines the Measurment structure thats used for service-client communication.
//

#include "pch.h"
#include "framework.h"


#pragma once

#define STRING_LENGHT 10

typedef struct _msgFormat {
    char topic[STRING_LENGHT];
    char type[STRING_LENGHT];
    int value;

}Measurment;