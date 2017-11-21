/*
 * almacenamiento.c
 *
 *  Created on: 9/11/2017
 *      Author: utnso
 */
#include "funcionesYM.h"
extern t_list * listaEstadoEnProceso,*listaJobFinalizados;
extern float retardoPlanificacionSegs;
extern int idTareaGlobal;
void manejarFinAlmacenamientoOK(int sockMaster){
	int idTareaFinalizada = recibirValor(sockMaster);
	sleep(retardoPlanificacionSegs);
	printf("Almacenamiento final de la tarea %d\n",idTareaFinalizada);
	puts("actuializo tbala de estados");
	moverAListaFinalizadosOK(idTareaFinalizada);
	liberarCargaNodos(idTareaFinalizada);
}


void manejarFinAlmacenamientoFail(int sockMaster){
	Theader *headEnvio = malloc(sizeof(Theader));
	int idTareaFinalizada = recibirValor(sockMaster);
	sleep(retardoPlanificacionSegs);
	puts("fin almacenamiento local fail");
	puts("actuializo tbala de estados");
	moverAListaError(idTareaFinalizada);
	printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
	puts("Se da x terminado el job");
	headEnvio->tipo_de_proceso=YAMA;
	headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
	enviarHeader(sockMaster,headEnvio);
	liberarCargaNodos(idTareaFinalizada);
	if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
		TjobFinalizado *jobFinAF = malloc(sizeof (TjobFinalizado));
		TpackTablaEstados *tareaFinalizadaAF=getTareaPorId(idTareaFinalizada);
		jobFinAF->nroJob = tareaFinalizadaAF->job;
		jobFinAF->finCorrecto=false;
		list_add(listaJobFinalizados,jobFinAF);
	}
	free(headEnvio);

}



int comenzarAlmacenadoFinal(int idTareaFinalizada,int sockMaster){

	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	TjobMaster *job = getJobPorNroJob(tareaFinalizada->job);

	int idTareaActual = idTareaGlobal++;


	TinfoAlmacenadoFinal *infoAlmacenado = malloc(sizeof(TinfoAlmacenadoFinal));
	infoAlmacenado->idTarea=idTareaActual;
	infoAlmacenado->job=tareaFinalizada->job;
	infoAlmacenado->ipNodo=malloc(MAXIMA_LONGITUD_IP);
	infoAlmacenado->ipNodo=getIpNodo(tareaFinalizada->nodo,job);
	infoAlmacenado->ipNodoLen=strlen(infoAlmacenado->ipNodo)+1;

	infoAlmacenado->puertoNodo=malloc(MAXIMA_LONGITUD_IP);
	infoAlmacenado->puertoNodo=getPuertoNodo(tareaFinalizada->nodo,job);
	infoAlmacenado->puertoNodoLen=strlen(infoAlmacenado->puertoNodo)+1;

	infoAlmacenado->nombreTempReduccion=malloc(MAXIMA_LONGITUD_IP);
	infoAlmacenado->nombreTempReduccion=tareaFinalizada->nombreArchTemporal;
	infoAlmacenado->nombreTempReduccionLen=strlen(infoAlmacenado->nombreTempReduccion)+1;

	Theader head;
	head.tipo_de_proceso=YAMA;
	head.tipo_de_mensaje=INFOALMACENADOFINAL;

	int packSize;
	char * buffer;
	int stat;
	packSize=0;
	buffer=serializeInfoAlmacenadoFinal(head,infoAlmacenado,&packSize);
	printf("Info del almacenado final serializado, total %d bytes\n",packSize);
	if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info del almacenado final. ");
		return  FALLO_SEND;
	}
	printf("se enviaron %d bytes de la info del almacenado final\n",stat);

	free(buffer);



	agregarAlmacenadoFinalAListaEnProceso(infoAlmacenado,tareaFinalizada->nodo,tareaFinalizada->bloquesReducidos,job);


	return 0;
}
void agregarAlmacenadoFinalAListaEnProceso(TinfoAlmacenadoFinal *infoAlmacenado,char * nombreNodo,char * bloquesReducidos,TjobMaster *job){

		TpackTablaEstados * estado = malloc(sizeof (TpackTablaEstados));
		estado->idTarea=infoAlmacenado->idTarea;
		estado->job=infoAlmacenado->job;
		estado->master=job->masterId;
		estado->bloqueDelArchivo=-1;
		estado->nodo=malloc(TAMANIO_NOMBRE_NODO);
		estado->nodo=nombreNodo;
		estado->etapa=ALMACENAMIENTOFINAL;
		estado->nombreArchTemporal=job->pathResultado;
		estado->bloquesReducidos=malloc(strlen(bloquesReducidos)+1);
		estado->bloquesReducidos=bloquesReducidos;
		list_add(listaEstadoEnProceso,estado);

		mostrarTablaDeEstados();
}
