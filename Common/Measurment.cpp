#include "framework.h"
#include "pch.h"
#include "Measurment.h"


const char* GetStringFromEnumHelper(Topic topic) {
	const char* strings[] = { "Analog", "Status" };
	return strings[topic];
}

const char* GetStringFromEnumHelper(Type type) {
	const char* strings[] = { "SWG", "CRB", "MER" };
	return strings[type];
}

void PrintMeasurment(Measurment* m) {
	printf("Measurment: ");
	printf(" %s %s %d\n", GetStringFromEnumHelper(m->topic), GetStringFromEnumHelper(m->type), m->value);
}
