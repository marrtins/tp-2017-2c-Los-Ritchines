/*
 * hiloWorkerRedLocal.c
 *
 *  Created on: 27/10/2017
 *      Author: utnso
 */

#include "funcionesMS.h"
extern char * rutaReductor;
extern pthread_mutex_t mux_cantFallos;
extern int cantFallos;

void hiloWorkerReduccionLocal(void *info){
	TatributosHiloReduccionLocal *atributos = (TatributosHiloReduccionLocal *)info;


	int sockYama = atributos->sockYama;


	int stat,sockWorker,idTarea;
	char *buffer;
	int packSize;
	bool finCorrecto = false;
	bool finDesconexion=true;

	idTarea=atributos->infoReduccion.idTarea;

	time_t horaInicio;
	time_t horaFin;
	time(&horaInicio);
	inicioEjecucionRL(idTarea);

	printf("hilo reductor del %s. id tarea %d",atributos->infoReduccion.nombreNodo,idTarea);

	log_info(logInfo,"HILO REDUCTOR del %s",atributos->infoReduccion.nombreNodo);
	log_info(logInfo,"ID tarea%d\n",idTarea);
	log_info(logInfo,"Nombre nodo %s\n ip nodo %s\n puerto nodo %s\n Nombre tempored %s \n",
			atributos->infoReduccion.nombreNodo,atributos->infoReduccion.ipNodo,atributos->infoReduccion.puertoNodo,
			atributos->infoReduccion.tempRed);


	int i;
	for(i=0;i<list_size(atributos->infoReduccion.listaTemporalesTransformacion);i++){
		TreduccionLista *infoAux = list_get(atributos->infoReduccion.listaTemporalesTransformacion,i);
		log_info(logInfo," nombre temp transformacion: %s \n",infoAux->nombreTemporal);
	}

	Theader headASerializar;

	if((sockWorker = conectarAServidor(atributos->infoReduccion.ipNodo, atributos->infoReduccion.puertoNodo))<0){
		puts("No pudo conectarse a worker. para la reduccion local le avisamos a yama");
		log_info(logInfo,"No pudo conectarse a worker. para la reduccion local le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONLOCALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
		return;
	}

	log_info(logInfo,"Nos conectamos a worker");

	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//enviarHeader(sockWorker,headEnvio);

	//Envio al worker el nro de bloque, el tamaño y el nombre temporal
	headASerializar.tipo_de_mensaje=INICIARREDUCCIONLOCAL;
	headASerializar.tipo_de_proceso=MASTER;
	packSize=0;
	log_info(logInfo,"LIST SIZE: %d",list_size(atributos->infoReduccion.listaTemporalesTransformacion));
	buffer = serializarInfoReduccionLocalMasterWorker(headASerializar,atributos->infoReduccion.tempRedLen,atributos->infoReduccion.tempRed,atributos->infoReduccion.listaTemporalesTransformacion,&packSize);

	log_info(logInfo,"Info de la reduccion local serializada, enviamos\n");
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la trasnformacion");
		log_info(logInfo,"no se pudo enviar info de la trasnformacion");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONLOCALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
		return;
	}
	free(buffer);
	log_info(logInfo,"se enviaron %d bytes de la info de la reduccion\n",stat);



	//envio el script de reduccion
	stat=enviarScript(rutaReductor,sockWorker);
	if(stat<0){
		puts("Error al enviar el script transformador");
		log_info(logInfo,"error al enviar el script transfo. fin RL");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONLOCALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
		return;

	}
	log_info(logInfo,"al while");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {

		case(FIN_REDUCCIONLOCALOK):
		log_info(logInfo,"fin RL OK");
		puts("worker avisa q la rl salio ok");
		finCorrecto = true;
		finDesconexion=false;
			close(sockWorker);
			break;
		case(FIN_REDUCCIONLOCALFAIL):
		log_info(logInfo,"fin RL FAIL");
					finCorrecto=false;
			finDesconexion=false;
			puts("worker nos avisa q la RL fallo.");
			break;
		default:
			break;
		}


	}
	time(&horaFin);
	double diferencia = difftime(horaFin, horaInicio);

	if(finCorrecto){
		puts("Termina la conexion con worker.. La reduccion local salio OK. Le avisamos a yama ");
		log_info(logInfo,"Termina la conexion con worker.. La reduccion local salio OK. Le avisamos a yama ");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONLOCALOK;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		finEjecucionRL(idTarea,diferencia);


	}else if(finDesconexion){
		puts("termino la conexion con worker de manera inesperada. reduccion local fallo. Le avisamos a yama");
		log_info(logInfo,"termino la conexion con worker de manera inesperada. reduccion local fallo. Le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONLOCALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
	}else{
		puts("reduccion local fallo. Le avisamos a yama");
		log_info(logInfo,"reduccion local fallo. Le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONLOCALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
	}
	log_info(logInfo,"fin thread de reduccion local del nodo %s \n",atributos->infoReduccion.nombreNodo);
	free(atributos->infoReduccion.ipNodo);
	free(atributos->infoReduccion.nombreNodo);
	free(atributos->infoReduccion.puertoNodo);
	free(atributos->infoReduccion.tempRed);
	list_destroy_and_destroy_elements(atributos->infoReduccion.listaTemporalesTransformacion,limpiarTemporales);
	free(atributos);
}

void limpiarTemporales(void * info){
	log_info(logInfo,"free lista temp list");
	TreduccionLista * infoLista = (TreduccionLista*) info;
	free(infoLista->nombreTemporal);
	free(infoLista);
	log_info(logInfo,"pase listsa temp list");
}


int conectarseAWorkerParaReduccionLocal(TreduccionLocal *infoReduccion,int sockYama){


	TatributosHiloReduccionLocal * atributos = malloc(sizeof (TatributosHiloReduccionLocal));
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
