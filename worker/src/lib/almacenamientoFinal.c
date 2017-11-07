/*
 * almacenamientoFinal.c
 *
 *  Created on: 3/11/2017
 *      Author: utnso
 */


#include "funcionesWK.h"


int realizarAlmacenamientoFinal(client_sock){


	char * buffer;



	TinfoAlmacenadoMasterWorker *infoAlmacenado;
	if ((buffer = recvGenericWFlags(client_sock,MSG_WAITALL)) == NULL){
		puts("Fallo recepcion de INFOBLOQUE");
		return FALLO_RECV;
	}

	if ((infoAlmacenado = deserializeInfoAlmacenadoMasterWorker(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del deserializar info reduccion local");
		return FALLO_GRAL;
	}
	printf("llego la info apra almacenamientofinal\n");
	printf("nombre resultante %s\n tempred %s\n",infoAlmacenado->nombreResultante,infoAlmacenado->nombreTempReduccion);



//puts aca envio a filesystem

	sleep(1);


	Theader *headEnvio=malloc(sizeof(Theader));

	headEnvio->tipo_de_proceso=WORKER;
	headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALOK;
	enviarHeader(client_sock,headEnvio);

	puts("fin almacenamiento global. envie header");

	return 0;
}
