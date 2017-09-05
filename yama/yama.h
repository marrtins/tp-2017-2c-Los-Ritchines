/*
 * yama.h
 *
 *  Created on: 1/9/2017
 *      Author: utnso
 */

#ifndef YAMA_H_
#define YAMA_H_

#define MAX_PORT_LEN 6
#define MAX_IP_LEN 16

#define BACKLOG 20

typedef struct{

	char* ip_filesystem;
	char* puerto_entrada;
	char* puerto_master;
	char* puerto_filesystem;
	int   algoritmo_balanceo,
	   	  retardo_planificacion,
		  tipo_de_proceso;
}tYama;



tYama *getConfigYama(char* ruta);

void mostrarConfiguracion(tYama *yama);
int conectarConFS(tYama *yama);


#endif /* YAMA_H_ */
