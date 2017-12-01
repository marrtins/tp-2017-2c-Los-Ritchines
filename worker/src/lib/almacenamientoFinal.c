/*
 * almacenamientoFinal.c
 *
 *  Created on: 3/11/2017
 *      Author: utnso
 */


#include "funcionesWK.h"
extern Tworker *worker;

int realizarAlmacenamientoFinal(client_sock){


	char * buffer;
	char * archivoFinalMapeado;
	char * contenidoArchivoFinal;
	int fileDescriptorArchivoFinal;
	Theader * head = malloc(sizeof(Theader));

	int stat;
	unsigned long long tamanioArchivoFinal;
	Theader * header = malloc(sizeof(Theader));
	FILE * archivoFinal;
	Tbuffer * tbuffer;
	header->tipo_de_proceso = WORKER;
	header->tipo_de_mensaje = ALMACENAR_ARCHIVO;
	Theader *headEnvio=malloc(sizeof(Theader));
	TinfoAlmacenadoMasterWorker *infoAlmacenado;
	if ((buffer = recvGenericWFlags(client_sock,MSG_WAITALL)) == NULL){
		puts("Fallo recepcion de INFOBLOQUE");

		headEnvio->tipo_de_proceso=WORKER;
		headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeader(client_sock,headEnvio);
		return FALLO_RECV;
	}

	if ((infoAlmacenado = deserializeInfoAlmacenadoMasterWorker(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del deserializar info reduccion local");

		headEnvio->tipo_de_proceso=WORKER;
		headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeader(client_sock,headEnvio);
		return FALLO_GRAL;
	}

	free(buffer);
	//printf("llego la info apra almacenamientofinal\n");
	printf("Inicio almacenamiento final %s\n",infoAlmacenado->nombreResultante);
	log_info(logInfo,"llego la info para almac final");
	log_info(logInfo,"nombre resultante %s\n tempred %s\n",infoAlmacenado->nombreResultante,infoAlmacenado->nombreTempReduccion);





	log_info(logInfo,"me conecto a fs");
	int sockFS=conectarAServidor(worker->ip_filesystem,worker->puerto_filesystem);
	log_info(logInfo,"sock fs:%d\n",sockFS);
	if(sockFS<0){
		puts("fallo al enviar a fs el resultante");
		headEnvio->tipo_de_proceso=WORKER;
		headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeader(client_sock,headEnvio);
		return -1;
	}

	log_info(logInfo,"nombre temp reduccion %s ",infoAlmacenado->nombreTempReduccion);
	archivoFinal = fopen(infoAlmacenado->nombreTempReduccion, "r");
	tamanioArchivoFinal = tamanioArchivo(archivoFinal);
	fileDescriptorArchivoFinal = fileno(archivoFinal);
	contenidoArchivoFinal = malloc(tamanioArchivoFinal);
	if ((archivoFinalMapeado = mmap(NULL, tamanioArchivoFinal, PROT_READ, MAP_SHARED,	fileDescriptorArchivoFinal, 0)) == MAP_FAILED) {
		puts("error de almacenamiento. fallo mmap");
		headEnvio->tipo_de_proceso=WORKER;
		headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeader(client_sock,headEnvio);
		return -1;
		//logErrorAndExit("Error al hacer mmap");
	}

	memcpy(contenidoArchivoFinal, archivoFinalMapeado, tamanioArchivoFinal);


	//yamafs
	tbuffer = empaquetarArchivoFinal(header, infoAlmacenado->nombreResultante, contenidoArchivoFinal, tamanioArchivoFinal);

	if (send(sockFS, tbuffer->buffer , tbuffer->tamanio, 0) == -1){
		puts("fallo al enviar a fs el resultante");
		headEnvio->tipo_de_proceso=WORKER;
		headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeader(client_sock,headEnvio);
		return -1;

		//logErrorAndExit("Fallo al enviar a Nodo el bloque a almacenar");

	}

	munmap(archivoFinalMapeado, tamanioArchivoFinal);
	log_info(logInfo,"mande a fs ");
	close(fileDescriptorArchivoFinal);
	fclose(archivoFinal);
	free(tbuffer->buffer);
	free(tbuffer);
	free(contenidoArchivoFinal);




//puts aca envio a filesystem

	//sleep(2);


	//todo: que fs me envie un header diciendo si salio ok


	if((stat=recv(sockFS,head,sizeof(Theader),0))<0){
		puts("error al recibir rta de fs");
		headEnvio->tipo_de_proceso=WORKER;
		headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeader(client_sock,headEnvio);
		return -1;
	}
	log_info(logInfo,"llego la rta de FS");

	if(head->tipo_de_proceso==FILESYSTEM && head->tipo_de_mensaje==FIN_ALMACENAMIENTOFINALOK){
		puts("Fin AlmFi ok");
		log_info(logInfo,"fin almac final ok");
		headEnvio->tipo_de_proceso=WORKER;
		headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALOK;
		enviarHeader(client_sock,headEnvio);

	}else{
		puts("fin lmacenamiento final fail");
		log_info(logInfo,"fin lmacenamiento final fail");
		headEnvio->tipo_de_proceso=WORKER;
		headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeader(client_sock,headEnvio);

	}


	free(head);
	free(header);
	free(headEnvio);
	//log_info(logInfo,"32");
	free(infoAlmacenado->nombreResultante);
	//log_info(logInfo,"33");
	free(infoAlmacenado->nombreTempReduccion);
	//log_info(logInfo,"34");
	free(infoAlmacenado);
	//log_info(logInfo,"35");


	close(sockFS);



	return 0;
}
