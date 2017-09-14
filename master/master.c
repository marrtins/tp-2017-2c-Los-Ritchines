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
#include <commons/string.h>
tMaster *master;
int sock_yama;

int main(int argc, char* argv[]){

	int stat;

	if(argc!=6){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}

	 char *configPath = string_new();
	 configPath=argv[1];
	 char *transformadorPath = string_new();
	 transformadorPath=argv[2];
	 char *reductorPath = string_new();
	 reductorPath=argv[3];
	 char *archivoAReducirPath = string_new();
	 archivoAReducirPath=argv[4];
	 char *resultadoPath = string_new();
	 resultadoPath=argv[5];


	master=getConfigMaster(configPath);
	mostrarConfiguracion(master);

	printf("Transformador Path: %s\n",transformadorPath);
	printf("Reductor Path: %s\n",reductorPath);
	printf("Archivo a reducir Path: %s\n",archivoAReducirPath);
	printf("Resultado Path: %s\n",resultadoPath);


	if ((stat = conectarAYama()) < 0){
		puts("No se pudo conectar con Yama!");
		return 0;
	}




	puts("Conectado a yama.. Inicio ejecucion..");
	tHeader head_tmp;
	int pack_size;
	char *buffer;



	//envio el script TRANSFORMADOR
	tPackSrcCode *src_transformador = readFileIntoPack(MASTER, transformadorPath);
	puts("codigo fuente creado de TRANSFORMADOR");
	head_tmp.tipo_de_proceso = MASTER; head_tmp.tipo_de_mensaje = SRC_CODE_TRANSF; pack_size = 0;
	buffer = serializeBytes(head_tmp, src_transformador->bytes, src_transformador->bytelen, &pack_size);

	puts("codigo fuente de TRANSFORMADOR serializado");

	puts("enviando codigo fuente de TRANSFORMADOR");

	if ((stat = send(sock_yama, buffer, pack_size, 0)) == -1){
		puts("no se pudo enviar codigo fuente de transformador a YAMA. ");
		return  FALLO_SEND;
	}

	printf("se enviaron %d bytes del codigo fuente de transformador a YAMA\n",stat);

	//envio el script REDUCTOR
	tPackSrcCode *src_reductor = readFileIntoPack(MASTER, reductorPath);
	puts("codigo fuente creado de REDUCTOR");
	head_tmp.tipo_de_proceso = MASTER; head_tmp.tipo_de_mensaje = SRC_CODE_RED; pack_size = 0;
	buffer = serializeBytes(head_tmp, src_reductor->bytes, src_reductor->bytelen, &pack_size);

	puts("codigo fuente de REDUCTOR serializado");

	puts("enviando codigo fuente de REDUCTOR");

	if ((stat = send(sock_yama, buffer, pack_size, 0)) == -1){
		puts("no se pudo enviar codigo fuente de REDUCTOR a YAMA. ");
		return  FALLO_SEND;
	}

	printf("se enviaron %d bytes del codigo fuente de REDUCTOR a YAMA\n",stat);

	// enviamos ambos scripts a yama del codigo fuente, lo liberamos ahora antes de olvidarnos..
	freeAndNULL((void **) &src_reductor->bytes);
	freeAndNULL((void **) &src_reductor);
	freeAndNULL((void **) &src_transformador->bytes);
	freeAndNULL((void **) &src_transformador);

	//enviamos el path del archivo a reducir

	head_tmp.tipo_de_proceso = MASTER; head_tmp.tipo_de_mensaje = PATH_FILE_TOREDUCE ;pack_size = 0;

	buffer=serializeBytes(head_tmp,archivoAReducirPath,(strlen(archivoAReducirPath)+1),&pack_size);

	puts("Path del archivo a reducir serializado");

	puts("enviando Path del archivo a reducir");

	if ((stat = send(sock_yama, buffer, pack_size, 0)) == -1){
		puts("no se pudo enviar Path del archivo a reducir a YAMA. ");
		return  FALLO_SEND;
	}

	printf("se enviaron %d bytes del Path del archivo a reducir a YAMA\n",stat);
	//enviamos el path del resultado

	head_tmp.tipo_de_proceso = MASTER; head_tmp.tipo_de_mensaje = PATH_RES_FILE ;pack_size = 0;

	buffer=serializeBytes(head_tmp,resultadoPath,(strlen(resultadoPath)+1),&pack_size);

	puts("Path del resultado serializado");

	puts("enviando Path del resultado");

	if ((stat = send(sock_yama, buffer, pack_size, 0)) == -1){
		puts("no se pudo enviar Path del aresultado a YAMA. ");
		return  FALLO_SEND;
	}

	printf("se enviaron %d bytes del Path del resultado a YAMA\n",stat);

	freeAndNULL((void **) &buffer);



	while(1);

	return 0;
}


int conectarAYama(){

	int stat;


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


