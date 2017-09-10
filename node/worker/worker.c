/*
 * worker.c
 *
 *  Created on: 31/8/2017
 *      Author: utnso
 */



#include <commons/config.h>
#include <stdlib.h>

#include "worker.h"

#include "../../compartidas/definiciones.h"
#include "../../compartidas/compartidas.c"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdio.h>

tWorker *worker;



int main(int argc, char* argv[]){

	int stat;

	if(argc!=2){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}


	worker=getConfigWorker(argv[1]);
	mostrarConfiguracion(worker);

	int sock_entrada , client_sock , clientSize;

	struct sockaddr_in client;
	clientSize = sizeof client;


	if ((sock_entrada = makeListenSock(worker->puerto_entrada)) < 0){
		fprintf(stderr, "No se pudo crear un socket de listen. fallo: %d", sock_entrada);
		return FALLO_GRAL;
	}

	//Listen
	if ((stat = listen(sock_entrada , BACKLOG)) == -1){
		perror("No se pudo hacer listen del socket. error");
		return FALLO_GRAL;
	}

	//acepta y escucha comunicaciones
	tHeader head;
	puts("esperando comunicaciones entrantes...");
	while((client_sock = accept(sock_entrada, (struct sockaddr*) &client, (socklen_t*) &clientSize)) != -1){
		puts("Conexion aceptada");
		if ((stat = recv(client_sock, &head, HEAD_SIZE, 0)) < 0){
			perror("Error en la recepcion de header. error");
			return FALLO_RECV;
		}

		switch(head.tipo_de_proceso){

		case MASTER:
			puts("Se conecto master, forkeamos para que atienda su solicitud");
			/*
			 * FORK ETC ETC ETC
			 */
			break;
		default:
			puts("Trato de conectarse algo que no era ni Kernel ni CPU!");
			printf("El tipo de proceso y mensaje son: %d y %d\n", head.tipo_de_proceso, head.tipo_de_mensaje);
			printf("Se recibio esto del socket: %d\n", client_sock);
			return CONEX_INVAL;
		}
	}

	// Si salio del ciclo es porque fallo el accept()

	perror("Fallo el accept(). error");

	//liberarConfiguracionYama();





	return 0;
}





tWorker *getConfigWorker(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tWorker *worker = malloc(sizeof(tWorker));

	worker->ip_filesystem       =    malloc(MAX_IP_LEN);
	worker->puerto_entrada= malloc(MAX_PORT_LEN);
	worker->puerto_master = malloc(MAX_PORT_LEN);
	worker->puerto_filesystem = malloc(MAX_PORT_LEN);
	worker->ruta_databin=malloc(MAX_RUTA_LEN);
	worker->nombre_nodo=malloc(MAX_NOMBRE_LEN);

	t_config *workerConfig = config_create(ruta);

	strcpy(worker->ip_filesystem, config_get_string_value(workerConfig, "IP_FILESYSTEM"));
	strcpy(worker->puerto_entrada, config_get_string_value(workerConfig, "PUERTO_WORKER"));
	strcpy(worker->puerto_master, config_get_string_value(workerConfig, "PUERTO_MASTER"));
	strcpy(worker->puerto_filesystem, config_get_string_value(workerConfig, "PUERTO_FILESYSTEM"));
	strcpy(worker->ruta_databin, config_get_string_value(workerConfig, "RUTA_DATABIN"));
	strcpy(worker->nombre_nodo, config_get_string_value(workerConfig, "NOMBRE_NODO"));





	//worker->tipo_de_proceso = DATANODE;

	config_destroy(workerConfig);
	return worker;
}
void mostrarConfiguracion(tWorker *worker){

	printf("Puerto Entrada: %s\n",  worker->puerto_entrada);
	printf("IP Filesystem %s\n",    worker->ip_filesystem);
	printf("Puerto Master: %s\n",       worker->puerto_master);
	printf("Puerto Filesystem: %s\n", worker->puerto_filesystem);
	printf("Ruta Databin: %s\n", worker->ruta_databin);
	printf("Nombre Nodo: %s\n", worker->nombre_nodo);
	printf("Tipo de proceso: %d\n", worker->tipo_de_proceso);
}
