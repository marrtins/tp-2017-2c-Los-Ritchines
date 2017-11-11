#ifndef LIB_ESTRUCTURASDN_H_
#define LIB_ESTRUCTURASDN_H_

#include "definicionesDN.h"

typedef struct{
	char* ip_filesystem;
	char* puerto_entrada;
	char* puerto_master;
	char* puerto_filesystem;
	char* ruta_databin;
	char* nombre_nodo;
	char* ip_nodo;
	char* puerto_worker;
	int tipo_de_proceso;
	int tamanio_databin_mb;
}TdataNode;

typedef struct{
	int nroBloque;
	unsigned long long tamanioContenido;
	char *contenido;
}Tbloque;

char * archivoMapeado;

#endif
