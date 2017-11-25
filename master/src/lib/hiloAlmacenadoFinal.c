/*
 * hiloAlmacenadoFinal.c
 *
 *  Created on: 3/11/2017
 *      Author: utnso
 */


#include "funcionesMS.h"
extern char * rutaResultado;
extern double duracionAlmacenado;
extern pthread_mutex_t mux_cantFallos;
extern int cantFallos;
void hiloWorkerAlmacenamientoFinal(void *info){
	TatributosHiloAlmacenamientoFinal *atributos = (TatributosHiloAlmacenamientoFinal *)info;


	int sockYama = atributos->sockYama;


	int stat,sockWorker,idTarea;

	char *buffer;
	int packSize;

	bool finCorrecto = false;
	bool finDesconexion=true;

	idTarea=atributos->infoAlmacenamiento.idTarea;
	time_t horaInicio;
	time_t horaFin;
	time(&horaInicio);



	log_info(logInfo,"HILO de Almacenamiento Final");
	log_info(logInfo,"ID tarea%d\n",idTarea);

	//printf("hilo almac final tarea %d\n",idTarea);
	printf("Inicio AlmFinal %s",atributos->infoAlmacenamiento.nombreTempReduccion);

	//enviamos la misma info que yama nos mando al worker. para que se conecte a todos los nodos.
	Theader head;
	head.tipo_de_proceso=MASTER;
	head.tipo_de_mensaje=INICIARALMACENAMIENTOFINAL;

	TinfoAlmacenadoMasterWorker *infoMW = malloc(sizeof(TinfoAlmacenadoMasterWorker));
	infoMW->nombreResultante=malloc(strlen(rutaResultado)+1);
	infoMW->nombreResultante=rutaResultado;
	infoMW->nombreResultanteLen=strlen(infoMW->nombreResultante)+1;

	infoMW->nombreTempReduccion=malloc(strlen(atributos->infoAlmacenamiento.nombreTempReduccion)+1);
	infoMW->nombreTempReduccion=atributos->infoAlmacenamiento.nombreTempReduccion;
	infoMW->nombreTempReduccionLen=strlen(infoMW->nombreTempReduccion)+1;

	sockWorker=conectarAServidor(atributos->infoAlmacenamiento.ipNodo,atributos->infoAlmacenamiento.puertoNodo);
	Theader headASerializar;

	packSize=0;
	buffer=serializeInfoAlmacenadoFinalMasterWorker(head,infoMW,&packSize);
	log_info(logInfo,"Info del almacenamiento final serializada. total %d bytes\n",packSize);
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("No pudo conectarse a worker para el almacenado final");
		log_info(logInfo,"no pudo conectarse a wk para el almca final");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
		return;
	}
	log_info(logInfo,"se enviaron %d bytes de la info de la reduccion global a worker\n",stat);
	free(buffer);
	//free(infoMW->nombreResultante);
	//free(infoMW->nombreTempReduccion);
	//free(infoMW);


	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//envio el script de reduccion

	log_info(logInfo,"al while");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {

		case(FIN_ALMACENAMIENTOFINALOK):
			finCorrecto = true;
		finDesconexion=false;
			close(sockWorker);
			break;
		case(FIN_ALMACENAMIENTOFINALFAIL):
			puts("El almacenamiento final falló");
			log_info(logInfo,"wkr nos avisa q finalizo el almac de manera fallida");
			finCorrecto=false;
			finDesconexion=false;
			close(sockWorker);
			break;
		default:
			break;
		}


	}
	time(&horaFin);
	duracionAlmacenado = difftime(horaFin, horaInicio);

	if(finCorrecto){
		puts("Fin AF OK ");
		log_info(logInfo,"termina la conexcion c worker. almacn final ok. aivsamos");
		head.tipo_de_proceso=MASTER;
		head.tipo_de_mensaje=FIN_ALMACENAMIENTOFINALOK;
		enviarHeaderYValor(head,idTarea,sockYama);


	}else if(!finDesconexion){
		puts("Fin AF Fail");
		log_info(logInfo,"almac finalfallo. Le avisamos a yama");
		head.tipo_de_proceso=MASTER;
		head.tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeaderYValor(head,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);

	}else{
		puts("Fin AF Fail, nodo desconectado");
		log_info(logInfo," se termino la conexion de forma inesperada. se cayo worker .almac finalfallo. Le avisamos a yama");
		head.tipo_de_proceso=MASTER;
		head.tipo_de_mensaje=FIN_ALMACENAMIENTOFINALFAIL;
		enviarHeaderYValor(head,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
	}
	log_info(logInfo,"fin thread de almacenamiento final del job %d \n",atributos->infoAlmacenamiento.job);
	log_info(logInfo,"fin conexion c yaama");
	//close(sockYama);
	mostrarMetricasJob();
	if(finCorrecto){
		printf("\n\n\n ####### FIN JOB OK ###### \n\n");

	}
	free(atributos->infoAlmacenamiento.ipNodo);
	free(atributos->infoAlmacenamiento.nombreTempReduccion);
	free(atributos->infoAlmacenamiento.puertoNodo);
	free(atributos);
	exit(1);
}















int conectarseAWorkerParaAlmacenamientoFinal(TinfoAlmacenadoFinal *infoAlmacenamiento,int sockYama){


	TatributosHiloAlmacenamientoFinal * atributos = malloc(sizeof (TatributosHiloAlmacenamientoFinal));
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
