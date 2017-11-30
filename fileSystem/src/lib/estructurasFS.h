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

typedef struct{
	int tamanio;
	char * rutaArchivo;
	unsigned long long tamanioContenido;
	char * contenidoArchivo;
}TarchivoFinal;

typedef struct{
	char * nombreArchivo;
	t_list * bloques;
}TarchivoDeTablaArchivoGlobal;

typedef struct{
	char * nombreNodo;
	t_list * archivos;
}TelementoDeTablaArchivoGlobal;

typedef struct lista{
	int valor;
	struct lista * siguiente;
}TlistaCircular;

t_list * listaDeNodos;
t_list * listaDeNodosDesconectados;
t_list * listaTablaDirectorios;
t_list * listaInfoNodo;

//Se usa en el cpblock;
Tbuffer* bloqueACopiar;

pthread_mutex_t bloqueMutex;

sem_t yama;

int esEstadoRecuperado;
int cantDeNodosDeEstadoAnterior;

#endif
