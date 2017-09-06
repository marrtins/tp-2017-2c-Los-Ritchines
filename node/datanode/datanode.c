/*
 * datanode.c
 *
 *  Created on: 31/8/2017
 *      Author: utnso
 */



#include <commons/config.h>
#include <stdlib.h>

#include "datanode.h"

#include "../../compartidas/definiciones.h"


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


#define BACKLOG 20

tDataNode *datanode;


tDataNode *getConfigdn(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tDataNode *dn = malloc(sizeof(tDataNode));

	dn->ip_filesystem       =    malloc(MAX_IP_LEN);
	dn->puerto_entrada = malloc(MAX_PORT_LEN);
	dn->puerto_master = malloc(MAX_PORT_LEN);
	dn->puerto_filesystem = malloc(MAX_PORT_LEN);
	dn->ruta_databin=malloc(MAX_RUTA_LEN);
	dn->nombre_nodo=malloc(MAX_RUTA_LEN);

	t_config *dnConfig = config_create(ruta);

	strcpy(dn->ip_filesystem, config_get_string_value(dnConfig, "IP_FILESYSTEM"));
	strcpy(dn->puerto_entrada, config_get_string_value(dnConfig, "PUERTO_DATANODE"));
	strcpy(dn->puerto_master, config_get_string_value(dnConfig, "PUERTO_MASTER"));
	strcpy(dn->puerto_filesystem, config_get_string_value(dnConfig, "PUERTO_FILESYSTEM"));
	strcpy(dn->ruta_databin, config_get_string_value(dnConfig, "RUTA_DATABIN"));
	strcpy(dn->nombre_nodo, config_get_string_value(dnConfig, "NOMBRE_NODO"));



	dn->tipo_de_proceso = DATANODE;

	config_destroy(dnConfig);
	return dn;
}
void mostrarConfiguracion(tDataNode *dn){

	printf("Puerto Entrada: %s\n",  dn->puerto_entrada);
	printf("IP Filesystem %s\n",    dn->ip_filesystem);
	printf("Puerto Master: %s\n",       dn->puerto_master);
	printf("Puerto Filesystem: %s\n", dn->puerto_filesystem);
	printf("Ruta Databin: %s\n", dn->ruta_databin);
	printf("Nombre Nodo: %s\n", dn->nombre_nodo);
	printf("Tipo de proceso: %d\n", dn->tipo_de_proceso);
}



bool assertEq(int expected, int actual, const char* errmsg){
	if (expected != actual){
		fprintf(stderr, "%s\n", errmsg);
		fprintf(stderr, "Error. Se esperaba %d, se obtuvo %d\n", expected, actual);
		return false;
	}
	return true;
}

int validarRespuesta(int sock, tHeader h_esp, tHeader *h_obt){

	if ((recv(sock, h_obt, HEAD_SIZE, 0)) == -1){
		perror("Fallo recv de Header. error");
		return FALLO_RECV;
	}

	if (h_esp.tipo_de_proceso != h_obt->tipo_de_proceso){
		printf("Fallo de comunicacion. Se espera un mensaje de %d, se recibio de %d\n",
				h_esp.tipo_de_proceso, h_obt->tipo_de_proceso);
		return FALLO_GRAL;
	}

	if (h_esp.tipo_de_mensaje != h_obt->tipo_de_mensaje){
		printf("Fallo ejecucion de funcion con valor %d\n", h_obt->tipo_de_mensaje);
		return FALLO_GRAL;
	}

	return 0;
}

void freeAndNULL(void **ptr){
	free(*ptr);
	*ptr = NULL;
}

unsigned long fsize(FILE* f){

    fseek(f, 0, SEEK_END);
    unsigned long len = (unsigned long) ftell(f);
    fseek(f, 0, SEEK_SET);
    return len;

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
	head.tipo_de_mensaje = NEW_DN;

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

	free(package);
	return stat;
}
int enviarHeader(int sock_dest,tHeader head){

	int stat;
	char *package;

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

	free(package);
	return stat;
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
		printf("Error al tratar de conectar con FS!\n");
		return FALLO_CONEXION;
	}

	return sock_dest;
}

int main(int argc, char* argv[]){

	if(argc!=2){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}
	int sock_fs;




	datanode=getConfigdn(argv[1]);
	mostrarConfiguracion(datanode);


	puts("conectando con filesystem");
	sock_fs = establecerConexion(datanode->ip_filesystem, datanode->puerto_filesystem);
	if (sock_fs < 0){
		puts("Fallo conexion a FS");
		return -1;
	}
	puts("me conecte  fs");


	tHeader head;
	int stat;
	head.tipo_de_proceso=DATANODE;
	head.tipo_de_mensaje=NEW_DN;

	if((stat=enviarHeader(sock_fs,head))<0){
		puts("No se pudo enviar el header de inicio");
	}

	while((stat = recv(sock_fs, &head, HEAD_SIZE, 0)) > 0){



	}




	return 1;
}
