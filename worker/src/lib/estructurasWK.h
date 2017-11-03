#ifndef LIB_ESTRUCTURASWK_H_
#define LIB_ESTRUCTURASWK_H_

#include "definicionesWK.h"


typedef struct{

	char* ip_filesystem;
	char* puerto_filesystem;
	char* puerto_entrada;
	char* puerto_master;
	char* ruta_databin;
	char* nombre_nodo;
	int   tipo_de_proceso;
}Tworker;


typedef struct{
	char * nombreArchivoTemporal;
	char * rutaArchivoTemporal;
	int jobAsociado;
}TinfoTemporalJobAsoc;
typedef struct{
	int fdWorker;
	bool eofTemporal;
	bool encargado;
}TinfoApareoGlobal;

#endif
