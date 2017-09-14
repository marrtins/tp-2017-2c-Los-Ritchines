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
#include <commons/string.h>
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


	if ((stat = conectarAFs()) < 0){
			puts("No se pudo conectar con FS!");
			exit(-1);
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


	int sock_master = (int *)client_sock;

	int stat;

	tHeader head = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};
	tPackSrcCode *entradaTransformador;
	tPackSrcCode *entradaReductor;

	tPackBytes *pathArchivoAReducir;
	tPackBytes *pathResultado;

	char* buffer;
	puts("Nuevo hilo MASTERHANDLER creado");



	puts("Esperando solicitud de master");
	while((stat = recv(sock_master, &head, HEAD_SIZE, 0)) > 0){

			puts("Se recibio un paquete de Master");
			printf("proc %d \t msj %d \n", head.tipo_de_proceso, head.tipo_de_mensaje);

			switch(head.tipo_de_mensaje){





			case SRC_CODE_TRANSF:
					puts("Master quiere iniciar un JOB");
					puts("Nos llega el codigo fuente del script TRANSFORMDOR");


					if ((buffer = recvGeneric(sock_master)) == NULL){
						puts("Fallo recepcion de SRC_CODE TRANSFORMADOR");

						return;
					}

					if ((entradaTransformador = (tPackSrcCode *) deserializeBytes(buffer)) == NULL){

						puts("Fallo deserializacion de Bytes del src_transformador");
						return;
					}
					printf("Codigo fuente Transformador: %s \n",entradaTransformador->bytes);
					freeAndNULL((void **) &buffer);
					puts("fin case src_code_trans");
					break;
			case SRC_CODE_RED:

				puts("Nos llega el codigo fuente del script REDUCTOR");


				if ((buffer = recvGeneric(sock_master)) == NULL){
					puts("Fallo recepcion de SRC_CODE REDUCTOR");

					return;
				}

				if ((entradaReductor = (tPackSrcCode *) deserializeBytes(buffer)) == NULL){

					puts("Fallo deserializacion de Bytes del src_reductor");
					return;
				}

				printf("Codigo fuente reductor: %s \n",entradaReductor->bytes);
				freeAndNULL((void **) &buffer);
				puts("fin case src_code_red");

				break;
			case PATH_FILE_TOREDUCE:

				puts("Nos llega el path del archivo a reducir");


				if ((buffer = recvGeneric(sock_master)) == NULL){
					puts("Fallo recepcion de PATH_FILE_TOREDUCE");

					return;
				}

				if ((pathArchivoAReducir = (tPackBytes *) deserializeBytes(buffer)) == NULL){

					puts("Fallo deserializacion de Bytes del path arch a reducir");
					return;
				}

				printf("Path archivo a reducir: : %s\n",pathArchivoAReducir->bytes);
				freeAndNULL((void **) &buffer);
				puts("fin case PATH_FILE_TOREDUCE");

				break;
			case PATH_RES_FILE:

							puts("Nos llega el path del resultado");


							if ((buffer = recvGeneric(sock_master)) == NULL){
								puts("Fallo recepcion de PATH_RES_FILE");

								return;
							}

							if ((pathResultado = (tPackBytes *) deserializeBytes(buffer)) == NULL){

								puts("Fallo deserializacion de Bytes del path_res_file");
								return;
							}

							printf("Path del resultado: : %s\n",pathResultado->bytes);
							freeAndNULL((void **) &buffer);
							puts("fin case PATH_RES_FILE");

							//Como este es el último atributo que recibimos de master.  Aca le enviamos la "info" de los workers a los que se tiene que conectar
							//Por el momento info hardcode y sera la info de los unicos 2 workers conectados. Se la pedimos al filesystem y cuando nos la devuelve, le reenviamos a master.

							/* pido info, la proceso y ahora se a que workers el master se va a conectar y se los paso...*/

							//x ahora solo le aviso q inice la transf local

							tHeader head;
							int stat;
							head.tipo_de_proceso=YAMA;
							head.tipo_de_mensaje=START_LOCALTRANSF;

							if((stat=enviarHeader(sock_master,head))<0){
								puts("No se pudo enviar el header de inicio");
							}


							break;
			default:
				break;

			}

		}


}



int conectarAFs(){

	int stat;
	tHeader head;
	// Se trata de conectar con FS
	if ((sock_fs = establecerConexion(yama->ip_filesystem, yama->puerto_filesystem)) < 0){
		fprintf(stderr, "No se pudo conectar con FS! sock_fs: %d\n", sock_fs);

		return FALLO_CONEXION;
	}
	head.tipo_de_mensaje=INICIOYAMA;
	head.tipo_de_proceso=YAMA;
	// No permitimos continuar la ejecucion hasta lograr un handshake con FS
	if ((stat = enviarHeader(sock_fs, head)) < 0){
		fprintf(stderr, "No se pudo hacer hadshake con FS\n");

		return FALLO_GRAL;
	}

	printf("Se enviaron: %d bytes a FS del handshake \n", stat);

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

