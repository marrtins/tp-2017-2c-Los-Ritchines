/*
 * reduccionGlobal.c
 *
 *  Created on: 9/11/2017
 *      Author: utnso
 */

#include "funcionesYM.h"

extern t_list * listaEstadoEnProceso,*listaJobFinalizados,*listaEstadoFinalizadoOK;
extern float retardoPlanificacionSegs;
extern int idTareaGlobal;

void manejarFinReduccionGlobalOK(int sockMaster){


	int idTareaFinalizada = recibirValor(sockMaster);
	sleep(retardoPlanificacionSegs);
	printf("REDUCCION GLOBAL OK de la tarea %d\n",idTareaFinalizada);
	log_info(logInfo,"rg ok de la tarea %d",idTareaFinalizada);
	log_info(logInfo,"actuializo tbala de estados");
	//liberarCargaNodos(idTareaFinalizada);

	moverAListaFinalizadosOK(idTareaFinalizada);

	comenzarAlmacenadoFinal(idTareaFinalizada,sockMaster);



}

void manejarFinReduccionGlobalFail(int sockMaster){

	Theader *headEnvio=malloc(sizeof(Theader));
	int idTareaFinalizada = recibirValor(sockMaster);
	sleep(retardoPlanificacionSegs);
	printf("fin reduccion global fail id tarea %d \n",idTareaFinalizada);
	log_info(logInfo,"fin rg global fail de la tarea %d",idTareaFinalizada);
	log_info(logInfo,"actuializo tbala de estados");
	moverAListaError(idTareaFinalizada);
	printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
	puts("Se da x terminado el job");
	log_info(logInfo,"la tarea %d no se puede replanificar. se da x temrinado el job",idTareaFinalizada);
	headEnvio->tipo_de_proceso=YAMA;
	headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
	enviarHeader(sockMaster,headEnvio);
	liberarCargaNodos(idTareaFinalizada);

	if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
		TjobFinalizado *jobFinRG = malloc(sizeof (TjobFinalizado));
		TpackTablaEstados *tareaFinalizadaRG=getTareaPorId(idTareaFinalizada);
		jobFinRG->nroJob = tareaFinalizadaRG->job;
		jobFinRG->finCorrecto=false;
		list_add(listaJobFinalizados,jobFinRG);
	}

}


int comenzarReduccionGlobal(int idTareaFinalizada,int sockMaster){

	log_info(logInfo,"comenzar red glob");
	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	TjobMaster *job = getJobPorNroJob(tareaFinalizada->job);
	int i,packSize,stat;
	char * buffer;
	int jobAReducir = tareaFinalizada->job;


	int idTareaActual = idTareaGlobal++;


	TreduccionGlobal *nuevaReduccion = malloc(sizeof (TreduccionGlobal));
	nuevaReduccion->idTarea=idTareaActual;
	nuevaReduccion->job=jobAReducir;
	nuevaReduccion->tempRedGlobal=malloc(TAMANIO_NOMBRE_TEMPORAL);
	nuevaReduccion->tempRedGlobal=generarNombreReduccionGlobalTemporal(job->masterId);
	nuevaReduccion->tempRedGlobalLen=strlen(nuevaReduccion->tempRedGlobal)+1;
	nuevaReduccion->listaNodos=list_create();



	t_list * listaInformacionNodos = list_create();
	t_list * bloques = list_create();
	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
			TpackTablaEstados *tareaOk = list_get(listaEstadoFinalizadoOK,i);
			if(tareaOk->job==jobAReducir && tareaOk->etapa==REDUCCIONLOCAL){
				TinfoNodoReduccionGlobal * infoNodoAux=  malloc(sizeof (TinfoNodoReduccionGlobal));
				infoNodoAux->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
				infoNodoAux->nombreNodo=tareaOk->nodo;
				infoNodoAux->nombreNodoLen=strlen(infoNodoAux->nombreNodo)+1;
				infoNodoAux->ipNodo=malloc(MAXIMA_LONGITUD_IP);
				infoNodoAux->ipNodo=getIpNodo(infoNodoAux->nombreNodo,job);
				infoNodoAux->ipNodoLen=strlen(infoNodoAux->ipNodo)+1;
				infoNodoAux->puertoNodo=malloc(MAXIMA_LONGITUD_PUERTO);
				infoNodoAux->puertoNodo=getPuertoNodo(infoNodoAux->nombreNodo,job);
				infoNodoAux->puertoNodoLen=strlen(infoNodoAux->puertoNodo)+1;
				infoNodoAux->temporalReduccion=malloc(TAMANIO_NOMBRE_TEMPORAL);
				infoNodoAux->temporalReduccion=tareaOk->nombreArchTemporal;
				infoNodoAux->temporalReduccionLen=strlen(infoNodoAux->temporalReduccion)+1;
				infoNodoAux->nodoEncargado=0;
				list_add(listaInformacionNodos,infoNodoAux);
				list_add(bloques,tareaOk->bloquesReducidos);


			}
		}
	log_info(logInfo,"designo al encargado de la rg");

	mostrarTablaCargas();

	TinfoNodoReduccionGlobal *aux = list_get(listaInformacionNodos,0);
	int indiceElegido = 0;
	int menorCarga =getCargaWorker(aux->nombreNodo);
	int cargaAux;

	for(i=0;i<list_size(listaInformacionNodos);i++){
		aux=list_get(listaInformacionNodos,i);
		cargaAux=getCargaWorker(aux->nombreNodo);
		if(cargaAux<menorCarga){
			menorCarga=cargaAux;
			indiceElegido=i;
		}
	}

	aux=list_get(listaInformacionNodos,indiceElegido);
	aux->nodoEncargado=1;


	mostrarTablaCargas();
	log_info(logInfo,"Nodo encargado: %s. menor carga: %d \n",getNodoElegido(listaInformacionNodos),menorCarga);




	nuevaReduccion->listaNodosSize=list_size(listaInformacionNodos);
	nuevaReduccion->listaNodos=listaInformacionNodos;

	Theader head;
	head.tipo_de_proceso=YAMA;
	head.tipo_de_mensaje=INFOREDUCCIONGLOBAL;


	packSize=0;
	buffer=serializeInfoReduccionGlobal(head,nuevaReduccion,&packSize);
	log_info(logInfo,"Info de la reduccion global serializado, total %d bytes\n",packSize);
	if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la reduccion global. ");
		return  FALLO_SEND;
	}
	log_info(logInfo,"se enviaron %d bytes de la info de la reduccion global\n",stat);

	free(buffer);
	char * bloquesReducidos = string_new();
	string_append(&bloquesReducidos,"{");
	for(i=0;i<list_size(bloques);i++){
		if(i!=0) string_append(&bloquesReducidos,",");
		string_append(&bloquesReducidos,(list_get(bloques,i)));

	}
	string_append(&bloquesReducidos,"}");

	log_info(logInfo,"bloques de la RG: %s",bloquesReducidos);

	agregarReduccionGlobalAListaEnProceso(nuevaReduccion,bloquesReducidos,job);

	//int cargaWorker = divideYRedondea(list_size(job->listaNodosArchivo),2);
	int cargaWorker = getCargaReduccionGlobal(job->nroJob);
	log_info(logInfo,"\n\nsumo carga de la rg: %d a %s\n\n",cargaWorker,getNodoElegido(listaInformacionNodos));
	actualizarCargaWorkerEn(getNodoElegido(listaInformacionNodos),cargaWorker);
	aumentarHistoricoEn(getNodoElegido(listaInformacionNodos),cargaWorker);
	mostrarTablaCargas();

	list_destroy(bloques);
	return 0;
}



bool sePuedeComenzarReduccionGlobal(int idTareaFinalizada){
	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	int i;
	//1-Verifico q el job no se haya terminado x error de replanificacion .
	if(yaFueFinalizadoPorErrorDeReplanificacion(tareaFinalizada->job)){
		return false;
	}

	//2-Verifico si ya se terminaron todas las reducciones locales relacionadas con este archivo


	TpackTablaEstados *tareaAuxiliar;
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		tareaAuxiliar=list_get(listaEstadoEnProceso,i);
		if(tareaFinalizada->job==tareaAuxiliar->job){
			return false;
		}
	}





		return true;

}
