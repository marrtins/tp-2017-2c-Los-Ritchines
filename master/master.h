/*
 * master.h
 *
 *  Created on: 5/9/2017
 *      Author: utnso
 */

#ifndef MASTER_H_
#define MASTER_H_

#define MAX_IP_LEN 16
#define MAX_PORT_LEN 10


typedef struct{

	char* ip_yama;
	char* puerto_yama;
	int   tipo_de_proceso;
}tMaster;

typedef struct{

	char* ip_worker;
	char* puerto_worker;
}tInfoWorker;



tMaster *getConfigMaster(char* ruta);
void mostrarConfiguracion(tMaster *master);
int conectarAYama();
void *workerHandler(void *atributos);

#endif /* MASTER_H_ */
