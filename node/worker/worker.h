/*
 * worker.h
 *
 *  Created on: 5/9/2017
 *      Author: utnso
 */

#ifndef WORKER_H_
#define WORKER_H_

#define MAX_IP_LEN 16
#define MAX_PORT_LEN 8
#define MAX_RUTA_LEN 100
#define MAX_NOMBRE_LEN 100


typedef struct{

	char* ip_filesystem;
	char* puerto_filesystem;
	char* puerto_entrada;
	char* puerto_master;
	char* ruta_databin;
	char* nombre_nodo;
	int   tipo_de_proceso;
}tWorker;

tWorker *getConfigWorker(char* ruta);
void mostrarConfiguracion(tWorker *worker);
#endif /* WORKER_H_ */
