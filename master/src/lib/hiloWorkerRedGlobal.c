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
			printf("Nos conectamos a %s, el encargado de la reduccion global\n",infoNodo->nombreNodo);
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
		puts("Error al enviar el script reductor");
	}
	puts("al while(redu global)");

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
	printf("fin thread de reduccion global del job %d \n",atributos->infoReduccionGlobal.job);
}


int conectarseAWorkerParaReduccionGlobal(TreduccionGlobal *infoReduccion,int sockYama){


	TatributosHiloReduccionGlobal * atributos = malloc(sizeof (TatributosHiloReduccionGlobal));
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
