#ifndef LIB_ESTRUCTURASWK_H_
#define LIB_ESTRUCTURASWK_H_

#include "definicionesWK.h"


typedef struct{

	char* ip_filesystem;
	char* puerto_entrada;
	char* puerto_master;
	char* puerto_filesystem;
	char* ruta_databin;
	char* nombre_nodo;
	char* ip_nodo;
	char* puerto_datanode;
	int tipo_de_proceso;
	int tamanio_databin_mb;
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

typedef struct{
	int fdWorker;
	FILE * fdTemporal;
}Tapareo;

typedef struct{
	unsigned long long tamanioContenido;
	char * contenidoArchivo;
}Tscript;

#endif
