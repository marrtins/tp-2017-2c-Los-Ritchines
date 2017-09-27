#ifndef LIB_ESTRUCTURAS_H_
#define LIB_ESTRUCTURAS_H_

#include "definiciones.h"

typedef struct{

	char* ip_filesystem;
	char* puerto_entrada;
	char* puerto_master;
	char* puerto_filesystem;
	int   algoritmo_balanceo,
	   	  retardo_planificacion,
		  tipo_de_proceso;
}Tyama;

typedef struct {

	Theader head;
	int bytelen;
	char *bytes;
} TpackSrcCode,TpackBytes;

typedef struct {

	Theader head;
	int bytelen1;
	char *bytes1;
	int bytelen2;
	char *bytes2;
}__attribute__((packed)) Tpack2Bytes;

typedef struct {

	Theader head;
	int puertoLen;
	char *puertoWorker;
	int ipLen;
	char *ipNodo;
	int nombreLen;
	char *nombreNodo;
}__attribute__((packed)) TpackInfoNodo;

t_log* logger;

#endif
