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
}Tdirectorios;

typedef struct{
	char * nombreDeNodo;
	char * numeroBloqueDeNodo;
}TcopiaNodo;

typedef struct{
	TcopiaNodo copiaCero;
	TcopiaNodo copiaUno;
	unsigned long long bytes;
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

typedef struct{
	int numeroDeBloque;
	char *contenido;
	unsigned long long tamanio;
}TbloqueAEnviar;

typedef struct{
	char* nombre;
	int fd;
	unsigned int cantidadBloquesTotal;
	unsigned int cantidadBloquesLibres;
	int primerBloqueLibreBitmap;
	t_bitarray * bitmap;
}Tnodo;

t_list * listaDeNodos;
Tdirectorios tablaDirectorios[100];

#endif
