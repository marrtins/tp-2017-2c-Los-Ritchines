#ifndef LIB_ESTRUCTURASMS_H_
#define LIB_ESTRUCTURASMS_H_

#include "definicionesMS.h"

typedef struct{
	char * ipYama;
	char * puertoYama;
	int tipoDeProceso;
}Tmaster;

typedef struct{
	char* ipWorker;
	char* puertoWorker;
}TinfoWorker;

t_log* logger;

#endif
