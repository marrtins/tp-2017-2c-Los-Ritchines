#ifndef LIB_ESTRUCTURASFS_H_
#define LIB_ESTRUCTURASFS_H_

#include "definicionesFS.h"

typedef struct{

	char* puerto_entrada;
	char* puerto_datanode;
	char* puerto_yama;
	char* ip_yama;
	int tipo_de_proceso;
	int cant_nodos;

}TfileSystem;

//el index es el indice del vector
typedef struct{
	int index;
	char nombre[255];
	int padre;
}Tdirectorio;

typedef struct{
	char * nombreDeNodo;
	int numeroBloqueDeNodo;
	int nroDeCopia;
}TcopiaNodo;

typedef struct{
	t_list * copia;
	unsigned long long bytes;
	int cantidadCopias;
}Tbloques;

typedef struct{
	char * nombreArchivoSinExtension;
	unsigned long long tamanioTotal;
	char * extensionArchivo;
	Tbloques * bloques;
}Tarchivo;

typedef struct{
	int cantBloques;
	int cantLibres;
}TnodoBloque;

typedef struct{
	int cantBloquesTotal;
	int cantLibresTotal;
	char ** nodos;
	TnodoBloque * nodoBloques;
}Tnodos;

t_list * listaDeNodos;
t_list * listaDeNodosDesconectados;
t_list * listaTablaDirectorios;

//Se usa en el cpblock;
Tbuffer* bloqueACopiar;

pthread_mutex_t bloqueMutex;

//bool
int estable;

#endif
