/*
 * hiloWorkerRedGlobal.c
 *
 *  Created on: 31/10/2017
 *      Author: utnso
 */

#include "funcionesMS.h"
extern char * rutaReductor;
extern double duracionRG;
extern pthread_mutex_t mux_cantFallos;
extern int cantFallos;
void hiloWorkerReduccionGlobal(void *info){
	TatributosHiloReduccionGlobal *atributos = (TatributosHiloReduccionGlobal *)info;


	int sockYama = atributos->sockYama;


	int stat,sockWorker,idTarea;

	char *buffer;
	int packSize;

	bool finCorrecto = false;
	bool finDesconexion=true;

	idTarea=atributos->infoReduccionGlobal.idTarea;

	time_t horaInicio;
	time_t horaFin;
	time(&horaInicio);
	Theader head;

	char *buffInicio = malloc(100);
	log_info(logInfo,"HILO de Reduccion GLOBAL");
	strftime (buffInicio, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&horaInicio));
	log_info(logInfo,"Hora de inicio RG: %s\n", buffInicio);

	log_info(logInfo,"ID tarea%d\n",idTarea);
	printf("hilo RG de la tarea %d",idTarea);

	int i;
	TreduccionGlobal * infoReduccionAEnviar=malloc(sizeof(TreduccionGlobal));
	t_list * listaNodos=list_create();
	for(i=0;i<list_size(atributos->infoReduccionGlobal.listaNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(atributos->infoReduccionGlobal.listaNodos,i);
		log_info(logInfo,"Nombre nodo: %s\n",infoNodo->nombreNodo);
		log_info(logInfo,"ip nodo: %s\n",infoNodo->ipNodo);
		log_info(logInfo,"puerto nodo: %s\n",infoNodo->puertoNodo);
		log_info(logInfo,"temp reduccion: %s\n",infoNodo->temporalReduccion);
		log_info(logInfo,"Nodo elegido: %d\n",infoNodo->nodoEncargado);
		if(infoNodo->nodoEncargado==1){//1--> es el nodo encargado. el unico con ese valor =1. el resto =0.
			if((sockWorker = conectarAServidor(infoNodo->ipNodo,infoNodo->puertoNodo))<0){
				puts("No pudo conectarse a worker");
				head.tipo_de_proceso=MASTER;
				head.tipo_de_mensaje=FIN_REDUCCIONGLOBALFAIL;
				enviarHeaderYValor(head,idTarea,sockYama);
				MUX_LOCK(&mux_cantFallos);
				cantFallos++;
				MUX_UNLOCK(&mux_cantFallos);
				return;
			}
			log_info(logInfo,"Nos conectamos a %s, el encargado de la reduccion global\n",infoNodo->nombreNodo);
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
	head.tipo_de_proceso=MASTER;
	head.tipo_de_mensaje=INICIARREDUCCIONGLOBAL;

	Theader headASerializar;

	packSize=0;
	buffer=serializeInfoReduccionGlobal(head,infoReduccionAEnviar,&packSize);
	log_info(logInfo,"Info de la reduccion global serializado, total %d bytes\n",packSize);
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("No pudo conectarse a worker para la reduccion global. le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONGLOBALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
		return;
	}
	free(buffer);
	list_destroy_and_destroy_elements(infoReduccionAEnviar->listaNodos,limpiarListaNodos);

	free(infoReduccionAEnviar->tempRedGlobal);

	log_info(logInfo,"se enviaron %d bytes de la info de la reduccion global a worker\n",stat);



	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//envio el script de reduccion
	stat=enviarScript(rutaReductor,sockWorker);
	if(stat<0){
		puts("Error al enviar el script reductor");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FIN_REDUCCIONGLOBALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
		return;
	}
	log_info(logInfo,"al while(redu global)");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {

		case(FIN_REDUCCIONGLOBALOK):
		log_info(logInfo,"fin ok rg");
			finCorrecto = true;
		finDesconexion=false;
			close(sockWorker);
			break;
		case(FIN_REDUCCIONGLOBALFAIL):
			puts("worker nos avisa q hubo un error en la redu global");
		log_info(logInfo,"fin rg fail wk");
			finDesconexion=false;
			finCorrecto=false;
			break;
		default:
			break;
		}


	}
	time(&horaFin);
	duracionRG = difftime(horaFin, horaInicio);
	strftime (buffInicio, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&horaFin));
	log_info(logInfo,"Hora de fin RG: %s\n", buffInicio);
	free(buffInicio);

	if(finCorrecto){
		puts("Termina la conexion con worker.. La reduccion Global salio OK. Le avisamos a yama ");
		log_info(logInfo,"Termina la conexion con worker.. La reduccion Global salio OK. Le avisamos a yama ");
		head.tipo_de_proceso=MASTER;
		head.tipo_de_mensaje=FIN_REDUCCIONGLOBALOK;
		enviarHeaderYValor(head,idTarea,sockYama);


	}else if(finDesconexion){
		puts("termino la conexion con worker de manera inesperada. reduccion global fallo. Le avisamos a yama");
		log_info(logInfo,"termino la conexion con worker de manera inesperada. reduccion global fallo. Le avisamos a yama");
		head.tipo_de_proceso=MASTER;
		head.tipo_de_mensaje=FIN_REDUCCIONGLOBALFAIL;
		enviarHeaderYValor(head,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);

	}else{
		puts("reduccion global fallo. Le avisamos a yama");
		log_info(logInfo,"reduccion global fallo. Le avisamos a yama");
		head.tipo_de_proceso=MASTER;
		head.tipo_de_mensaje=FIN_REDUCCIONGLOBALFAIL;
		enviarHeaderYValor(head,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);

	}

	log_info(logInfo,"fin thread de reduccion global del job %d \n",atributos->infoReduccionGlobal.job);
	//free(atributos->infoReduccionGlobal.tempRedGlobal);
	//list_destroy_and_destroy_elements(atributos->infoReduccionGlobal.listaNodos,limpiarListaNodos);
	free(atributos);
}
void limpiarListaNodos(void * info){
	log_info(logInfo,"free lista temp list");
	TinfoNodoReduccionGlobal * infoNodo = (TinfoNodoReduccionGlobal*) info;
	free(infoNodo->ipNodo);
	free(infoNodo->nombreNodo);
	free(infoNodo->puertoNodo);
	free(infoNodo->temporalReduccion);
	free(infoNodo);
	log_info(logInfo,"pase listsa temp list");
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
			log_info(logInfo,"Nombre nodo: %s\n",infoNodo->nombreNodo);
			log_info(logInfo,"ip nodo: %s\n",infoNodo->ipNodo);
			log_info(logInfo,"puerto nodo: %s\n",infoNodo->puertoNodo);
			log_info(logInfo,"temp reduccion: %s\n",infoNodo->temporalReduccion);
			log_info(logInfo,"Nodo elegido: %d\n",infoNodo->nodoEncargado);

		}
	log_info(logInfo,"creo hilo rg \n");

	pthread_t workerReduccionGlobalThread;
	crearHilo(&workerReduccionGlobalThread, (void*)hiloWorkerReduccionGlobal, (void*)atributos);


	return 0;
}
