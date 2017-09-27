#ifndef LIB_ESTRUCTURAS_H_
#define LIB_ESTRUCTURAS_H_

#include "definiciones.h"

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
