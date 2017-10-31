/*
 * hiloWorkerRedGlobal.c
 *
 *  Created on: 31/10/2017
 *      Author: utnso
 */

#include "funcionesMS.h"
extern char * rutaReductor;



int conectarseAWorkerParaReduccionGlobal(TreduccionLocal *infoReduccion,int sockYama){


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
	//crearHilo(&workerReduccionLocalThread, (void*)hiloWorkerReduccionLocal, (void*)atributos);


	return 0;
}
