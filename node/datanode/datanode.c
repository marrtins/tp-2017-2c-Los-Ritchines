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
#include <stdio.h>


#define BACKLOG 20

tDataNode *datanode;

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


	//Enviamos el Header De inicio.
	tHeader head;
	int stat;
	head.tipo_de_proceso=DATANODE;
	head.tipo_de_mensaje=NEW_DN;

	if((stat=enviarHeader(sock_fs,head))<0){
		puts("No se pudo enviar el header de inicio");
	}

	//Enviamos info sobre el Worker Asociado al datanode.


	//enviamos el path del archivo a reducir

	int pack_size;	char *buffer;


	head.tipo_de_mensaje = INFO_WORKER ; pack_size = 0;

	int puertoWorkerSize = (strlen(datanode->puerto_worker));
	int ipWorkerSize = (strlen(datanode->ip_nodo));

	buffer=serializeDosChar(head,datanode->ip_filesystem,ipWorkerSize,datanode->puerto_worker,puertoWorkerSize,&pack_size);

	puts("IP y Puerto serializados");

	puts("enviando ip y puertos");

	if ((stat = send(sock_fs, buffer, pack_size, 0)) == -1){
		puts("no se pudo enviar ip y puerto a fs ");
		return  FALLO_SEND;
	}

	printf("se enviaron %d bytes del ip y puerto a FS\n",stat);


	while((stat = recv(sock_fs, &head, HEAD_SIZE, 0)) > 0){



	}




	return 1;
}







tDataNode *getConfigdn(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tDataNode *dn = malloc(sizeof(tDataNode));

	dn->ip_filesystem       =    malloc(MAX_IP_LEN);
	dn->ip_nodo       =    malloc(MAX_IP_LEN);
	dn->puerto_entrada = malloc(MAX_PORT_LEN);
	dn->puerto_master = malloc(MAX_PORT_LEN);
	dn->puerto_worker = malloc(MAX_PORT_LEN);
	dn->puerto_filesystem = malloc(MAX_PORT_LEN);
	dn->ruta_databin=malloc(MAX_RUTA_LEN);
	dn->nombre_nodo=malloc(MAX_RUTA_LEN);

	t_config *dnConfig = config_create(ruta);

	strcpy(dn->ip_filesystem, config_get_string_value(dnConfig, "IP_FILESYSTEM"));
	strcpy(dn->ip_nodo, config_get_string_value(dnConfig, "IP_NODO"));
	strcpy(dn->puerto_entrada, config_get_string_value(dnConfig, "PUERTO_DATANODE"));
	strcpy(dn->puerto_worker, config_get_string_value(dnConfig, "PUERTO_WORKER"));
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
	printf("IP Nodo %s\n",    dn->ip_nodo);
	printf("Puerto Master: %s\n",       dn->puerto_master);
	printf("Puerto Filesystem: %s\n", dn->puerto_filesystem);
	printf("Puerto Worker: %s\n", dn->puerto_worker);
	printf("Ruta Databin: %s\n", dn->ruta_databin);
	printf("Nombre Nodo: %s\n", dn->nombre_nodo);
	printf("Tipo de proceso: %d\n", dn->tipo_de_proceso);
}




