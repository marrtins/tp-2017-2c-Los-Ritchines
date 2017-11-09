/*
 * reduccionLocal.c
 *
 *  Created on: 9/11/2017
 *      Author: utnso
 */
#include "funcionesYM.h"

extern t_list * listaEstadoEnProceso,*listaJobFinalizados,*listaEstadoFinalizadoOK;
extern float retardoPlanificacionSegs;
extern int idTareaGlobal;

void manejarFinReduccionLocalOK(int sockMaster){

	int idTareaFinalizada = recibirValor(sockMaster);
	sleep(retardoPlanificacionSegs);
	printf("REDUCCION LOCAL OK de la tarea %d\n",idTareaFinalizada);
	puts("actuializo tbala de estados");

	moverAListaFinalizadosOK(idTareaFinalizada);

	if(sePuedeComenzarReduccionGlobal(idTareaFinalizada)){
		comenzarReduccionGlobal(idTareaFinalizada,sockMaster);
	}


}

void manejarFinReduccionLocalFail(int sockMaster){

	Theader * headEnvio=malloc(sizeof(Theader));
	int idTareaFinalizada = recibirValor(sockMaster);
	sleep(retardoPlanificacionSegs);
	printf("FIN REDUCCION LOCAL FAIL de la tarea%d\n",idTareaFinalizada);
	puts("actuializo tbala de estados");
	moverAListaError(idTareaFinalizada);
	printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
	puts("Se da x terminado el job");
	headEnvio->tipo_de_proceso=YAMA;
	headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
	enviarHeader(sockMaster,headEnvio);
	liberarCargaNodos(idTareaFinalizada);

	if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
		TjobFinalizado *jobFinRL = malloc(sizeof (TjobFinalizado));
		TpackTablaEstados *tareaFinalizadaRL=getTareaPorId(idTareaFinalizada);
		jobFinRL->nroJob = tareaFinalizadaRL->job;
		jobFinRL->finCorrecto=false;
		list_add(listaJobFinalizados,jobFinRL);
	}




}


int comenzarReduccionLocal(int idTareaFinalizada,int sockMaster){


	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	TjobMaster *job = getJobPorNroJob(tareaFinalizada->job);
	char * buffer;
	int packSize,stat;

	int jobAReducir = tareaFinalizada->job;

	int idTareaActual = idTareaGlobal++;


	char *  nodoReductor = tareaFinalizada->nodo;
	int i;

	TreduccionLocal * infoReduccion = malloc(sizeof(TreduccionLocal));
	infoReduccion->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
	infoReduccion->nombreNodo=nodoReductor;
	infoReduccion->nombreNodoLen=strlen(infoReduccion->nombreNodo)+1;
	infoReduccion->ipNodo=malloc(MAXIMA_LONGITUD_IP);
	infoReduccion->ipNodo=getIpNodo(nodoReductor,job);
	infoReduccion->ipLen=strlen(infoReduccion->ipNodo)+1;
	infoReduccion->puertoNodo=malloc(MAXIMA_LONGITUD_PUERTO);
	infoReduccion->puertoNodo=getPuertoNodo(nodoReductor,job);
	infoReduccion->puertoLen=strlen(infoReduccion->puertoNodo)+1;
	infoReduccion->tempRed=malloc(TAMANIO_NOMBRE_TEMPORAL);
	infoReduccion->tempRed=generarNombreReductorTemporal(nodoReductor,job->masterId);
	infoReduccion->tempRedLen=strlen(infoReduccion->tempRed)+1;
	infoReduccion->job=jobAReducir;
	infoReduccion->idTarea=idTareaActual;
	infoReduccion->listaTemporalesTransformacion=list_create();

	t_list *listaTemporales= list_create();
	t_list * bloques = list_create();
	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
		TpackTablaEstados *tareaOk = list_get(listaEstadoFinalizadoOK,i);
		if(tareaOk->job==jobAReducir && tareaOk->nodo==nodoReductor &&tareaOk->etapa==TRANSFORMACION){
			TreduccionLista * reduccionAux=  malloc(sizeof (TreduccionLista));
			reduccionAux->nombreTemporal=malloc(TAMANIO_NOMBRE_TEMPORAL);
			reduccionAux->nombreTemporal=tareaOk->nombreArchTemporal;
			reduccionAux->nombreTemporalLen=strlen(reduccionAux->nombreTemporal)+1;

			list_add(listaTemporales,reduccionAux);
			int bloqueAux = tareaOk->bloqueDelArchivo;
			list_add(bloques,string_itoa(bloqueAux));

		}
	}
	infoReduccion->listaSize=list_size(listaTemporales);
	infoReduccion->listaTemporalesTransformacion=listaTemporales;

	Theader head;
	head.tipo_de_proceso=YAMA;
	head.tipo_de_mensaje=INFOREDUCCIONLOCAL;


	packSize=0;
	buffer=serializeInfoReduccionLocal(head,infoReduccion,&packSize);
	printf("Info de la reduccion local serializado, total %d bytes\n",packSize);
	if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info dede la reduccion local ");
		return  FALLO_SEND;
	}
	printf("se enviaron %d bytes de la info de la reduccion local\n",stat);

	char * bloquesReducidos = string_new();
	string_append(&bloquesReducidos,"[");
	for(i=0;i<list_size(bloques);i++){
		if(i!=0) string_append(&bloquesReducidos,", ");
		string_append(&bloquesReducidos,(list_get(bloques,i)));

	}
	string_append(&bloquesReducidos,"]");

	agregarReduccionLocalAListaEnProceso(infoReduccion,bloquesReducidos,job);

	actualizarCargaWorkerEn(infoReduccion->nombreNodo,1);
	aumentarHistoricoEn(infoReduccion->nombreNodo,1);
	return 0;
}


bool sePuedeComenzarReduccionLocal(int idTareaFinalizada){
	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	int i;
//1-Verifico q el job no se haya terminado x error de replanificacion .
	if(yaFueFinalizadoPorErrorDeReplanificacion(tareaFinalizada->job)){
		return false;
	}

//2-Verifico si ya se terminaron todas las transformaciones relacionadas con este archivo
//en el nodo q acaba de finalizar

	TpackTablaEstados *tareaAuxiliar;
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		tareaAuxiliar=list_get(listaEstadoEnProceso,i);
		if(tareaFinalizada->job == tareaAuxiliar->job){
			if((tareaFinalizada->nodo == tareaAuxiliar->nodo) && (tareaAuxiliar->etapa == TRANSFORMACION)){
				return false;
			}
		}
	}


	return true;
}


