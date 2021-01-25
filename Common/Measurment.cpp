#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#include "../Common/Measurment.h";

/*
* Get string representation from enum object, can't do it better in c.
*/
const char* GetStringFromEnumHelper(Topic topic) {
	const char* strings[] = { "Analog", "Status" };
	return strings[topic];
}


const char* GetStringFromEnumHelper(Type type) {
	const char* strings[] = { "SWG", "CRB", "MER" };
	return strings[type];
}


/*
* Print measurment struct, specifically to be used by the generic list.
*/
void PrintMeasurment(Measurment* m) {
	printf("Measurment: ");
	printf(" %s %s %d\n", GetStringFromEnumHelper(m->topic), GetStringFromEnumHelper(m->type), m->value);
}

