#ifndef LIB_ESTRUCTURASMS_H_
#define LIB_ESTRUCTURASMS_H_

#include "definicionesMS.h"

typedef struct{
	char * ipYama;
	char * puertoYama;
	int tipoDeProceso;
}Tmaster;

typedef struct{
	int ipLen;
	int puertoLen;
	char* ipWorker;
	char* puertoWorker;
}TinfoWorker;

typedef struct{
	TpackInfoBloque infoBloque;
	TinfoWorker infoWorker;
}TatributosHilo;

#endif
