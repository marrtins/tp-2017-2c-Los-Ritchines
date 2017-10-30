/*
 * hiloWorkerRedLocal.c
 *
 *  Created on: 27/10/2017
 *      Author: utnso
 */

#include "funcionesMS.h"
extern char * rutaReductor;

char *serializarInfoReduccionLocalMasterWorker2(Theader head,int nombreTemporalReduccionLen,char * nombreTemporalReduccion,t_list * listaTemporales, int *pack_size){

	char *bytes_serial;

	int i;
	int espacioPackSize = sizeof(int);
	int espacioListSize = sizeof(int);
	int espacioNombreTemporalLen = sizeof(int);
	int sizeLista = list_size(listaTemporales);

	int espaciosVariables=0;
	for(i=0;i< sizeLista;i++){
		TreduccionLista * aux = list_get(listaTemporales,i);
		espaciosVariables += aux->nombreTemporalLen;
	}
	espaciosVariables += sizeof(int)*sizeLista;
	int espacioAMallocar = HEAD_SIZE + espacioPackSize+espacioListSize+espacioNombreTemporalLen+nombreTemporalReduccionLen+espaciosVariables;
	printf("Espacio a mallocar: %d\n",espacioAMallocar);

	if ((bytes_serial = malloc(espacioAMallocar)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}


	*pack_size = 0;
	memcpy(bytes_serial + *pack_size, &head, HEAD_SIZE);
	*pack_size += HEAD_SIZE;

	// hacemos lugar para el payload_size
	*pack_size += sizeof(int);

	memcpy(bytes_serial + *pack_size, &nombreTemporalReduccionLen, sizeof (int));
	*pack_size += sizeof (int);
	memcpy(bytes_serial + *pack_size, nombreTemporalReduccion, nombreTemporalReduccionLen);
	*pack_size += nombreTemporalReduccionLen;


	memcpy(bytes_serial + *pack_size, &sizeLista, sizeof (int));
	*pack_size += sizeof (int);

	printf("EN COMPARTIDAS; SIZE LISTA: %d\n",sizeLista);
	for(i=0;i<sizeLista;i++){
		TreduccionLista * aux = list_get(listaTemporales,i);
		memcpy(bytes_serial + *pack_size, &aux->nombreTemporalLen, sizeof(int));
		*pack_size += sizeof(int);
		memcpy(bytes_serial + *pack_size, aux->nombreTemporal, aux->nombreTemporalLen);
		*pack_size += aux->nombreTemporalLen;
	}


	memcpy(bytes_serial + HEAD_SIZE, pack_size, sizeof(int));

	printf("Pack size:serializarInfoReduccionLocalMasterWorker2 %d\n",*pack_size);
	return bytes_serial;
}
void hiloWorkerReduccionLocal(void *info){
	TatributosHiloReduccionLocal *atributos = (TatributosHiloReduccionLocal *)info;


	int sockYama = atributos->sockYama;


	int stat,sockWorker,idTarea;
//	int fdTransformador;
//	int len,remain_data,sent_bytes;
//	off_t offset;
//	struct stat file_stat;
//	char file_size[sizeof(int)];
	char *buffer;
	int packSize;
	Theader *headEnvio = malloc(sizeof(headEnvio));
	bool finCorrecto = false;


	idTarea=atributos->infoReduccion.idTarea;

	printf("\n\n\n HILO REDUCTOR del %s\n",atributos->infoReduccion.nombreNodo);
	printf("ID tarea%d\n",idTarea);
	printf("Nombre nodo %s\n ip nodo %s\n puerto nodo %s\n Nombre tempored %s \n",
			atributos->infoReduccion.nombreNodo,atributos->infoReduccion.ipNodo,atributos->infoReduccion.puertoNodo,
			atributos->infoReduccion.tempRed);


	int i;
	for(i=0;i<list_size(atributos->infoReduccion.listaTemporalesTransformacion);i++){
		TreduccionLista *infoAux = list_get(atributos->infoReduccion.listaTemporalesTransformacion,i);
		printf(" nombre temp transformacion: %s \n",infoAux->nombreTemporal);
	}


	if((sockWorker = conectarAServidor(atributos->infoReduccion.ipNodo, atributos->infoReduccion.puertoNodo))<0){
		puts("No pudo conectarse a worker");
		return;
	}

	puts("Nos conectamos a worker");

	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//enviarHeader(sockWorker,headEnvio);

	//Envio al worker el nro de bloque, el tamaño y el nombre temporal
	Theader headASerializar;
	headASerializar.tipo_de_mensaje=INICIARREDUCCIONLOCAL;
	headASerializar.tipo_de_proceso=MASTER;
	packSize=0;
	printf("LIST SIZE: %d",list_size(atributos->infoReduccion.listaTemporalesTransformacion));
	buffer = serializarInfoReduccionLocalMasterWorker2(headASerializar,atributos->infoReduccion.tempRedLen,atributos->infoReduccion.tempRed,atributos->infoReduccion.listaTemporalesTransformacion,&packSize);

//	printf("Info de la reduccion local serializada, enviamos\n");
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la trasnformacion");
		return;
	}
//	printf("se enviaron %d bytes de la info de la reduccion\n",stat);



	//envio el script de reduccion
	stat=enviarScript(rutaReductor,sockWorker);
	if(stat<0){
		puts("Error al enviar el script transformador");
	}
	puts("al while");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {

		case(FIN_REDUCCIONLOCALOK):
			finCorrecto = true;
			close(sockWorker);
			break;
		default:
			break;
		}


	}
	if(finCorrecto){
		puts("Termina la conexion con worker.. La reduccion local salio OK. Le avisamos a yama ");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONLOCALOK;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);


	}else{
		puts("termino la conexion con worker de manera inesperada. reduccion local fallo. Le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONLOCALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);

	}
	printf("fin thread de reduccion local del nodo %s \n",atributos->infoReduccion.nombreNodo);
}


int conectarseAWorkerParaReduccionLocal(TreduccionLocal *infoReduccion,int sockYama){


	TatributosHiloReduccionLocal * atributos = malloc(sizeof atributos);
	atributos->infoReduccion.idTarea=infoReduccion->idTarea;
	atributos->infoReduccion.ipLen=infoReduccion->ipLen;
	atributos->infoReduccion.ipNodo=malloc(MAXIMA_LONGITUD_IP);
	atributos->infoReduccion.ipNodo=infoReduccion->ipNodo;
	atributos->infoReduccion.job=infoReduccion->job;
	atributos->infoReduccion.nombreNodoLen=infoReduccion->nombreNodoLen;
	atributos->infoReduccion.nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
	atributos->infoReduccion.nombreNodo=infoReduccion->nombreNodo;
	atributos->infoReduccion.puertoLen=infoReduccion->puertoLen;
	atributos->infoReduccion.puertoNodo=malloc(MAXIMA_LONGITUD_PUERTO);
	atributos->infoReduccion.puertoNodo=infoReduccion->puertoNodo;
	atributos->infoReduccion.tempRedLen=infoReduccion->tempRedLen;
	atributos->infoReduccion.tempRed=malloc(TAMANIO_NOMBRE_TEMPORAL);
	atributos->infoReduccion.tempRed=infoReduccion->tempRed;
	atributos->infoReduccion.listaSize=infoReduccion->listaSize;
	atributos->infoReduccion.listaTemporalesTransformacion=infoReduccion->listaTemporalesTransformacion;
	atributos->sockYama=sockYama;

	//printf("creo hilo \n");

	pthread_t workerReduccionLocalThread;
	crearHilo(&workerReduccionLocalThread, (void*)hiloWorkerReduccionLocal, (void*)atributos);


	return 0;
}
