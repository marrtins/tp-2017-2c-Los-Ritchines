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
#include "../compartidas/compartidas.c"

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


