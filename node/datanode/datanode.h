/*
 * datanode.h
 *
 *  Created on: 4/9/2017
 *      Author: utnso
 */

#ifndef DATANODE_H_
#define DATANODE_H_

#define MAX_PORT_LEN 4
#define MAX_IP_LEN 16
#define MAX_RUTA_LEN 100
#define MAX_NOMBRE_LEN 100

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
}tDataNode;



tDataNode *getConfigdn(char* ruta);
void mostrarConfiguracion(tDataNode *dn);

#endif /* DATANODE_H_ */
