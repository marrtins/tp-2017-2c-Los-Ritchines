/*
 * estructuras.h
 *
 *  Created on: 20/9/2017
 *      Author: utnso
 */

#ifndef LIB_ESTRUCTURAS_H_
#define LIB_ESTRUCTURAS_H_

#include "definiciones.h"

typedef struct{

	char* ip_filesystem;
	char* puerto_entrada;
	char* puerto_master;
	char* puerto_filesystem;
	char* ruta_databin;
	char* nombre_nodo;
	char* ip_nodo;
	char* puerto_worker;
	int   tipo_de_proceso;
}TdataNode;

t_log* logger;

#endif /* LIB_ESTRUCTURAS_H_ */
