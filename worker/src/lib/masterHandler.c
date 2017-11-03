/*
 * masterHandler.c
 *
 *  Created on: 2/11/2017
 *      Author: utnso
 */


#include "funcionesWK.h"


int manejarConexionMaster(Theader *head, int client_sock){


	int stat;
	Theader *headEnvio  = malloc(sizeof headEnvio);


	if(head->tipo_de_mensaje==NUEVATRANSFORMACION){

		if((stat = realizarTransformacion(client_sock))<0){
			puts("Fallo en la transformacion");
		}

	}else if(head->tipo_de_mensaje==INICIARREDUCCIONLOCAL){

		if((stat = realizarReduccionLocal(client_sock))<0){
			puts("Fallo en la reduccion local");
		}

	}else if(head->tipo_de_mensaje==INICIARREDUCCIONGLOBAL){
		if((stat = realizarReduccionGlobal(client_sock))<0){
			puts("Fallo en la reduccion global");
		}
	}

	//free(headEnvio);

	return 0;

}
