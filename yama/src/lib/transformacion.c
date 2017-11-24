/*
 * transformacion.c
 *
 *  Created on: 9/11/2017
 *      Author: utnso
 */

#include "funcionesYM.h"
extern t_list * listaJobFinalizados,*listaJobsMaster,*listaEstadoFinalizadoOK;
extern float retardoPlanificacionSegs;
extern int idTareaGlobal,idJobGlobal,idMasterGlobal;



void iniciarNuevoJob(int sockMaster,int socketFS){

	TjobMaster *nuevoJob=malloc(sizeof(TjobMaster));
	puts("Master quiere iniciar un nuevo JOB");
	log_info(logInfo,"master quiere inicar u nvo job");
	idJobGlobal++;
	idMasterGlobal++;
	nuevoJob->fdMaster=sockMaster;
	nuevoJob->nroJob=idJobGlobal;
	nuevoJob->masterId=idMasterGlobal;
	printf("Id del nuevo job: %d, master id:%d, fdMaster :%d \n",idJobGlobal,idMasterGlobal,sockMaster);
	log_info(logInfo,"Id del nuevo job: %d, master id:%d, fdMaster :%d \n",idJobGlobal,idMasterGlobal,sockMaster);
	char * pathResultado = recibirPathArchivo(sockMaster);
	nuevoJob->pathResultado=pathResultado;

	list_add(listaJobsMaster,nuevoJob);
	Theader head;
	Theader *headEnvio=malloc(sizeof(Theader));
	int stat;
	stat = recv(sockMaster, &head, sizeof(Theader), 0);
	char * pathArchivoAReducir;
	if(stat>0 && head.tipo_de_mensaje==PATH_FILE_TOREDUCE){

		log_info(logInfo,"llega path file to reduce del job :%d. ID master: %d\n",nuevoJob->nroJob,nuevoJob->masterId);
		pathArchivoAReducir  = recibirPathArchivo(sockMaster);
		log_info(logInfo,"patha rch a red %s",pathArchivoAReducir);
	}

	//todo: aca pido info a filesystem sobre el archivo.


	head.tipo_de_proceso=YAMA;
	head.tipo_de_mensaje=INFO_ARCHIVO;

	char * buffer3;

	//envio la ruta del archivo a reducir a filesystem para que me devuelva la info del archivo
	/*buffer1=empaquetarBytes(head2,pathArchivoAReducir);
	puts("Path del archivo a reducir serializado; lo enviamos");
	if ((stat = send(socketFS, buffer1->buffer, buffer1->tamanio, 0)) == -1){
		puts("no se pudo enviar Path del archivo a reducir a FILESYSTEM. ");
		return;
	}
	printf("se enviaron %d bytes del Path del archivo a reducir a FS\n",stat);
*/	int packSize;
	char * buffer=serializeBytes(head,pathArchivoAReducir,(strlen(pathArchivoAReducir)+1),&packSize);
	if ((stat = send(socketFS, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar Path del aresultado a YAMA. ");
		return  ;
	}




		TinfoNodosFSYama *infoNodos;
		TinfoArchivoFSYama *infoArchivo;
	stat = recv(socketFS, &head, sizeof(Theader), 0);
	log_info(logInfo,"stat redv foscket fs %d \n",stat);
	if (head.tipo_de_mensaje == ARCH_NO_VALIDO) {
		puts("El archivo no es valido");
		log_info(logInfo,"el ach no es valido");
		headEnvio->tipo_de_proceso=YAMA;
		headEnvio->tipo_de_mensaje=ARCH_NO_VALIDO;
		enviarHeader(sockMaster,headEnvio);
		return;
	} else if (head.tipo_de_mensaje == INFO_ARCHIVO) {
		puts("FS nos quiere mandar la info del archivo que pedi");
		log_info(logInfo,"Fs nos manda la info del archivo pedid");
		buffer3 = recvGeneric(socketFS);
		infoArchivo = deserializarInfoArchivoYamaFS(buffer3);
		free(buffer3);
		stat = recv(socketFS, &head, sizeof(Theader), 0);
		if(head.tipo_de_mensaje==INFO_NODO){
			puts("ahora recibimos info de los nodos");
			log_info(logInfo,"ahora info de los nods");
			buffer3 = recvGeneric(socketFS);
			 infoNodos= deserializarInfoNodosFSYama(buffer3);
			 log_info(logInfo,"ya recibi todo");
			 free(buffer3);
		}else{
			log_info(logInfo,"error al recibir info nodos");
			puts("error al recibir info de los nodos");
		}


		int i;
		for(i=0;i<infoArchivo->listaSize;i++){
			TpackageUbicacionBloques *bloqueAux = list_get((infoArchivo->listaBloques),i);
			log_info(logInfo,"bloque %d ; nodoc1 %s ;bloquec1 %d;nodoc2 %s;bloquec2 %d;finbloque %d\n",bloqueAux->bloque,bloqueAux->nombreNodoC1,bloqueAux->bloqueC1,bloqueAux->nombreNodoC2, bloqueAux->bloqueC2,bloqueAux->finBloque);
		}
		for(i=0;i<infoNodos->listaSize;i++){
			TpackageInfoNodo *nodoAux=list_get((infoNodos->listaNodos),i);
			log_info(logInfo,"nombre nodo %s ipnodo %s puerto nodo %s\n",nodoAux->nombreNodo,nodoAux->ipNodo,nodoAux->puertoWorker);
		}

	}
	log_info(logInfo,"pongo en nevo job");
	nuevoJob->listaComposicionArchivo=list_create();
	nuevoJob->listaComposicionArchivo=infoArchivo->listaBloques;
	nuevoJob->listaNodosArchivo=list_create();
	nuevoJob->listaNodosArchivo=infoNodos->listaNodos;
	//list_destroy_and_destroy_elements(infoArchivo->listaBloques,liberarInfoArchivo);
	//list_destroy_and_destroy_elements(infoNodos->listaNodos,liberarInfoNodos);

	free(infoArchivo);
	free(infoNodos);

	responderTransformacion(nuevoJob);
}

void liberarInfoArchivo(void * info){
	log_info(logInfo,"free info archi ");
	TpackageUbicacionBloques * bloque = (TpackageUbicacionBloques*) info;
	log_info(logInfo,"1");
	free(bloque->nombreNodoC1);
	free(bloque->nombreNodoC2);
	free(bloque);
	log_info(logInfo,"pase free   info archi");
}


void liberarInfoNodos(void * info){
	log_info(logInfo,"free info nodos");
	TpackageInfoNodo * nodo = (TpackageInfoNodo*) info;
	log_info(logInfo,"1");
	free(nodo->ipNodo);
	free(nodo->nombreNodo);
	free(nodo->puertoWorker);
	free(nodo);
	log_info(logInfo,"pase free info nodosl");
}










int responderTransformacion(TjobMaster *job){

	int sockMaster = job->fdMaster;
	int stat;

	job->nroJob=idJobGlobal++;
	sleep(retardoPlanificacionSegs);

	t_list *listaBloquesPlanificados=planificar(job);


	log_info(logInfo,"lista blqoes planificados job%d",job->nroJob);
	int k;
	for(k=0;k<list_size(listaBloquesPlanificados);k++){
		TpackInfoBloque *bloqueAux = list_get(listaBloquesPlanificados,k);
		log_info(logInfo,"blArch %d blData %d bytsOcup %d idTarea %d ipW %s pWork %s nombreNo %s nTemp %s",bloqueAux->bloqueDelArchivo,bloqueAux->bloqueDelDatabin,bloqueAux->bytesOcupados,bloqueAux->idTarea,bloqueAux->ipWorker,bloqueAux->puertoWorker,bloqueAux->nombreNodo,bloqueAux->nombreTemporal);
	}
	/*if((stat=responderSolicTransf(sockMaster,listaBloquesPlanificados,job))<0){
		puts("No se pudo responder la solicitud de transferencia");
		return -1;
	}*/


	Theader head;
	head.tipo_de_proceso=YAMA;

	int i;
	int packSize;
	char * buffer;
	bool mostrarTabla = false;

	log_info(logInfo,"Cantidad de paquetes con info de bloques a enviar: %d\n",list_size(listaBloquesPlanificados));
	for (i=0;i<list_size(listaBloquesPlanificados);i++){
		if(i+1<list_size(listaBloquesPlanificados)){
			head.tipo_de_mensaje=INFOBLOQUE;
			mostrarTabla=false;
		}else{
			head.tipo_de_mensaje=INFOULTIMOBLOQUE;
			mostrarTabla=true;
		}
		packSize=0;
		TpackInfoBloque *bloqueAEnviar = list_get(listaBloquesPlanificados,i);

		bloqueAEnviar->idTarea = idTareaGlobal++;

		buffer=serializeInfoBloque(head,bloqueAEnviar,&packSize);

		log_info(logInfo,"Info del bloque %d serializado, enviamos\n",bloqueAEnviar->bloqueDelArchivo);

		if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
			puts("no se pudo enviar info del bloque. ");
			return  FALLO_SEND;
		}
		log_info(logInfo,"se enviaron %d bytes de la info del bloque\n",stat);
		free(buffer);
		agregarTransformacionAListaEnProceso(job,bloqueAEnviar,mostrarTabla);


	}
	list_destroy(listaBloquesPlanificados);
	//list_destroy_and_destroy_elements(listaBloquesPlanificados,liberarBloquesPlanificados);


	log_info(logInfo,"Se envio la info de todos los bloques.");


return 0;
}

void liberarBloquesPlanificados(void * info){
	log_info(logInfo,"free info bl pl");
	TpackInfoBloque * infoBloque = (TpackInfoBloque*) info;
	log_info(logInfo,"1");
	free(infoBloque->ipWorker);
	log_info(logInfo,"2");
	free(infoBloque->nombreNodo);
	log_info(logInfo,"3");
	free(infoBloque->nombreTemporal);
	log_info(logInfo,"4");
	free(infoBloque->puertoWorker);
	log_info(logInfo,"5");
	free(infoBloque);
	log_info(logInfo,"6");
	log_info(logInfo,"pase free info bl pl");
}

void manejarFinTransformacionOK(int sockMaster){

	int idTareaFinalizada;

	idTareaFinalizada = recibirValor(sockMaster);
	sleep(retardoPlanificacionSegs);
	printf("FINTRANSFORMACIONLOCAL OK de la tarea%d\n",idTareaFinalizada);
	log_info(logInfo,"fin TL OK de la tarea %d",idTareaFinalizada);
	log_info(logInfo,"actuializo tbala de estados");
	moverAListaFinalizadosOK(idTareaFinalizada);

	//liberarCargaNodos(idTareaFinalizada);
	//liberarCargaEn()

	if(sePuedeComenzarReduccionLocal(idTareaFinalizada)){
		log_info(logInfo,"se pued comenzar RL");
		comenzarReduccionLocal(idTareaFinalizada,sockMaster);
	}


}

void manejarFinTransformacionFailDesconexion(int sockMaster){
	int idTareaFinalizada,stat;
	Theader *headEnvio=malloc(sizeof(Theader));

	idTareaFinalizada = recibirValor(sockMaster);
	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);


	TjobMaster *jobFinalizado = getJobPorNroJob(tareaFinalizada->job);

	sleep(retardoPlanificacionSegs);
	printf("FINTRANSFORMACIONLOCAL FAIL x desconexion del %s de la tarea%d\n",tareaFinalizada->nodo,idTareaFinalizada);
	log_info(logInfo,"fin TL fail x desco del %s . tarea %d",tareaFinalizada->nodo,idTareaFinalizada);
	puts("actuializo tbala de estados y reasigno todos los temporales ok a otros nodos");
	log_info(logInfo,"actuializo tbala de estados y reasigno todos los temporales ok a otros nodos");

	moverAListaError(idTareaFinalizada);


	if(sePuedeReplanificar(idTareaFinalizada,jobFinalizado->listaComposicionArchivo)){
			log_info(logInfo,"se peude replanificar");
			stat = replanificar(idTareaFinalizada,sockMaster,jobFinalizado->listaComposicionArchivo);

			if(stat<0){
				//esto no deberia pasar nunca.. lo dejo aca para que no rompa tod
				printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
				puts("Se da x terminado el job");
				log_info(logInfo,"al final no se podia");
				headEnvio->tipo_de_proceso=YAMA;
				headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
				enviarHeader(sockMaster,headEnvio);

				if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
					TjobFinalizado *job = malloc(sizeof (TjobFinalizado));
					TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
					job->nroJob = tareaFinalizada->job;
					job->finCorrecto=false;
					list_add(listaJobFinalizados,job);
				}

			}
		}else{

			printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
			puts("Se da x terminado el job");
			log_info(logInfo,"%d no se puede repla. se da x temrinado eljobn",idTareaFinalizada);
			headEnvio->tipo_de_proceso=YAMA;
			headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
			enviarHeader(sockMaster,headEnvio);
			liberarCargaNodos(idTareaFinalizada);

			if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
				TjobFinalizado *job = malloc(sizeof (TjobFinalizado));
				TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
				job->nroJob = tareaFinalizada->job;
				job->finCorrecto=false;
				list_add(listaJobFinalizados,job);
			}

		}



	int i;
	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
		TpackTablaEstados *aux = list_get(listaEstadoFinalizadoOK,i);
		if(string_equals_ignore_case(aux->nodo,tareaFinalizada->nodo) && tareaFinalizada->etapa==TRANSFORMACION && tareaFinalizada->job == aux->job){
			moverFinalizadaAListaError(aux->idTarea);
			//list_remove(listaEstadoFinalizadoOK,i);
			if(sePuedeReplanificar(aux->idTarea,jobFinalizado->listaComposicionArchivo)){
				replanificar(aux->idTarea,sockMaster,jobFinalizado->listaComposicionArchivo);
			}else{
				printf("La tarea %d no se puede replanificar ",aux->idTarea);
				puts("Se da x terminado el job");
				log_info(logInfo,"la tarea %d no se puede repla. se da x termiando el job",aux->idTarea);
				headEnvio->tipo_de_proceso=YAMA;
				headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
				enviarHeader(sockMaster,headEnvio);
				liberarCargaNodos(aux->idTarea);

				if(!yaFueAgregadoAlistaJobFinalizados(aux->idTarea)){
					TjobFinalizado *job = malloc(sizeof (TjobFinalizado));
					TpackTablaEstados *tareaFinalizada=getTareaPorId(aux->idTarea);
					job->nroJob = tareaFinalizada->job;
					job->finCorrecto=false;
					list_add(listaJobFinalizados,job);
				}
			}
		}
	}

}

void manejarFinTransformacionFail(int sockMaster){
	int idTareaFinalizada,stat;
	Theader *headEnvio=malloc(sizeof(Theader));

	idTareaFinalizada = recibirValor(sockMaster);
	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);


	TjobMaster *jobFinalizado = getJobPorNroJob(tareaFinalizada->job);

	sleep(retardoPlanificacionSegs);
	printf("FINTRANSFORMACIONLOCAL FAIL de la tarea%d\n",idTareaFinalizada);
	log_info(logInfo,"fin transf fail de la tarea %d ",idTareaFinalizada);
	log_info(logInfo,"actuializo tbala de estados");


	moverAListaError(idTareaFinalizada);

	if(sePuedeReplanificar(idTareaFinalizada,jobFinalizado->listaComposicionArchivo)){
		log_info(logInfo,"se peude replanificar");
		stat = replanificar(idTareaFinalizada,sockMaster,jobFinalizado->listaComposicionArchivo);

		if(stat<0){
			//esto no deberia pasar nunca.. lo dejo aca para que no rompa tod
			printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
			puts("Se da x terminado el job");
			log_info(logInfo,"al final no se podia");
			headEnvio->tipo_de_proceso=YAMA;
			headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
			enviarHeader(sockMaster,headEnvio);

			if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
				TjobFinalizado *job = malloc(sizeof (TjobFinalizado));
				TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
				job->nroJob = tareaFinalizada->job;
				job->finCorrecto=false;
				list_add(listaJobFinalizados,job);
			}

		}
	}else{

		printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
		puts("Se da x terminado el job");
		log_info(logInfo,"%d no se puede repla. se da x temrinado eljobn",idTareaFinalizada);
		headEnvio->tipo_de_proceso=YAMA;
		headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
		enviarHeader(sockMaster,headEnvio);
		liberarCargaNodos(idTareaFinalizada);

		if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
			TjobFinalizado *job = malloc(sizeof (TjobFinalizado));
			TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
			job->nroJob = tareaFinalizada->job;
			job->finCorrecto=false;
			list_add(listaJobFinalizados,job);
		}

	}

}




