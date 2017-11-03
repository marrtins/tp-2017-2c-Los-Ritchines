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
	int sockYama;
}TatributosHilo;

typedef struct{
	TreduccionLocal infoReduccion;
	int sockYama;
}TatributosHiloReduccionLocal;

typedef struct{
	TreduccionGlobal infoReduccionGlobal;
	int sockYama;
}TatributosHiloReduccionGlobal;

typedef struct{
	TinfoAlmacenadoFinal infoAlmacenamiento;
	int sockYama;
}TatributosHiloAlmacenamientoFinal;

#endif
