/*
 * master.c
 *
 *  Created on: 31/8/2017
 *      Author: utnso
 */



#include <commons/config.h>
#include <stdlib.h>

#include "master.h"

#include "../compartidas/definiciones.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

tMaster *master;
int sock_yama;

int main(int argc, char* argv[]){

	int stat;

	if(argc!=2){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}


	master=getConfigMaster(argv[1]);
	mostrarConfiguracion(master);


	if ((stat = conectarAYama()) < 0){
				puts("No se pudo conectar con Yama!");
				return 0;
	}




	puts("Conectado a yama.. Inicio ejecucion..");

	while(1);

	return 0;
}


int conectarAYama(){

	int stat;
	int sock_yama;
	tHeader h_esperado,h_obtenido;

	// Se trata de conectar con YAMA
	if ((sock_yama = establecerConexion(master->ip_yama, master->puerto_yama)) < 0){
		fprintf(stderr, "No se pudo conectar con YAMA! sock_yama: %d\n", sock_yama);

		return FALLO_CONEXION;
	}

	// No permitimos continuar la ejecucion hasta lograr un handshake con YAMA
		if ((stat = handshakeCon(sock_yama, master->tipo_de_proceso)) < 0){
			fprintf(stderr, "No se pudo hacer hadshake con YAMA\n");

			return FALLO_GRAL;
		}



	printf("Se enviaron: %d bytes a YAMA\n", stat);

	return 0;
}



tMaster *getConfigMaster(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tMaster *master = malloc(sizeof(tMaster));

	master->ip_yama=malloc(MAX_IP_LEN);
	master->puerto_yama = malloc(MAX_PORT_LEN);

	t_config *masterConfig = config_create(ruta);

	strcpy(master->ip_yama, config_get_string_value(masterConfig, "IP_YAMA"));
	strcpy(master->puerto_yama, config_get_string_value(masterConfig, "PUERTO_YAMA"));

	master->tipo_de_proceso = MASTER;

	config_destroy(masterConfig);
	return master;
}
void mostrarConfiguracion(tMaster *master){

	printf("IP Yama %s\n",    master->ip_yama);
	printf("Puerto Yama: %s\n",       master->puerto_yama);
	printf("Tipo de proceso: %d\n", master->tipo_de_proceso);
}

int establecerConexion(char *ip_dest, char *port_dest){

	int stat;
	int sock_dest; // file descriptor para el socket del destino a conectar
	struct addrinfo hints, *destInfo;

	setupHints(&hints, AF_INET, SOCK_STREAM, 0);

	if ((stat = getaddrinfo(ip_dest, port_dest, &hints, &destInfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(stat));
		return FALLO_GRAL;
	}

	if ((sock_dest = socket(destInfo->ai_family, destInfo->ai_socktype, destInfo->ai_protocol)) == -1){
		perror("No se pudo crear socket. error.");
		return FALLO_GRAL;
	}

	if ((stat = connect(sock_dest, destInfo->ai_addr, destInfo->ai_addrlen)) == -1){
		perror("No se pudo establecer conexion, fallo connect(). error");
		printf("Fallo conexion con destino IP: %s PORT: %s\n", ip_dest, port_dest);
		return FALLO_CONEXION;
	}

	freeaddrinfo(destInfo);

	if (sock_dest < 0){
		printf("Error al tratar de conectar con Yama!\n");
		return FALLO_CONEXION;
	}

	return sock_dest;
}

void setupHints(struct addrinfo *hints, int address_family, int socket_type, int flags){
	memset(hints, 0, sizeof *hints);
	hints->ai_family = address_family;
	hints->ai_socktype = socket_type;
	hints->ai_flags = flags;
}

int handshakeCon(int sock_dest, int id_sender){

	int stat;
	char *package;
	tHeader head;
	head.tipo_de_proceso = id_sender;
	head.tipo_de_mensaje = INICIOYAMA;

	if ((package = malloc(HEAD_SIZE)) == NULL){
		fprintf(stderr, "No se pudo hacer malloc\n");
		return FALLO_GRAL;
	}
	memcpy(package, &head, HEAD_SIZE);

	if ((stat = send(sock_dest, package, HEAD_SIZE, 0)) == -1){
		perror("Fallo send de handshake. error");
		printf("Fallo send() al socket: %d\n", sock_dest);
		return FALLO_SEND;
	}

	printf("Envié un paquete a %d\n",sock_dest);
	free(package);
	return stat;
}


