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
	unsigned long long tamanioArchivoFinal;
	Theader * header = malloc(sizeof(Theader));
	FILE * archivoFinal;
	Tbuffer * tbuffer;
	header->tipo_de_proceso = WORKER;
	header->tipo_de_mensaje = ALMACENAR_ARCHIVO;

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





	puts("me conecto a fs");
	int sockFS=conectarAServidor(worker->ip_filesystem,worker->puerto_filesystem);
	printf("sock fs:%d\n",sockFS);


	archivoFinal = fopen(infoAlmacenado->nombreTempReduccion, "r");
	tamanioArchivoFinal = tamanioArchivo(archivoFinal);
	fileDescriptorArchivoFinal = fileno(archivoFinal);
	contenidoArchivoFinal = malloc(tamanioArchivoFinal);
	if ((archivoFinalMapeado = mmap(NULL, tamanioArchivoFinal, PROT_READ, MAP_SHARED,	fileDescriptorArchivoFinal, 0)) == MAP_FAILED) {
		logAndExit("Error al hacer mmap");
	}

	memcpy(contenidoArchivoFinal, archivoFinalMapeado, tamanioArchivoFinal);

	//yamafs
	tbuffer = empaquetarArchivoFinal(header, infoAlmacenado->nombreResultante, contenidoArchivoFinal, tamanioArchivoFinal);

	if (send(sockFS, tbuffer->buffer , tbuffer->tamanio, 0) == -1){
		logAndExit("Fallo al enviar a Nodo el bloque a almacenar");
	}

	munmap(archivoFinalMapeado, infoAlmacenado->nombreTempReduccionLen);

	close(fileDescriptorArchivoFinal);
	fclose(archivoFinal);
	free(tbuffer->buffer);
	free(tbuffer);
	free(contenidoArchivoFinal);


//puts aca envio a filesystem

	sleep(2);


	Theader *headEnvio=malloc(sizeof(Theader));

	headEnvio->tipo_de_proceso=WORKER;
	headEnvio->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALOK;
	enviarHeader(client_sock,headEnvio);

	puts("fin almacenamiento global. envie header");

	return 0;
}
