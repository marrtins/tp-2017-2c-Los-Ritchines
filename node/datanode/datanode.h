/*
 * datanode.h
 *
 *  Created on: 4/9/2017
 *      Author: utnso
 */

#ifndef DATANODE_H_
#define DATANODE_H_

#define MAX_PORT_LEN 6
#define MAX_IP_LEN 16
#define MAX_RUTA_LEN 60

typedef struct{

	char* ip_filesystem;
	char* puerto_entrada;
	char* puerto_master;
	char* puerto_filesystem;
	char* ruta_databin;
	int   tipo_de_proceso;
}tDataNode;



tDataNode *getConfigdn(char* ruta);
void mostrarConfiguracion(tDataNode *dn);

#endif /* DATANODE_H_ */
