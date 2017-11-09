/*
 * transformacion.c
 *
 *  Created on: 9/11/2017
 *      Author: utnso
 */

#include "funcionesYM.h"
extern t_list * listaJobFinalizados,*listaJobsMaster;
extern float retardoPlanificacionSegs;
extern int idTareaGlobal,idJobGlobal,idMasterGlobal;



void iniciarNuevoJob(int sockMaster,int socketFS){

	TjobMaster *nuevoJob=malloc(sizeof(TjobMaster));
	puts("Master quiere iniciar un nuevo JOB");
	idJobGlobal++;
	idMasterGlobal++;
	nuevoJob->fdMaster=sockMaster;
	nuevoJob->nroJob=idJobGlobal;
	nuevoJob->masterId=idMasterGlobal;
	printf("Id del nuevo job: %d, master id:%d, fdMaster :%d \n",idJobGlobal,idMasterGlobal,sockMaster);
	char * pathResultado = recibirPathArchivo(sockMaster);
	nuevoJob->pathResultado=pathResultado;

	list_add(listaJobsMaster,nuevoJob);
	Theader head;
	int stat;
	stat = recv(sockMaster, &head, sizeof(Theader), 0);
	char * pathArchivoAReducir;
	if(stat>0 && head.tipo_de_mensaje==PATH_FILE_TOREDUCE){

		printf("llega path file to reduce del job :%d. ID master: %d\n",nuevoJob->nroJob,nuevoJob->masterId);
		pathArchivoAReducir  = recibirPathArchivo(sockMaster);
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
		puts("#envio1");



		TinfoNodosFSYama *infoNodos;
		TinfoArchivoFSYama *infoArchivo;
	stat = recv(socketFS, &head, sizeof(Theader), 0);
	if (head.tipo_de_mensaje == ARCH_NO_VALIDO) {
		puts("El archivo no es valido");
	} else if (head.tipo_de_mensaje == INFO_ARCHIVO) {
		puts("FS nos quiere mandar la info del archivo que pedi");

		buffer3 = recvGeneric(socketFS);
		infoArchivo = deserializarInfoArchivoYamaFS(buffer3);

		stat = recv(socketFS, &head, sizeof(Theader), 0);
		if(head.tipo_de_mensaje==INFO_NODO){
			puts("ahora recibimos info de los nodos");
			buffer3 = recvGeneric(socketFS);
			 infoNodos= deserializarInfoNodosFSYama(buffer3);
			puts("ya recibi todo");
		}else{
			puts("error al recibir info nodos");
		}


		int i;
		for(i=0;i<infoArchivo->listaSize;i++){
			TpackageUbicacionBloques *bloqueAux = list_get((infoArchivo->listaBloques),i);
			printf("bloque %d ; nodoc1 %s ;bloquec1 %d;nodoc2 %s;bloquec2 %d;finbloque %d\n",bloqueAux->bloque,bloqueAux->nombreNodoC1,bloqueAux->bloqueC1,bloqueAux->nombreNodoC2, bloqueAux->bloqueC2,bloqueAux->finBloque);
		}
		for(i=0;i<infoNodos->listaSize;i++){
			TpackageInfoNodo *nodoAux=list_get((infoNodos->listaNodos),i);
			printf("nombre nodo %s ipnodo %s puerto nodo %s\n",nodoAux->nombreNodo,nodoAux->ipNodo,nodoAux->puertoWorker);
		}

	}
	puts("pongo en nevo job");
	nuevoJob->listaComposicionArchivo=list_create();
	nuevoJob->listaComposicionArchivo=infoArchivo->listaBloques;
	nuevoJob->listaNodosArchivo=list_create();
	nuevoJob->listaNodosArchivo=infoNodos->listaNodos;





	/*while((stat = recv(socketFS, &head, sizeof(Theader), 0))>0){
		if (head.tipo_de_mensaje == ARCH_NO_VALIDO) {
			puts("El archivo no es valido");
		} else if (head.tipo_de_mensaje == INFO_ARCHIVO) {
			puts("FS nos quiere mandar la info del archivo que pedi");
			unsigned long long sizePaquete;
			stat = recv(socketFS, &sizePaquete, sizeof(unsigned long long), 0);
			printf("paquete size %llu\n",sizePaquete);
			char *buffer3 = malloc(sizePaquete);
			puts("Recibo el tamaño del paquete");
			stat = recv(socketFS, buffer3, sizePaquete, 0);
			puts("Recibo el paquete");
			TinfoArchivoFSYama *infoArchivo = malloc(sizeof(TinfoArchivoFSYama));
			infoArchivo = deserializarInfoArchivoYamaFS(buffer3);
			nuevoJob->listaComposicionArchivo=list_create();
			nuevoJob->listaComposicionArchivo=infoArchivo->listaBloques;
			puts("deserealice la info del archivo");
			//seguior aca info nodos ....


			stat = recv(socketFS, &head, sizeof(Theader), 0);
			puts("recibo otro head");
			if (head.tipo_de_mensaje == INFO_NODO) {
				puts("FS me quiere dar la info del nodo");
				TpackInfoBloqueDN * infoBloque;
				infoBloque = recvInfoNodoYAMA(socketFS);

				puts("Recibo la informacion del nodo");

				TpackageInfoNodo * nodo = malloc(
						sizeof(TpackageInfoNodo));

				nodo->tamanioIp = infoBloque->tamanioIp;
				nodo->tamanioNombre = infoBloque->tamanioNombre;
				nodo->tamanioPuerto = infoBloque->tamanioPuerto;

				nodo->ipNodo = malloc(nodo->tamanioIp);
				nodo->nombreNodo = malloc(nodo->tamanioNombre);
				nodo->puertoWorker = malloc(nodo->tamanioPuerto);

				strcpy(nodo->ipNodo, infoBloque->ipNodo);
				strcpy(nodo->nombreNodo, infoBloque->nombreNodo);
				strcpy(nodo->puertoWorker, infoBloque->puertoNodo);
				puts("Termine");

			}
		}
	}
	*/


	//fs me devuelve una lista de nodos y una lista d ebloques

	//t_list *listaComposicionArchivo=list_create();
	//generarListaComposicionArchivoHardcode(listaComposicionArchivo);

	responderTransformacion(nuevoJob);
}











int responderTransformacion(TjobMaster *job){

	int sockMaster = job->fdMaster;
	int stat;

	job->nroJob=idJobGlobal++;
	sleep(retardoPlanificacionSegs);

	t_list *listaBloquesPlanificados=planificar(job);

	if((stat=responderSolicTransf(sockMaster,listaBloquesPlanificados,job))<0){
		puts("No se pudo responder la solicitud de transferencia");
		return -1;
	}


	Theader head;
	head.tipo_de_proceso=YAMA;

	int i;
	int packSize;
	char * buffer;

	printf("Cantidad de paquetes con info de bloques a enviar: %d\n",list_size(listaBloquesPlanificados));
	for (i=0;i<list_size(listaBloquesPlanificados);i++){
		if(i<=list_size(listaBloquesPlanificados)-2){
			head.tipo_de_mensaje=INFOBLOQUE;
		}else{
			head.tipo_de_mensaje=INFOULTIMOBLOQUE;
		}
		packSize=0;
		TpackInfoBloque *bloqueAEnviar = list_get(listaBloquesPlanificados,i);

		bloqueAEnviar->idTarea = idTareaGlobal++;

		buffer=serializeInfoBloque(head,bloqueAEnviar,&packSize);

		printf("Info del bloque %d serializado, enviamos\n",bloqueAEnviar->bloqueDelArchivo);

		if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
			puts("no se pudo enviar info del bloque. ");
			return  FALLO_SEND;
		}
		printf("se enviaron %d bytes de la info del bloque\n",stat);

		agregarTransformacionAListaEnProceso(job,bloqueAEnviar);


	}

	puts("Se envio la info de todos los bloques.");


return 0;
}

void manejarFinTransformacionOK(int sockMaster){

	int idTareaFinalizada;

	idTareaFinalizada = recibirValor(sockMaster);
	sleep(retardoPlanificacionSegs);
	printf("FINTRANSFORMACIONLOCAL OK de la tarea%d\n",idTareaFinalizada);
	puts("actuializo tbala de estados");
	moverAListaFinalizadosOK(idTareaFinalizada);


	if(sePuedeComenzarReduccionLocal(idTareaFinalizada)){
		comenzarReduccionLocal(idTareaFinalizada,sockMaster);
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
				puts("actuializo tbala de estados");

				moverAListaError(idTareaFinalizada);

				if(sePuedeReplanificar(idTareaFinalizada,jobFinalizado->listaComposicionArchivo)){

					stat = replanificar(idTareaFinalizada,sockMaster,jobFinalizado->listaComposicionArchivo);

					if(stat<0){
						//esto no deberia pasar nunca.. lo dejo aca para que no rompa tod
						printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
						puts("Se da x terminado el job");
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


int responderSolicTransf(int sockMaster,t_list * listaBloques,TjobMaster *job){


	//aca planifico, pido la inf del archivo,etcetc

	//x ahora envio una rta hardcodeada pero siguiendo el formato
	int i,packSize,stat;
	char *buffer;






	Theader head;
	head.tipo_de_proceso=YAMA;



	printf("Cantidad de paquetes con info de bloques a enviar: %d\n",list_size(listaBloques));
	for (i=0;i<list_size(listaBloques);i++){
		if(i<=list_size(listaBloques)-2){
			head.tipo_de_mensaje=INFOBLOQUE;
		}else{
			head.tipo_de_mensaje=INFOULTIMOBLOQUE;
		}
		packSize=0;
		TpackInfoBloque *bloqueAEnviar = list_get(listaBloques,i);

		bloqueAEnviar->idTarea = idTareaGlobal++;

		buffer=serializeInfoBloque(head,bloqueAEnviar,&packSize);

		printf("Info del bloque %d serializado, enviamos\n",bloqueAEnviar->bloqueDelArchivo);

		if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
			puts("no se pudo enviar info del bloque. ");
			return  FALLO_SEND;
		}
		printf("se enviaron %d bytes de la info del bloque\n",stat);

		agregarTransformacionAListaEnProceso(job,bloqueAEnviar);


	}

	puts("Se envio la info de todos los bloques.");





	return 0;


}




