/*
 * hiloAlmacenadoFinal.c
 *
 *  Created on: 3/11/2017
 *      Author: utnso
 */


#include "funcionesMS.h"
extern char * rutaResultado;
void hiloWorkerAlmacenamientoFinal(void *info){
	TatributosHiloAlmacenamientoFinal *atributos = (TatributosHiloAlmacenamientoFinal *)info;


	int sockYama = atributos->sockYama;


	int stat,sockWorker,idTarea;

	char *buffer;
	int packSize;
	Theader *headEnvio = malloc(sizeof(headEnvio));
	bool finCorrecto = false;


	idTarea=atributos->infoAlmacenamiento.idTarea;

	printf("\n\n\n HILO de Almacenamiento FinaL\n");
	printf("ID tarea%d\n",idTarea);


	//enviamos la misma info que yama nos mando al worker. para que se conecte a todos los nodos.
	Theader head;
	head.tipo_de_proceso=MASTER;
	head.tipo_de_mensaje=INICIARALMACENAMIENTOFINAL;

	TinfoAlmacenadoMasterWorker *infoMW = malloc(sizeof(infoMW));
	infoMW->nombreResultante=malloc(TAMANIO_NOMBRE_TEMPORAL); //todo:tamanio nombre archivo
	infoMW->nombreResultante=rutaResultado;
	infoMW->nombreResultanteLen=strlen(infoMW->nombreResultante)+1;

	infoMW->nombreTempReduccion=malloc(TAMANIO_NOMBRE_TEMPORAL); //todo:tamanio nombre archivo
	infoMW->nombreTempReduccion=atributos->infoAlmacenamiento.nombreTempReduccion;
	infoMW->nombreTempReduccionLen=strlen(infoMW->nombreTempReduccion)+1;

	sockWorker=conectarAServidor(atributos->infoAlmacenamiento.ipNodo,atributos->infoAlmacenamiento.puertoNodo);

	packSize=0;
	buffer=serializeInfoAlmacenadoFinalMasterWorker(head,infoMW,&packSize);
	printf("Info del almacenamiento final serializada. total %d bytes\n",packSize);
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la reduccion global. ");
		return;
	}
	printf("se enviaron %d bytes de la info de la reduccion global a worker\n",stat);



	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//envio el script de reduccion

	puts("al while");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {

		case(FIN_ALMACENAMIENTOFINALOK):
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
		head.tipo_de_mensaje=FIN_ALMACENAMIENTOFINALOK;
		enviarHeaderYValor(head,idTarea,sockYama);


	}else{
		puts("termino la conexion con worker de manera inesperada. reduccion global fallo. Le avisamos a yama");
		head.tipo_de_proceso=MASTER;
		head.tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeaderYValor(head,idTarea,sockYama);

	}
	printf("fin thread de almacenamiento final del job %d \n",atributos->infoAlmacenamiento.job);
}















int conectarseAWorkerParaAlmacenamientoFinal(TinfoAlmacenadoFinal *infoAlmacenamiento,int sockYama){


	TatributosHiloAlmacenamientoFinal * atributos = malloc(sizeof atributos);
	atributos->infoAlmacenamiento.idTarea=infoAlmacenamiento->idTarea;
	atributos->infoAlmacenamiento.job=infoAlmacenamiento->job;

	atributos->infoAlmacenamiento.nombreTempReduccionLen=infoAlmacenamiento->nombreTempReduccionLen;
	atributos->infoAlmacenamiento.nombreTempReduccion=malloc(TAMANIO_NOMBRE_TEMPORAL);
	atributos->infoAlmacenamiento.nombreTempReduccion=infoAlmacenamiento->nombreTempReduccion;

	atributos->infoAlmacenamiento.ipNodoLen=infoAlmacenamiento->ipNodoLen;
	atributos->infoAlmacenamiento.ipNodo=malloc(MAXIMA_LONGITUD_IP);
	atributos->infoAlmacenamiento.ipNodo=infoAlmacenamiento->ipNodo;

	atributos->infoAlmacenamiento.puertoNodoLen=infoAlmacenamiento->puertoNodoLen;
	atributos->infoAlmacenamiento.puertoNodo=malloc(MAXIMA_LONGITUD_PUERTO);
	atributos->infoAlmacenamiento.puertoNodo=infoAlmacenamiento->puertoNodo;



	atributos->sockYama=sockYama;


	//printf("creo hilo \n");

	pthread_t workerAlmacenamientoFinalThread;
	crearHilo(&workerAlmacenamientoFinalThread, (void*)hiloWorkerAlmacenamientoFinal, (void*)atributos);


	return 0;
}
