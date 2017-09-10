/*
 * yama.c
 *
 *  Created on: 31/8/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

#include <sys/socket.h>
#include "yama.h"
#include <commons/config.h>
#include <string.h>

#include <pthread.h>

#include "../compartidas/definiciones.h"
#include "../compartidas/compartidas.c"

#include <netinet/in.h>

/*#Var Globales*/

tYama *yama;
int sock_fs;



int main(int argc, char* argv[]){
	int stat;



	pthread_t fs_thread;
	pthread_t master_thread;

	if(argc!=2){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}


	yama=getConfigYama(argv[1]);
	mostrarConfiguracion(yama);


	int sock_entrada , client_sock , clientSize;

	struct sockaddr_in client;
	clientSize = sizeof client;


	if(pthread_create(&fs_thread, NULL, (void*) fsHandler, NULL) < 0){
		perror("no pudo crear hilo. error");
		return FALLO_GRAL;
	}


	if ((sock_entrada = makeListenSock(yama->puerto_entrada)) < 0){
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
			puts("Se conecto master, creamos hilo manejador");
			if(pthread_create(&master_thread, NULL, (void*) masterHandler,(void*) client_sock) < 0){
				perror("No pudo crear hilo. error");
				return FALLO_GRAL;
			}
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


void masterHandler(void *client_sock){


	int *sock_master = (int *)client_sock;

	int stat;
	tHeader *head;

	puts("Nuevo hilo MASTERHANDLER creado");


	puts("Esperando solicitud de master");
	while((stat = recv(*sock_master, &head, HEAD_SIZE, 0)) > 0){

			puts("Se recibio un paquete de Master");
			printf("proc %d \t msj %d \n", head->tipo_de_proceso, head->tipo_de_mensaje);

			switch(head->tipo_de_mensaje){

			case(1):
				break;

			default:
				break;

			}

		}


}


void fsHandler(void){

	int stat;

	tHeader *head;

	if ((stat = conectarAFs()) < 0){
			puts("No se pudo conectar con FS!");
	}

	while((stat = recv(sock_fs, &head, HEAD_SIZE, 0)) > 0){

		puts("Se recibio un paquete de FS");
		printf("proc %d \t msj %d \n", head->tipo_de_proceso, head->tipo_de_mensaje);

		switch(head->tipo_de_mensaje){

		case(1):
				break;

		default:
			break;

		}

	}


}
int conectarAFs(){

	int stat;

	// Se trata de conectar con FS
	if ((sock_fs = establecerConexion(yama->ip_filesystem, yama->puerto_filesystem)) < 0){
		fprintf(stderr, "No se pudo conectar con FS! sock_fs: %d\n", sock_fs);

		return FALLO_CONEXION;
	}

	// No permitimos continuar la ejecucion hasta lograr un handshake con FS
		if ((stat = handshakeCon(sock_fs, yama->tipo_de_proceso)) < 0){
			fprintf(stderr, "No se pudo hacer hadshake con FS\n");

			return FALLO_GRAL;
		}


	// No permitimos continuar la ejecucion hasta no recibir el primer mensaje de FS
	/*h_esperado.tipo_de_proceso=YAMA;
	h_esperado.tipo_de_mensaje=INICIOYAMA;

	if ((stat = validarRespuesta(sock_yama,h_esperado,&h_obtenido)) != 0){
			fprintf(stderr, "No se recibio el mensaje de inicio de YAMA\n");
			return FALLO_GRAL;
		}

	*/

	printf("Se enviaron: %d bytes a FS\n", stat);

	return 0;
}

tYama *getConfigYama(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tYama *yama = malloc(sizeof(tYama));

	yama->ip_filesystem       =    malloc(MAX_IP_LEN);
	yama->puerto_entrada = malloc(MAX_PORT_LEN);
	yama->puerto_master = malloc(MAX_PORT_LEN);
	yama->puerto_filesystem = malloc(MAX_PORT_LEN);

	t_config *yamaConfig = config_create(ruta);

	strcpy(yama->ip_filesystem, config_get_string_value(yamaConfig, "IP_FILESYSTEM"));
	strcpy(yama->puerto_entrada, config_get_string_value(yamaConfig, "PUERTO_YAMA"));
	strcpy(yama->puerto_master, config_get_string_value(yamaConfig, "PUERTO_MASTER"));
	strcpy(yama->puerto_filesystem, config_get_string_value(yamaConfig, "PUERTO_FILESYSTEM"));


	yama->retardo_planificacion = config_get_int_value(yamaConfig, "RETARDO_PLANIFICACION");
	yama->algoritmo_balanceo =          config_get_int_value(yamaConfig, "ALGORITMO_BALANCEO");
	yama->tipo_de_proceso = YAMA;

	config_destroy(yamaConfig);
	return yama;
}
void mostrarConfiguracion(tYama *yama){

	printf("Puerto Entrada: %s\n",  yama->puerto_entrada);
	printf("IP Filesystem %s\n",    yama->ip_filesystem);
	printf("Puerto Master: %s\n",       yama->puerto_master);
	printf("Puerto Filesystem: %s\n", yama->puerto_filesystem);
	printf("Retardo Planificacion: %d\n",   yama->retardo_planificacion);
	printf("Algoritmo Balanceo: %d\n", yama->algoritmo_balanceo);
	printf("Tipo de proceso: %d\n", yama->tipo_de_proceso);
}

int conectarConFS(tYama *yama){

	int stat;

	printf("Conectando con FS...\n");

	sock_fs = establecerConexion(yama->ip_filesystem, yama->puerto_filesystem);
	if (sock_fs < 0){
		puts("Fallo conexion a FS");
		return FALLO_CONEXION;
	}

	if ((stat = handshakeCon(sock_fs, yama->tipo_de_proceso)) < 0){

		fprintf(stderr, "No se pudo hacer hadshake con FS\n");
		return FALLO_GRAL;
	}

	printf("Se enviaron: %d bytes a FS\n", stat);

	puts("Me conecte a FS!");



	printf("Me conecte a FILESYSTEM en socket #%d\n",sock_fs);
		return 0;
}



