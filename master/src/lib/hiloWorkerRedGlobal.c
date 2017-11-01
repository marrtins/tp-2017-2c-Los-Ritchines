/*
 * hiloWorkerRedGlobal.c
 *
 *  Created on: 31/10/2017
 *      Author: utnso
 */

#include "funcionesMS.h"
extern char * rutaReductor;

void hiloWorkerReduccionGlobal(void *info){
	TatributosHiloReduccionGlobal *atributos = (TatributosHiloReduccionGlobal *)info;


	int sockYama = atributos->sockYama;


	int stat,sockWorker,idTarea;

	char *buffer;
	int packSize;
	Theader *headEnvio = malloc(sizeof(headEnvio));
	bool finCorrecto = false;


	idTarea=atributos->infoReduccionGlobal.idTarea;

	printf("\n\n\n HILO de Reduccion GLOBAL\n");
	printf("ID tarea%d\n",idTarea);

	int i;
	TreduccionGlobal * infoReduccionAEnviar=malloc(sizeof(TreduccionGlobal));
	t_list * listaNodos=list_create();
	for(i=0;i<list_size(atributos->infoReduccionGlobal.listaNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(atributos->infoReduccionGlobal.listaNodos,i);
		printf("Nombre nodo: %s\n",infoNodo->nombreNodo);
		printf("ip nodo: %s\n",infoNodo->ipNodo);
		printf("puerto nodo: %s\n",infoNodo->puertoNodo);
		printf("temp reduccion: %s\n",infoNodo->temporalReduccion);
		printf("Nodo elegido: %d\n",infoNodo->nodoEncargado);
		if(infoNodo->nodoEncargado==1){//1--> es el nodo encargado. el unico con ese valor =1. el resto =0.
			if((sockWorker = conectarAServidor(infoNodo->ipNodo,infoNodo->puertoNodo))<0){
				puts("No pudo conectarse a worker");
				return;
			}
			printf("Nos conectamos a %s, el encargado de la reduccion local\n",infoNodo->nombreNodo);
		}
		list_add(listaNodos,infoNodo);

	}
	infoReduccionAEnviar->idTarea=atributos->infoReduccionGlobal.idTarea;
	infoReduccionAEnviar->job=atributos->infoReduccionGlobal.job;
	infoReduccionAEnviar->tempRedGlobal=malloc(TAMANIO_NOMBRE_TEMPORAL);
	infoReduccionAEnviar->tempRedGlobal=atributos->infoReduccionGlobal.tempRedGlobal;
	infoReduccionAEnviar->tempRedGlobalLen=strlen(infoReduccionAEnviar->tempRedGlobal)+1;
	infoReduccionAEnviar->listaNodos=list_create();
	infoReduccionAEnviar->listaNodos=listaNodos;
	infoReduccionAEnviar->listaNodosSize=list_size(listaNodos);


	//enviamos la misma info que yama nos mando al worker. para que se conecte a todos los nodos.
	Theader head;
	head.tipo_de_proceso=MASTER;
	head.tipo_de_mensaje=INICIARREDUCCIONGLOBAL;


	packSize=0;
	buffer=serializeInfoReduccionGlobal(head,infoReduccionAEnviar,&packSize);
	printf("Info de la reduccion global serializado, total %d bytes\n",packSize);
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la reduccion global. ");
		return;
	}
	printf("se enviaron %d bytes de la info de la reduccion global a worker\n",stat);







	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//envio el script de reduccion
	stat=enviarScript(rutaReductor,sockWorker);
	if(stat<0){
		puts("Error al enviar el script transformador");
	}
	puts("al while");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {

		case(FIN_REDUCCIONGLOBALOK):
			finCorrecto = true;
			close(sockWorker);
			break;
		default:
			break;
		}


	}
	if(finCorrecto){
		puts("Termina la conexion con worker.. La reduccion Global salio OK. Le avisamos a yama ");
		head.tipo_de_proceso=MASTER;
		head.tipo_de_mensaje=FIN_REDUCCIONGLOBALOK;
		enviarHeaderYValor(head,idTarea,sockYama);


	}else{
		puts("termino la conexion con worker de manera inesperada. reduccion global fallo. Le avisamos a yama");
		head.tipo_de_proceso=MASTER;
		head.tipo_de_mensaje=FIN_REDUCCIONGLOBALFAIL;
		enviarHeaderYValor(head,idTarea,sockYama);

	}
	printf("fin thread de reduccion global del job %s \n",atributos->infoReduccionGlobal.job);
}

char *serializarInfoReduccionGlobalMasterWorker2(Theader head,int nombreTemporalReduccionLen,char * nombreTemporalReduccion,t_list * listaTemporales, int *pack_size){

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
int conectarseAWorkerParaReduccionGlobal(TreduccionGlobal *infoReduccion,int sockYama){


	TatributosHiloReduccionGlobal * atributos = malloc(sizeof atributos);
	atributos->infoReduccionGlobal.idTarea=infoReduccion->idTarea;
	atributos->infoReduccionGlobal.job=infoReduccion->job;
	atributos->infoReduccionGlobal.tempRedGlobalLen=infoReduccion->tempRedGlobalLen;
	atributos->infoReduccionGlobal.tempRedGlobal=malloc(TAMANIO_NOMBRE_TEMPORAL);
	atributos->infoReduccionGlobal.tempRedGlobal=infoReduccion->tempRedGlobal;
	atributos->infoReduccionGlobal.listaNodosSize=infoReduccion->listaNodosSize;
	atributos->infoReduccionGlobal.listaNodos=infoReduccion->listaNodos;
	atributos->sockYama=sockYama;

	int i;
	for(i=0;i<list_size(atributos->infoReduccionGlobal.listaNodos);i++){
			TinfoNodoReduccionGlobal *infoNodo = list_get(atributos->infoReduccionGlobal.listaNodos,i);
			printf("Nombre nodo: %s\n",infoNodo->nombreNodo);
			printf("ip nodo: %s\n",infoNodo->ipNodo);
			printf("puerto nodo: %s\n",infoNodo->puertoNodo);
			printf("temp reduccion: %s\n",infoNodo->temporalReduccion);
			printf("Nodo elegido: %d\n",infoNodo->nodoEncargado);

		}
	//printf("creo hilo \n");

	pthread_t workerReduccionGlobalThread;
	crearHilo(&workerReduccionGlobalThread, (void*)hiloWorkerReduccionGlobal, (void*)atributos);


	return 0;
}
