/*
 * yama.c
 *
 *  Created on: 31/8/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include "yama.h"
#include <commons/config.h>
#include <string.h>

#include <netinet/in.h>

#include <funcionesCompartidas/funcionesCompartidas.h>
#include <funcionesPaquetes/funcionesPaquetes.h>
#include <tiposRecursos/tiposErrores.h>
#include <tiposRecursos/tiposPaquetes.h>



/*#Var Globales*/

tYama *yama;
int sock_fs;



int main(int argc, char* argv[]){


	int stat;

	if(argc!=2){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}


	yama=getConfigYama(argv[1]);
	mostrarConfiguracion(yama);


	int sock_entrada , client_sock , clientSize;

	struct sockaddr_in client;
	clientSize = sizeof client;

	if ((stat = conectarConFS(yama)) != 0){
		puts("No se pudo conectar con FileSystem");
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
	tPackHeader head;
	puts("esperando comunicaciones de master...");
	while((client_sock = accept(sock_entrada, (struct sockaddr*) &client, (socklen_t*) &clientSize)) != -1){
		puts("Conexion aceptada");

		if ((stat = recv(client_sock, &head, HEAD_SIZE, 0)) < 0){

			perror("Error en la solicitud de inicio. error");
			return FALLO_RECV;
		}

		switch(head.tipo_de_proceso){
		case MASTER:
			printf("Se conecto master\n");
			break;
		default:
			printf("Se conecto un proceso desconocido");
			break;
		}



	}
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
	tPackHeader h_esp;
	tPackHeader h_obt;

	printf("Conectando con FS...\n");

	sock_fs = establecerConexion(yama->ip_filesystem, yama->puerto_filesystem);
	if (sock_fs < 0){
		puts("Fallo conexion a FS");
		return FALLO_CONEXION;
	}

	if ((stat = handshakeCon(sock_fs, yama->tipo_de_proceso)) < 0){

		fprintf(stderr, "No se pudo hacer hadshake con Memoria\n");
		return FALLO_GRAL;
	}

	printf("Se enviaron: %d bytes a FS\n", stat);

	puts("Me conecte a FS!");

	h_esp.tipo_de_proceso = FILESYSTEM; h_esp.tipo_de_mensaje = INICIOFS;
	if (validarRespuesta(sock_fs, h_esp, &h_obt) != 0){
		printf("Se recibio un mensaje no esperado de FS\n");
	}

	printf("Me conecte a FILESYSTEM en socket #%d\n",sock_fs);
		return 0;
}


