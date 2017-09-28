#ifndef LIB_ESTRUCTURASFS_H_
#define LIB_ESTRUCTURASFS_H_

#include "definicionesFS.h"

typedef struct{

	char* puerto_entrada;
	char* puerto_datanode;
	char* puerto_yama;
	char* ip_yama;
	int tipo_de_proceso;

}TfileSystem;

//el index es el indice del vector
typedef struct{
	char nombre[255];
	int padre;
}Tdirectorios;

typedef struct{
	char * nombreDeNodo;
	int numeroBloqueDeNodo;
}TcopiaNodo;

typedef struct{
	TcopiaNodo copiaCero;
	TcopiaNodo copiaUno;
	int bytes;
}Tbloques;

typedef struct{
	int tamanioTotal;
	char * extensionArchivo;
	Tbloques * bloques;
}Tarchivos;

t_log* logger;

#endif
