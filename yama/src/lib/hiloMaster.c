/*
 * hiloMaster.c
 *
 *  Created on: 26/10/2017
 *      Author: utnso
 */
#include "funcionesYM.h"
int idTempName,idPropio;


extern int idMasterGlobal,idJobGlobal,idTareaGlobal;
extern float retardoPlanificacionSegs;

t_list *listaNodosDelJob;
extern t_list *listaJobFinalizados, * listaHistoricaTareas,*listaCargaGlobal,* listaEstadoEnProceso,*listaEstadoError,*listaEstadoFinalizadoOK;
extern pthread_mutex_t mux_listaJobFinalizados, mux_idTareaGlobal,mux_listaHistorica,mux_listaCargaGlobal,mux_idGlobal,mux_listaEnProceso,mux_listaError,mux_listaFinalizado,mux_jobIdGlobal;
extern Tyama *yama;
TpackBytes *pathResultado;

void masterHandler(void *atributos){

	//int sockMaster = (int *)client_sock;
	TatributosHiloMaster *atr = (TatributosHiloMaster *)atributos;
	int sockMaster =atr->fdMaster;
	int stat;
	int estado;
	int idTareaFinalizada;
	idTempName=0;

	listaNodosDelJob=list_create();


	pthread_mutex_lock(&mux_idGlobal);

	idPropio=idMasterGlobal++;
	pthread_mutex_unlock(&mux_idGlobal);

	Theader head = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};
	Theader *headEnvio = malloc(sizeof(headEnvio));

	TpackBytes *pathArchivoAReducir;


	char* buffer;
	Tbuffer * buffer1;

	puts("Nuevo hilo MASTERHANDLER creado");
	puts("Esperando solicitud de master");

	while((estado = recv(sockMaster, &head, HEAD_SIZE, 0)) > 0){
		switch(head.tipo_de_mensaje){

		case INICIOMASTER:
			puts("Master quiere iniciar un nuevo JOB. esperamos a recibir la info");

			break;


		case PATH_FILE_TOREDUCE:

			puts("Nos llega el path del archivo a reducir");

			if ((buffer = recvGeneric(sockMaster)) == NULL){
				puts("Fallo recepcion de PATH_FILE_TOREDUCE");
				return;
			}

			if ((pathArchivoAReducir = (TpackBytes *) deserializeBytes(buffer)) == NULL){
				puts("Fallo deserializacion de Bytes del path arch a reducir");
				return;
			}

			printf("Path archivo a reducir: : %s\n",pathArchivoAReducir->bytes);

			break;

		case PATH_RES_FILE:
			puts("Nos llega el path del resultado");
			if ((buffer = recvGeneric(sockMaster)) == NULL){
				puts("Fallo recepcion de PATH_RES_FILE");
				return;
			}
			if ((pathResultado = (TpackBytes *) deserializeBytes(buffer)) == NULL){
				puts("Fallo deserializacion de Bytes del path_res_file");
				return;
			}
			printf("Path del resultado: : %s\n",pathResultado->bytes);


			puts("Pido info a filesystem sobre el archivo a transformar");

					//	head.tipo_de_proceso=YAMA;
				//		head.tipo_de_mensaje=INFO_ARCHIVO;

			//			//envio la ruta del archivo a reducir a filesystem para que me devuelva la info del archivo
					//	buffer1=empaquetarBytes(&head,pathArchivoAReducir->bytes);
			//			puts("Path del archivo a reducir serializado; lo enviamos");
					//	if ((stat = send(socketFS, buffer1->buffer, buffer1->tamanio, 0)) == -1){
					//		puts("no se pudo enviar Path del archivo a reducir a FILESYSTEM. ");
					//		return;
						//}
			//			printf("se enviaron %d bytes del Path del archivo a reducir a FS\n",stat);


			//Espero a que FS me envie toda la informacion del archivo para seguir ejecutando

			//FS envia: cant de bloques del archivo (si es cero la ruta no es valida) para saber cuantos recv va a tener que hacer sobre
			//info de un bloque: nro de bloque + tamanioNombreNodo +copiacero nombrenodo+ copiacero nrobloquedatabin + tamanioNombreNodo +copiauno nombrenodo+ copiauno nrobloquedatabin
			//while((stat = recv(socketFS, &head, sizeof(Theader), 0))>0){
			//		if(head.tipo_de_mensaje==ARCH_NO_VALIDO){
			//			puts("no");
			//		}//else if(head.tipo_de_mensaje==INFO_ARCHIVO){
			//			int sizePaquete;
			/*			stat = recv(socketFS, &sizePaquete, sizeof(int), 0);
						Tbuffer *buffer = malloc(sizeof(Tbuffer));
						stat = recv(socketFS, buffer, sizePaquete, 0);
						TinfoArchivoFSYama *infoArchivo = malloc(sizeof(TinfoArchivoFSYama));

						infoArchivo=deserializarInfoArchivoYamaFS(buffer);
//seguior aca info nodos ....



				//	}
					//}

*/
			//Como este es el último atributo que recibimos de master.  Aca le enviamos la "info" de los workers a los que se tiene que conectar
			//Por el momento info hardcode y sera la info de los unicos 2 workers conectados. Se la pedimos al filesystem y cuando nos la devuelve, le reenviamos a master.
			/* pido info, la proceso y ahora se a que workers el master se va a conectar y se los paso...*/
			//x ahora solo le aviso q inice la transf local
			//Theader * head; esto no sabemos por que va


			t_list *listaComposicionArchivo=list_create();
			generarListaComposicionArchivoHardcode(listaComposicionArchivo);



			generarListaInfoNodos();


			sleep(retardoPlanificacionSegs);

			t_list *listaBloquesPlanificados=planificar(listaComposicionArchivo);


			//generarListaBloquesHardcode(listaBloquesPlanificados);

			if((stat=responderSolicTransf(sockMaster,listaBloquesPlanificados))<0){
				puts("No se pudo responder la solicitud de transferencia");
				return;
			}
			break;
		case FINTRANSFORMACIONLOCALOK:

			idTareaFinalizada = recibirValor(sockMaster);
			sleep(retardoPlanificacionSegs);
			printf("FINTRANSFORMACIONLOCAL OK de la tarea%d\n",idTareaFinalizada);
			puts("actuializo tbala de estados");
			moverAListaFinalizadosOK(idTareaFinalizada);


			if(sePuedeComenzarReduccionLocal(idTareaFinalizada)){
				comenzarReduccionLocal(idTareaFinalizada,sockMaster);
			}


			break;
		case FINTRANSFORMACIONLOCALFAIL:
			idTareaFinalizada = recibirValor(sockMaster);
			sleep(retardoPlanificacionSegs);
			printf("FINTRANSFORMACIONLOCAL FAIL de la tarea%d\n",idTareaFinalizada);
			puts("actuializo tbala de estados");

			moverAListaError(idTareaFinalizada);

			if(sePuedeReplanificar(idTareaFinalizada,listaComposicionArchivo)){

				stat = replanificar(idTareaFinalizada,sockMaster,listaComposicionArchivo);

				if(stat<0){
					//esto no deberia pasar nunca.. lo dejo aca para que no rompa tod
					printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
					puts("Se da x terminado el job");
					headEnvio->tipo_de_proceso=YAMA;
					headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
					enviarHeader(sockMaster,headEnvio);
					MUX_LOCK(&mux_listaJobFinalizados);
					if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
						TjobFinalizado *job = malloc(sizeof (TjobFinalizado));
						TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
						job->nroJob = tareaFinalizada->job;
						job->finCorrecto=false;
						list_add(listaJobFinalizados,job);
					}
					MUX_UNLOCK(&mux_listaJobFinalizados);
				}
			}else{

				printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
				puts("Se da x terminado el job");
				headEnvio->tipo_de_proceso=YAMA;
				headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
				enviarHeader(sockMaster,headEnvio);
				liberarCargaNodos(idTareaFinalizada);
				MUX_LOCK(&mux_listaJobFinalizados);
				if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
					TjobFinalizado *job = malloc(sizeof (TjobFinalizado));
					TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
					job->nroJob = tareaFinalizada->job;
					job->finCorrecto=false;
					list_add(listaJobFinalizados,job);
				}
				MUX_UNLOCK(&mux_listaJobFinalizados);
			}

			break;
		case FIN_REDUCCIONLOCALOK:
			idTareaFinalizada = recibirValor(sockMaster);
			sleep(retardoPlanificacionSegs);
			printf("REDUCCION LOCAL OK de la tarea %d\n",idTareaFinalizada);
			puts("actuializo tbala de estados");
			moverAListaFinalizadosOK(idTareaFinalizada);

			if(sePuedeComenzarReduccionGlobal(idTareaFinalizada)){
				comenzarReduccionGlobal(idTareaFinalizada,sockMaster);
			}

			break;
		case FIN_REDUCCIONLOCALFAIL:
			idTareaFinalizada = recibirValor(sockMaster);
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
			MUX_LOCK(&mux_listaJobFinalizados);
			if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
				TjobFinalizado *jobFinRL = malloc(sizeof (TjobFinalizado));
				TpackTablaEstados *tareaFinalizadaRL=getTareaPorId(idTareaFinalizada);
				jobFinRL->nroJob = tareaFinalizadaRL->job;
				jobFinRL->finCorrecto=false;
				list_add(listaJobFinalizados,jobFinRL);
			}
			MUX_UNLOCK(&mux_listaJobFinalizados);
			break;
		case FIN_REDUCCIONGLOBALOK:
			idTareaFinalizada = recibirValor(sockMaster);
			sleep(retardoPlanificacionSegs);
			printf("REDUCCION GLOBAL OK de la tarea %d\n",idTareaFinalizada);
			puts("actuializo tbala de estados");
			moverAListaFinalizadosOK(idTareaFinalizada);

			comenzarAlmacenadoFinal(idTareaFinalizada,sockMaster);

			break;
		case FIN_REDUCCIONGLOBALFAIL:
			idTareaFinalizada = recibirValor(sockMaster);
			sleep(retardoPlanificacionSegs);
			puts("fin reduccion global fail");
			puts("actuializo tbala de estados");
			moverAListaError(idTareaFinalizada);
			printf("La tarea %d no se puede replanificar ",idTareaFinalizada);
			puts("Se da x terminado el job");
			headEnvio->tipo_de_proceso=YAMA;
			headEnvio->tipo_de_mensaje=FINJOB_ERRORREPLANIFICACION;
			enviarHeader(sockMaster,headEnvio);
			liberarCargaNodos(idTareaFinalizada);
			MUX_LOCK(&mux_listaJobFinalizados);
			if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
				TjobFinalizado *jobFinRG = malloc(sizeof (TjobFinalizado));
				TpackTablaEstados *tareaFinalizadaRG=getTareaPorId(idTareaFinalizada);
				jobFinRG->nroJob = tareaFinalizadaRG->job;
				jobFinRG->finCorrecto=false;
				list_add(listaJobFinalizados,jobFinRG);
			}
			MUX_UNLOCK(&mux_listaJobFinalizados);
			break;
		case FIN_ALMACENAMIENTOFINALOK:
			idTareaFinalizada = recibirValor(sockMaster);
			sleep(retardoPlanificacionSegs);
			printf("Almacenamiento final de la tarea %d\n",idTareaFinalizada);
			puts("actuializo tbala de estados");
			moverAListaFinalizadosOK(idTareaFinalizada);
			liberarCargaNodos(idTareaFinalizada);

			break;
		case FIN_ALMACENAMIENTOFINALFAIL:
			idTareaFinalizada = recibirValor(sockMaster);
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
			MUX_LOCK(&mux_listaJobFinalizados);
			if(!yaFueAgregadoAlistaJobFinalizados(idTareaFinalizada)){
				TjobFinalizado *jobFinAF = malloc(sizeof (TjobFinalizado));
				TpackTablaEstados *tareaFinalizadaAF=getTareaPorId(idTareaFinalizada);
				jobFinAF->nroJob = tareaFinalizadaAF->job;
				jobFinAF->finCorrecto=false;
				list_add(listaJobFinalizados,jobFinAF);
			}
			MUX_UNLOCK(&mux_listaJobFinalizados);
			break;
		default:
			break;


		}
	}
	freeAndNULL((void **) &buffer);
}


int comenzarAlmacenadoFinal(int idTareaFinalizada,int sockMaster){

	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);

	MUX_LOCK(&mux_idTareaGlobal);
	int idTareaActual = idTareaGlobal++;
	MUX_UNLOCK(&mux_idTareaGlobal);

	TinfoAlmacenadoFinal *infoAlmacenado = malloc(sizeof(TinfoAlmacenadoFinal));
	infoAlmacenado->idTarea=idTareaActual;
	infoAlmacenado->job=tareaFinalizada->job;
	infoAlmacenado->ipNodo=malloc(MAXIMA_LONGITUD_IP);
	infoAlmacenado->ipNodo=getIpNodo(tareaFinalizada->nodo);
	infoAlmacenado->ipNodoLen=strlen(infoAlmacenado->ipNodo)+1;

	infoAlmacenado->puertoNodo=malloc(MAXIMA_LONGITUD_IP);
	infoAlmacenado->puertoNodo=getPuertoNodo(tareaFinalizada->nodo);
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





	agregarAlmacenadoFinalAListaEnProceso(infoAlmacenado,tareaFinalizada->nodo,tareaFinalizada->bloquesReducidos);


	return 0;
}
void agregarAlmacenadoFinalAListaEnProceso(TinfoAlmacenadoFinal *infoAlmacenado,char * nombreNodo,char * bloquesReducidos){
		MUX_LOCK(&mux_listaEnProceso);
		TpackTablaEstados * estado = malloc(sizeof (TpackTablaEstados));
		estado->idTarea=infoAlmacenado->idTarea;
		estado->job=infoAlmacenado->job;
		estado->master=idPropio;
		estado->bloqueDelArchivo=-1;
		estado->nodo=malloc(TAMANIO_NOMBRE_NODO);
		estado->nodo=nombreNodo;
		estado->etapa=ALMACENAMIENTOFINAL;
		estado->nombreArchTemporal=pathResultado->bytes;
		estado->bloquesReducidos=malloc(strlen(bloquesReducidos)+1);
		estado->bloquesReducidos=bloquesReducidos;
		list_add(listaEstadoEnProceso,estado);
		MUX_UNLOCK(&mux_listaEnProceso);
		mostrarTablaDeEstados();
}

int comenzarReduccionGlobal(int idTareaFinalizada,int sockMaster){

	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	int i,packSize,stat;
	char * buffer;
	int jobAReducir = tareaFinalizada->job;

	MUX_LOCK(&mux_idTareaGlobal);
	int idTareaActual = idTareaGlobal++;
	MUX_UNLOCK(&mux_idTareaGlobal);

	TreduccionGlobal *nuevaReduccion = malloc(sizeof (TreduccionGlobal));
	nuevaReduccion->idTarea=idTareaActual;
	nuevaReduccion->job=jobAReducir;
	nuevaReduccion->tempRedGlobal=malloc(TAMANIO_NOMBRE_TEMPORAL);
	nuevaReduccion->tempRedGlobal=generarNombreReduccionGlobalTemporal();
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
				infoNodoAux->ipNodo=getIpNodo(infoNodoAux->nombreNodo);
				infoNodoAux->ipNodoLen=strlen(infoNodoAux->ipNodo)+1;
				infoNodoAux->puertoNodo=malloc(MAXIMA_LONGITUD_PUERTO);
				infoNodoAux->puertoNodo=getPuertoNodo(infoNodoAux->nombreNodo);
				infoNodoAux->puertoNodoLen=strlen(infoNodoAux->puertoNodo)+1;
				infoNodoAux->temporalReduccion=malloc(TAMANIO_NOMBRE_TEMPORAL);
				infoNodoAux->temporalReduccion=tareaOk->nombreArchTemporal;
				infoNodoAux->temporalReduccionLen=strlen(infoNodoAux->temporalReduccion)+1;
				list_add(listaInformacionNodos,infoNodoAux);
				list_add(bloques,tareaOk->bloquesReducidos);


			}
		}

	asignarNodoElegido(listaInformacionNodos);


	nuevaReduccion->listaNodosSize=list_size(listaInformacionNodos);
	nuevaReduccion->listaNodos=listaInformacionNodos;

	Theader head;
	head.tipo_de_proceso=YAMA;
	head.tipo_de_mensaje=INFOREDUCCIONGLOBAL;


	packSize=0;
	buffer=serializeInfoReduccionGlobal(head,nuevaReduccion,&packSize);
	printf("Info de la reduccion global serializado, total %d bytes\n",packSize);
	if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la reduccion global. ");
		return  FALLO_SEND;
	}
	printf("se enviaron %d bytes de la info de la reduccion global\n",stat);


	char * bloquesReducidos = string_new();
	string_append(&bloquesReducidos,"{");
	for(i=0;i<list_size(bloques);i++){
		if(i!=0) string_append(&bloquesReducidos,", ");
		string_append(&bloquesReducidos,(list_get(bloques,i)));

	}
	string_append(&bloquesReducidos,"}");



	agregarReduccionGlobalAListaEnProceso(nuevaReduccion,bloquesReducidos);

	int cargaWorker = divideYRedondea(list_size(listaNodosDelJob),2);

	actualizarCargaWorkerEn(getNodoElegido(listaNodosDelJob),cargaWorker);
	aumentarHistoricoEn(getNodoElegido(listaNodosDelJob),cargaWorker);
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


int comenzarReduccionLocal(int idTareaFinalizada,int sockMaster){


	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	char * buffer;
	int packSize,stat;

	int jobAReducir = tareaFinalizada->job;
	MUX_LOCK(&mux_idTareaGlobal);
	int idTareaActual = idTareaGlobal++;
	MUX_UNLOCK(&mux_idTareaGlobal);

	char *  nodoReductor = tareaFinalizada->nodo;
	int i;

	TreduccionLocal * infoReduccion = malloc(sizeof(TreduccionLocal));
	infoReduccion->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
	infoReduccion->nombreNodo=nodoReductor;
	infoReduccion->nombreNodoLen=strlen(infoReduccion->nombreNodo)+1;
	infoReduccion->ipNodo=malloc(MAXIMA_LONGITUD_IP);
	infoReduccion->ipNodo=getIpNodo(nodoReductor);
	infoReduccion->ipLen=strlen(infoReduccion->ipNodo)+1;
	infoReduccion->puertoNodo=malloc(MAXIMA_LONGITUD_PUERTO);
	infoReduccion->puertoNodo=getPuertoNodo(nodoReductor);
	infoReduccion->puertoLen=strlen(infoReduccion->puertoNodo)+1;
	infoReduccion->tempRed=malloc(TAMANIO_NOMBRE_TEMPORAL);
	infoReduccion->tempRed=generarNombreReductorTemporal(nodoReductor);
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

	agregarReduccionLocalAListaEnProceso(infoReduccion,bloquesReducidos);

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




char *  generarNombreTemporal(){

	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append(&temp,string_itoa(idPropio));
	string_append(&temp,"-temp");
	string_append(&temp,string_itoa(idTempName++));


	return temp;
}

char *  generarNombreReductorTemporal(char * nombreNodo){

	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append(&temp,string_itoa(idPropio));
	string_append(&temp,"-");
	string_append(&temp,nombreNodo);


	return temp;
}


char *  generarNombreReduccionGlobalTemporal(){

	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append(&temp,string_itoa(idPropio));
	string_append(&temp,"-final");



	return temp;
}

int responderSolicTransf(int sockMaster,t_list * listaBloques){


	//aca planifico, pido la inf del archivo,etcetc

	//x ahora envio una rta hardcodeada pero siguiendo el formato
	int i,packSize,stat,jobActual;
	char *buffer;


	MUX_LOCK(&mux_jobIdGlobal);
	jobActual=idJobGlobal++;
	MUX_UNLOCK(&mux_jobIdGlobal);

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
		MUX_LOCK(&mux_idTareaGlobal);
		bloqueAEnviar->idTarea = idTareaGlobal++;
		MUX_UNLOCK(&mux_idTareaGlobal);
		buffer=serializeInfoBloque(head,bloqueAEnviar,&packSize);

		printf("Info del bloque %d serializado, enviamos\n",bloqueAEnviar->bloqueDelArchivo);

		if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
			puts("no se pudo enviar info del bloque. ");
			return  FALLO_SEND;
		}
		printf("se enviaron %d bytes de la info del bloque\n",stat);

		agregarTransformacionAListaEnProceso(jobActual,bloqueAEnviar->idTarea,bloqueAEnviar);


	}

	puts("Se envio la info de todos los bloques.");





	return 0;


}






bool sePuedeReplanificar(int idTarea,t_list * listaComposicionArchivo){
	TpackTablaEstados *tareaAReplanificar=getTareaPorId(idTarea);
	int i;
	if(tareaAReplanificar==NULL){
		puts("La tarea no existe en la tabla de estados");
		return false;
	}
	if(tareaAReplanificar->etapa==REDUCCIONGLOBAL || tareaAReplanificar->etapa==REDUCCIONLOCAL||tareaAReplanificar->etapa==ALMACENAMIENTOFINAL){
		puts("No existe posibilidad de replanificar tarea ya que se encontraba en etapa de Reduccion/AlmacenamientoFinal");
		return false;
	}
	if(tareaAReplanificar->fueReplanificada){
		puts("No se poseen nodos con copias de los datos. no se puede replanificar");
		return false;
	}
	for(i=0;i<list_size(listaComposicionArchivo);i++){
		TpackageUbicacionBloques * bloqueAux =list_get(listaComposicionArchivo,i);
		if(tareaAReplanificar->bloqueDelArchivo == bloqueAux->bloque){
			return true;
		}
	}
	puts("no se puede replanificar");
	return false;
}
int replanificar(int idTarea, int sockMaster,t_list * listaComposicionArchivo){

	sleep(retardoPlanificacionSegs);
	TpackTablaEstados *tareaAReplanificar=getTareaPorId(idTarea);
	int i,packSize,stat;
	char * buffer;

	if(tareaAReplanificar==NULL){
			puts("La tarea no existe en la tabla de estados");
			return FALLO_GRAL;
	}

	for(i=0;i<list_size(listaComposicionArchivo);i++){
		TpackageUbicacionBloques * bloqueAux =list_get(listaComposicionArchivo,i);
		if(tareaAReplanificar->bloqueDelArchivo == bloqueAux->bloque){
			TpackInfoBloque *bloqueRet=malloc(sizeof(TpackInfoBloque));
			if(tareaAReplanificar->nodo == bloqueAux->nombreNodoC1){
				//le paso la info para q labure en el otronodo

				bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC2;

				bloqueRet->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
				bloqueRet->nombreNodo=bloqueAux->nombreNodoC2;

				bloqueRet->puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
				bloqueRet->puertoWorker=getPuertoNodo(bloqueRet->nombreNodo);

				bloqueRet->ipWorker=malloc(MAXIMA_LONGITUD_IP);
				bloqueRet->ipWorker=getIpNodo(bloqueRet->nombreNodo);


			}else{
				//le paso la info para q trabaje en el otronodo
				bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC1;

				bloqueRet->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
				bloqueRet->nombreNodo=bloqueAux->nombreNodoC1;

				bloqueRet->puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
				bloqueRet->puertoWorker=getPuertoNodo(bloqueRet->nombreNodo);

				bloqueRet->ipWorker=malloc(MAXIMA_LONGITUD_IP);
				bloqueRet->ipWorker=getIpNodo(bloqueRet->nombreNodo);
			}

			bloqueRet->bloqueDelArchivo=tareaAReplanificar->bloqueDelArchivo;
			bloqueRet->bytesOcupados=bloqueAux->finBloque;
			MUX_LOCK(&mux_idTareaGlobal);
			bloqueRet->idTarea=idTareaGlobal++;
			MUX_UNLOCK(&mux_idTareaGlobal);
			bloqueRet->nombreTemporal=malloc(strlen(tareaAReplanificar->nombreArchTemporal)+1);
			tareaAReplanificar->fueReplanificada=true;



			Theader head;
			head.tipo_de_proceso=YAMA;
			head.tipo_de_mensaje=INFOBLOQUEREPLANIFICADO;
			packSize=0;
			buffer=serializeInfoBloque(head,bloqueRet,&packSize);

			printf("Info del bloque %d serializado, enviamos\n",bloqueRet->bloqueDelArchivo);

			if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
				puts("no se pudo enviar info del bloque. ");
				return  FALLO_SEND;
			}
			printf("se enviaron %d bytes de la info del bloque\n",stat);

			agregarTransformacionAListaEnProceso(tareaAReplanificar->job,bloqueRet->idTarea,bloqueRet);
			actualizarCargaWorkerEn(tareaAReplanificar->nodo,1);
			aumentarHistoricoEn(bloqueRet->nombreNodo,1);
			return 0;
		}
	}
	return FALLO_GRAL;

}

TpackTablaEstados * getTareaPorId(int idTarea){
	int i;
	MUX_LOCK(&mux_listaEnProceso);
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados * aux = list_get(listaEstadoEnProceso,i);
		if(aux->idTarea==idTarea){
			MUX_UNLOCK(&mux_listaEnProceso);
			return aux;
		}
	}
	MUX_UNLOCK(&mux_listaEnProceso);

	MUX_LOCK(&mux_listaError);
	for(i=0;i<list_size(listaEstadoError);i++){
			TpackTablaEstados * aux = list_get(listaEstadoError,i);
			if(aux->idTarea==idTarea){
				MUX_UNLOCK(&mux_listaError);
				return aux;
			}
		}
	MUX_UNLOCK(&mux_listaError);

	MUX_LOCK(&mux_listaFinalizado);
	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
			TpackTablaEstados * aux = list_get(listaEstadoFinalizadoOK,i);
			if(aux->idTarea==idTarea){
				MUX_UNLOCK(&mux_listaFinalizado);
				return aux;
			}
		}
	MUX_UNLOCK(&mux_listaFinalizado);
	printf("no se encontro %d en la tbala de estados\n",idTarea);
	return NULL;
}









/*void generarListaBloquesHardcode(t_list * listaBloques){


	//toda esta funcion va a volar, esta hecho asi nomas para ir probandolo. esta tod hardco



	int nombreLen=6;
	int ipLen=10;
	int puertoLen=5;
	int tmpLen=MAXSIZETEMPNAME;




	TpackInfoBloque *bloque1 = malloc(sizeof bloque1);


	bloque1->nombreNodo=malloc(nombreLen);
	bloque1->nombreNodo="Nodo1";
	bloque1->nombreLen=strlen(bloque1->nombreNodo)+1;
	bloque1->ipNodo=malloc(ipLen);
	bloque1->ipNodo="127.0.0.1";
	bloque1->ipLen=strlen(bloque1->ipNodo)+1;
	bloque1->puertoWorker=malloc(puertoLen);
	bloque1->puertoWorker = "5013";
	bloque1->puertoLen=strlen(bloque1->puertoWorker)+1;
	bloque1->bloque=38;
	bloque1->bytesOcupados=10180;
	bloque1->nombreTemporal=malloc(tmpLen);
	bloque1->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque1->nombreTemporalLen=strlen(bloque1->nombreTemporal) +1;
	list_add(listaBloques,bloque1);


	TpackInfoBloque *bloque2 = malloc(sizeof bloque2);
	bloque2->nombreNodo=malloc(nombreLen);
	bloque2->nombreNodo="Nodo1";

	bloque2->nombreLen=strlen(bloque2->nombreNodo)+1;
	bloque2->ipNodo=malloc(ipLen);
	bloque2->ipNodo="127.0.0.1";
	bloque2->ipLen=strlen(bloque2->ipNodo)+1;
	bloque2->puertoWorker=malloc(puertoLen);
	bloque2->puertoWorker = "5013";
	bloque2->puertoLen=strlen(bloque2->puertoWorker)+1;
	bloque2->bloque=39;
	bloque2->bytesOcupados=1048576;
	bloque2->nombreTemporal=malloc(tmpLen);
	bloque2->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque2->nombreTemporalLen=strlen(bloque2->nombreTemporal) +1;
	list_add(listaBloques,bloque2);




	TpackInfoBloque *bloque3 = malloc(sizeof bloque3);
	bloque3->nombreNodo=malloc(nombreLen);
	bloque3->nombreNodo="Nodo1";
	bloque3->nombreLen=strlen(bloque3->nombreNodo)+1;
	bloque3->ipNodo=malloc(ipLen);
	bloque3->ipNodo="127.0.0.1";
	bloque3->ipLen=strlen(bloque3->ipNodo)+1;
	bloque3->puertoWorker=malloc(puertoLen);
	bloque3->puertoWorker = "5013";
	bloque3->puertoLen=strlen(bloque3->puertoWorker)+1;
	bloque3->bloque=44;
	bloque3->bytesOcupados=1048576;
	bloque3->nombreTemporal=malloc(tmpLen);
	bloque3->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque3->nombreTemporalLen=strlen(bloque3->nombreTemporal) +1;
	list_add(listaBloques,bloque3);




	TpackInfoBloque *bloque4 = malloc(sizeof bloque4);
	bloque4->nombreNodo=malloc(nombreLen);
	bloque4->nombreNodo="Nodo1";
	bloque4->nombreLen=strlen(bloque4->nombreNodo)+1;
	bloque4->ipNodo=malloc(ipLen);
	bloque4->ipNodo="127.0.0.1";
	bloque4->ipLen=strlen(bloque4->ipNodo)+1;
	bloque4->puertoWorker=malloc(puertoLen);
	bloque4->puertoWorker = "5013";
	bloque4->puertoLen=strlen(bloque4->puertoWorker)+1;
	bloque4->bloque=39;
	bloque4->bytesOcupados=1048576;
	bloque4->nombreTemporal=malloc(tmpLen);
	bloque4->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque4->nombreTemporalLen=strlen(bloque4->nombreTemporal) +1;
	list_add(listaBloques,bloque4);




	TpackInfoBloque *bloque5 = malloc(sizeof bloque5);
	bloque5->nombreNodo=malloc(nombreLen);
	bloque5->nombreNodo="Nodo1";
	bloque5->nombreLen=strlen(bloque5->nombreNodo)+1;
	bloque5->ipNodo=malloc(ipLen);
	bloque5->ipNodo="127.0.0.1";
	bloque5->ipLen=strlen(bloque1->ipNodo)+1;
	bloque5->puertoWorker=malloc(puertoLen);
	bloque5->puertoWorker = "5013";
	bloque5->puertoLen=strlen(bloque5->puertoWorker)+1;
	bloque5->bloque=46;
	bloque5->bytesOcupados=1048576;
	bloque5->nombreTemporal=malloc(tmpLen);
	bloque5->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque5->nombreTemporalLen=strlen(bloque5->nombreTemporal) +1;
	list_add(listaBloques,bloque5);



}*/

void generarListaComposicionArchivoHardcode(t_list * listaComposicion){

	//toda esta funcion va a volar, esta hecho asi nomas para ir probandolo. esta tod hardco
	int maxNombreNodo=10;
	TpackageUbicacionBloques *bloque0 = malloc(sizeof(TpackageUbicacionBloques));
	bloque0->bloque=0;
	bloque0->nombreNodoC1=malloc(sizeof(maxNombreNodo));
	bloque0->nombreNodoC1="Nodo1";
	bloque0->nombreNodoC1Len=strlen(bloque0->nombreNodoC1)+1;
	bloque0->bloqueC1=5;
	bloque0->nombreNodoC2=malloc(sizeof(maxNombreNodo));
	bloque0->nombreNodoC2="Nodo2";
	bloque0->nombreNodoC2Len=strlen(bloque0->nombreNodoC2)+1;
	bloque0->bloqueC2=2;
	bloque0->finBloque=1048576;

	list_add(listaComposicion,bloque0);

	TpackageUbicacionBloques *bloque1 = malloc(sizeof(TpackageUbicacionBloques));
	bloque1->bloque=1;
	bloque1->nombreNodoC1=malloc(sizeof(maxNombreNodo));
	bloque1->nombreNodoC1="Nodo2";
	bloque1->nombreNodoC1Len=strlen(bloque1->nombreNodoC1)+1;
	bloque1->bloqueC1=10;
	bloque1->nombreNodoC2=malloc(sizeof(maxNombreNodo));
	bloque1->nombreNodoC2="Nodo3";
	bloque1->nombreNodoC2Len=strlen(bloque1->nombreNodoC2)+1;
	bloque1->bloqueC2=7;
	bloque1->finBloque=1048500;
	list_add(listaComposicion,bloque1);

	TpackageUbicacionBloques *bloque2 = malloc(sizeof(TpackageUbicacionBloques));
	bloque2->bloque=2;
	bloque2->nombreNodoC1=malloc(sizeof(maxNombreNodo));
	bloque2->nombreNodoC1="Nodo3";
	bloque2->nombreNodoC1Len=strlen(bloque2->nombreNodoC1)+1;
	bloque2->bloqueC1=12;
	bloque2->nombreNodoC2=malloc(sizeof(maxNombreNodo));
	bloque2->nombreNodoC2="Nodo1";
	bloque2->nombreNodoC2Len=strlen(bloque2->nombreNodoC2)+1;
	bloque2->bloqueC2=3;
	bloque2->finBloque=1048516;
	list_add(listaComposicion,bloque2);

}

void generarListaInfoNodos(){



	int nombreLen=6;
	int ipLen=10;
	int puertoLen=5;


	TpackageInfoNodo *nodo1 = malloc(sizeof(TpackageInfoNodo));
	nodo1->nombreNodo=malloc(nombreLen);
	nodo1->nombreNodo="Nodo1";
	nodo1->tamanioNombre=strlen(nodo1->nombreNodo)+1;
	nodo1->ipNodo=malloc(ipLen);
	nodo1->ipNodo="192.168.3.24";
	nodo1->tamanioIp=strlen(nodo1->ipNodo)+1;
	nodo1->puertoWorker=malloc(puertoLen);
	nodo1->puertoWorker = "5013";
	nodo1->tamanioPuerto=strlen(nodo1->puertoWorker)+1;
	list_add(listaNodosDelJob,nodo1);

	TpackageInfoNodo *nodo2 = malloc(sizeof(TpackageInfoNodo));
	nodo2->nombreNodo=malloc(nombreLen);
	nodo2->nombreNodo="Nodo2";
	nodo2->tamanioNombre=strlen(nodo2->nombreNodo)+1;
	nodo2->ipNodo=malloc(ipLen);
	nodo2->ipNodo="192.168.3.22";
	nodo2->tamanioIp=strlen(nodo2->ipNodo)+1;
	nodo2->puertoWorker=malloc(puertoLen);
	nodo2->puertoWorker = "5014";
	nodo2->tamanioPuerto=strlen(nodo2->puertoWorker)+1;

	list_add(listaNodosDelJob,nodo2);

	TpackageInfoNodo *nodo3 = malloc(sizeof(TpackageInfoNodo));
	nodo3->nombreNodo=malloc(nombreLen);
	nodo3->nombreNodo="Nodo3";
	nodo3->tamanioNombre=strlen(nodo3->nombreNodo)+1;
	nodo3->ipNodo=malloc(ipLen);
	nodo3->ipNodo="192.168.3.2";
	nodo3->tamanioIp=strlen(nodo3->ipNodo)+1;
	nodo3->puertoWorker=malloc(puertoLen);
	nodo3->puertoWorker = "5034";
	nodo3->tamanioPuerto=strlen(nodo3->puertoWorker)+1;

	list_add(listaNodosDelJob,nodo3);


}



/////////////////////////////////planificacion




bool yaFueAgregadoAlistaJobFinalizados(int idTareaFinalizada){
	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	int i;
	MUX_LOCK(&mux_listaJobFinalizados);
	for(i=0;i<list_size(listaJobFinalizados);i++){
		TjobFinalizado *job=list_get(listaJobFinalizados,i);
		if(job->nroJob==tareaFinalizada->job){
			MUX_UNLOCK(&mux_listaJobFinalizados);
			return true;
		}
	}
	MUX_UNLOCK(&mux_listaJobFinalizados);
	return false;
}


void liberarCargaNodos(int idTareaFinalizada){




	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	int jobALiberar = tareaFinalizada->job;

	//primero me fijo q no haya sido agregado a la lista de job finalizados, en ese caso ya se liberaron las cargas

	if(!yaFueFinalizadoPorErrorDeReplanificacion(jobALiberar)){


		//reduzco de todas las etpas q estan en proceso.
		int i;

		MUX_LOCK(&mux_listaEnProceso);
		for(i=0;i<list_size(listaEstadoEnProceso);i++){
			TpackTablaEstados *aux  = list_get(listaEstadoEnProceso,i);
			if(aux->idTarea==jobALiberar){
				if(aux->etapa==TRANSFORMACION){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONLOCAL){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONGLOBAL){
					int cargaAReducir = getCargaReduccionGlobal(jobALiberar);
					liberarCargaEn(aux->nodo,cargaAReducir);
				}
			}
		}
		MUX_UNLOCK(&mux_listaEnProceso);

		MUX_LOCK(&mux_listaFinalizado);
		for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
			TpackTablaEstados *aux  = list_get(listaEstadoFinalizadoOK,i);
			if(aux->idTarea==jobALiberar){
				if(aux->etapa==TRANSFORMACION){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONLOCAL){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONGLOBAL){
					int cargaAReducir = getCargaReduccionGlobal(jobALiberar);
					liberarCargaEn(aux->nodo,cargaAReducir);

				}
			}
		}
		MUX_UNLOCK(&mux_listaFinalizado);

		MUX_LOCK(&mux_listaError);
		for(i=0;i<list_size(listaEstadoError);i++){
			TpackTablaEstados *aux  = list_get(listaEstadoError,i);
			if(aux->idTarea==jobALiberar){
				if(aux->etapa==TRANSFORMACION){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONLOCAL){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONGLOBAL){
					int cargaAReducir = getCargaReduccionGlobal(jobALiberar);
					liberarCargaEn(aux->nodo,cargaAReducir);
				}
			}
		}
		MUX_UNLOCK(&mux_listaError);
		mostrarTablaDeEstados();


	}

}
void liberarCargaEn(char * nombreNodo,int cantidad){
	int i;
	MUX_LOCK(&mux_listaCargaGlobal);
	for(i=0;i<list_size(listaCargaGlobal);i++){
		TcargaGlobal *cargaNodo = list_get(listaCargaGlobal,i);
		cargaNodo->cargaGlobal-=cantidad;
		break;
	}
	MUX_UNLOCK(&mux_listaCargaGlobal);
}

void agregarReduccionGlobalAListaEnProceso(TreduccionGlobal *infoReduccion,char * bloquesReducidos){
		MUX_LOCK(&mux_listaEnProceso);
		TpackTablaEstados * estado = malloc(sizeof (TpackTablaEstados));
		estado->idTarea=infoReduccion->idTarea;
		estado->job=infoReduccion->job;
		estado->master=idPropio;
		estado->bloqueDelArchivo=-1;
		estado->nodo=malloc(TAMANIO_NOMBRE_NODO);
		estado->nodo=getNodoElegido(infoReduccion->listaNodos);
		estado->etapa=REDUCCIONGLOBAL;
		estado->nombreArchTemporal=infoReduccion->tempRedGlobal;
		estado->bloquesReducidos=malloc(strlen(bloquesReducidos)+1);
		estado->bloquesReducidos=bloquesReducidos;
		list_add(listaEstadoEnProceso,estado);
		MUX_UNLOCK(&mux_listaEnProceso);
		mostrarTablaDeEstados();
}
bool esNodoEncargado(char * nombreNodo){
	return false;
}
char * getNodoElegido(t_list * listaNodos){
	int i;
	for(i=0;i<list_size(listaNodos);i++){
		TinfoNodoReduccionGlobal *nodo =list_get(listaNodos,i);
		if(nodo->nodoEncargado==1){
			return nodo->nombreNodo;
		}
	}
	return "null";
}

bool yaFueFinalizadoPorErrorDeReplanificacion(int job){
	MUX_LOCK(&mux_listaJobFinalizados);
	int i;
	for(i=0;i<list_size(listaJobFinalizados);i++){
		TjobFinalizado *jobAux = list_get(listaJobFinalizados,i);
		if(jobAux->nroJob == job){

			MUX_UNLOCK(&mux_listaJobFinalizados);
			return true;
		}
	}
	MUX_UNLOCK(&mux_listaJobFinalizados);

	return false;
}

int moverAListaFinalizadosOK(int idTareaFinalizada){

	int i;
	MUX_LOCK(&mux_listaEnProceso);
	MUX_LOCK(&mux_listaFinalizado);
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados *aux  = list_get(listaEstadoEnProceso,i);
		if(aux->idTarea==idTareaFinalizada){
			list_add(listaEstadoFinalizadoOK,aux);
			list_remove(listaEstadoEnProceso,i);
			mostrarTablaDeEstados();
			MUX_UNLOCK(&mux_listaFinalizado);
			MUX_UNLOCK(&mux_listaEnProceso);
			return 0;
		}
	}
	MUX_UNLOCK(&mux_listaFinalizado);
	MUX_UNLOCK(&mux_listaEnProceso);
	mostrarTablaDeEstados();

	return -1;
}

int moverAListaError (int idTareaFinalizada){

	int i;
	MUX_LOCK(&mux_listaEnProceso);
	MUX_LOCK(&mux_listaError);
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados *aux  = list_get(listaEstadoEnProceso,i);
		if(aux->idTarea==idTareaFinalizada){
			list_add(listaEstadoError,aux);
			list_remove(listaEstadoEnProceso,i);
			mostrarTablaDeEstados();
			MUX_UNLOCK(&mux_listaError);
			MUX_UNLOCK(&mux_listaEnProceso);
			return 0;
		}
	}

	MUX_UNLOCK(&mux_listaError);
	MUX_UNLOCK(&mux_listaEnProceso);
	mostrarTablaDeEstados();
	return -1;
}

void agregarReduccionLocalAListaEnProceso(TreduccionLocal * infoReduccion,char * bloquesReducidos){
	MUX_LOCK(&mux_listaEnProceso);


	TpackTablaEstados * estado = malloc(sizeof (TpackTablaEstados));
	estado->idTarea=infoReduccion->idTarea;
	estado->job=infoReduccion->job;
	estado->master=idPropio;
	estado->bloqueDelArchivo=-1;
	estado->nodo=malloc(TAMANIO_NOMBRE_NODO);
	estado->nodo=infoReduccion->nombreNodo;
	estado->etapa=REDUCCIONLOCAL;
	estado->nombreArchTemporal=infoReduccion->tempRed;
	estado->bloquesReducidos=malloc(strlen(bloquesReducidos)+1);
	estado->bloquesReducidos=bloquesReducidos;
	list_add(listaEstadoEnProceso,estado);
	MUX_UNLOCK(&mux_listaEnProceso);
	mostrarTablaDeEstados();
}

void agregarTransformacionAListaEnProceso(int jobActual, int idTarea, TpackInfoBloque *bloque){

	MUX_LOCK(&mux_listaEnProceso);
	TpackTablaEstados * estado = malloc(sizeof (TpackTablaEstados));
	estado->idTarea=idTarea;
	estado->job=jobActual;
	estado->master= idPropio;
	estado->nodo=malloc(TAMANIO_NOMBRE_NODO);
	estado->nodo=bloque->nombreNodo;
	estado->bloqueDelArchivo=bloque->bloqueDelArchivo;
	estado->etapa=TRANSFORMACION;
	estado->nombreArchTemporal=malloc(TAMANIO_NOMBRE_TEMPORAL);
	estado->nombreArchTemporal=bloque->nombreTemporal;
	list_add(listaEstadoEnProceso,estado);
	MUX_UNLOCK(&mux_listaEnProceso);

	mostrarTablaDeEstados();

}

void mostrarTablaDeEstados(){
	printf("\n\n\n");
	puts("Hubo un cambio de estado, va la tabla actual:");
	int i;
	int id,job,master;
	char * bloque;
	char * etapa;
	char * nodo;
	char * archivoTemporal;
	printf("%-4s%-4s%-7s%-7s%-25s%-15s%-40s%-10s\n", "id", "Job", "Master", "Nodo", "Bloque","Etapa","A Temporal","Estado");
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados * aux = list_get(listaEstadoEnProceso,i);
		id = aux->idTarea;
		job=aux->job;
		master = aux->master;
		if(aux->bloqueDelArchivo != -1){
			bloque=string_itoa(aux->bloqueDelArchivo);
		}else{
			bloque = aux->bloquesReducidos;
		}
		etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"E Pr");
	}

	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
		TpackTablaEstados * aux = list_get(listaEstadoFinalizadoOK,i);
		id = aux->idTarea;
		job=aux->job;
		master = aux->master;
		if(aux->bloqueDelArchivo != -1){
			bloque=string_itoa(aux->bloqueDelArchivo);
		}else{
			bloque = aux->bloquesReducidos;
		}
		etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"FinOK");
	}

	for(i=0;i<list_size(listaEstadoError);i++){
		TpackTablaEstados * aux = list_get(listaEstadoError,i);
		id = aux->idTarea;
		job=aux->job;
		master = aux->master;
		if(aux->bloqueDelArchivo != -1){
			bloque=string_itoa(aux->bloqueDelArchivo);
		}else{
			bloque = aux->bloquesReducidos;
		}
		etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"ERR");
	}

	printf("\n\n\n");
}

char  * getNombreEtapa(int etapaEnum){
	char * ret=malloc(6);
	if(etapaEnum==TRANSFORMACION){
		ret="Tra";
	}else if(etapaEnum==REDUCCIONGLOBAL){
		ret="R Gl";
	}else if(etapaEnum==REDUCCIONLOCAL){
		ret="R Lo";
	}else if(etapaEnum==ALMACENAMIENTOFINAL){
			ret="A F";
	}else{
		ret = " ";
	}
	return ret;
}




char * getIpNodo(char * nombreNodo){
	TpackageInfoNodo *nodoAux;
	int i;
	for(i=0;i<list_size(listaNodosDelJob);i++){
		nodoAux=list_get(listaNodosDelJob,i);
		if(nodoAux->nombreNodo == nombreNodo){
			return nodoAux->ipNodo;
		}
	}
	return NULL;
}

char * getPuertoNodo(char * nombreNodo){
	TpackageInfoNodo *nodoAux;
	int i;
	for(i=0;i<list_size(listaNodosDelJob);i++){
		nodoAux=list_get(listaNodosDelJob,i);
		if(nodoAux->nombreNodo == nombreNodo){
			return nodoAux->puertoWorker;
		}
	}
	return NULL;
}



t_list * planificar(t_list * listaComposicionArchivo){




	t_list * listaPlanificada=list_create();
	t_list * listaWorkersPlanificacion = list_create();





	int stat,pwl;
	TpackageInfoNodo *aux;
	//lleno la lista con los workers asociados a esta transfo.
	int i;
	for(i=0;i<list_size(listaNodosDelJob);i++){
		aux = list_get(listaNodosDelJob,i);
		Tplanificacion * nodo = malloc(sizeof (Tplanificacion));
		nodo->infoNodo.nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
		nodo->infoNodo.nombreNodo=aux->nombreNodo;
		nodo->infoNodo.tamanioNombre=aux->tamanioNombre;
		nodo->infoNodo.ipNodo=malloc(MAXIMA_LONGITUD_IP);
		nodo->infoNodo.ipNodo=aux->ipNodo;
		nodo->infoNodo.tamanioIp=aux->tamanioIp;
		nodo->infoNodo.puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
		nodo->infoNodo.puertoWorker=aux->puertoWorker;
		nodo->infoNodo.tamanioPuerto=aux->tamanioPuerto;
		nodo->disponibilidadBase=yama->disponibilidadBase;

		if(yama->algoritmo_balanceo==WCLOCK){
			int wlMax=getMayorCargaAllWorkers();
			int wl = getCargaWorker(aux->nombreNodo);
			pwl=wlMax-wl;
		}else{
			pwl=0;
		}

		nodo->pwl=pwl;
		nodo->availability=nodo->disponibilidadBase+nodo->pwl;
		nodo->clock=false;
		list_add(listaWorkersPlanificacion,nodo);
	}

	if((stat = posicionarClock(listaWorkersPlanificacion))<0){
		puts("error");
	}
	int k;
	for(k=0;k<list_size(listaComposicionArchivo);k++){
		TpackageUbicacionBloques *bloqueAux=list_get(listaComposicionArchivo,k);

		TpackInfoBloque *bloque = asignarBloque(bloqueAux,listaWorkersPlanificacion);

		list_add(listaPlanificada,bloque);


	}

	int q;
	for(q=0;q<list_size(listaPlanificada);q++){
		TpackInfoBloque *aux = list_get(listaPlanificada,q);
		printf("%d %d %s\n",aux->bloqueDelDatabin,aux->bytesOcupados,aux->nombreNodo);
	}


	return listaPlanificada;
}

void actualizarCargaWorkerEn(char * nombreNodo, int cantidadAAumentar){
	MUX_LOCK(&mux_listaCargaGlobal);
	int i;
	for(i=0;i<list_size(listaCargaGlobal);i++){
		TcargaGlobal *aux = list_get(listaCargaGlobal,i);
		if(aux->nombreNodo == nombreNodo){
			aux->cargaGlobal +=cantidadAAumentar;
			MUX_UNLOCK(&mux_listaCargaGlobal);
			return;
		}
	}

	MUX_UNLOCK(&mux_listaCargaGlobal);
}
void aumentarHistoricoEn(char * nombreNodo,int cantidadAAumentar){
	int i;
	MUX_LOCK(&mux_listaHistorica);
	for(i=0;i<list_size(listaHistoricaTareas);i++){
		ThistorialTareas *historial = list_get(listaHistoricaTareas,i);
		if(historial->nombreNodo == nombreNodo){
			historial->tareasRealizadas+=cantidadAAumentar;
		}

	}
	MUX_UNLOCK(&mux_listaHistorica);

}

TpackInfoBloque * asignarBloque(TpackageUbicacionBloques *bloqueAux,t_list *listaWorkersPlanificacion){


	Tplanificacion *nodoApuntado = getNodoApuntado(listaWorkersPlanificacion);
	TpackInfoBloque *bloqueRet=malloc(sizeof(TpackInfoBloque));

	if((nodoApuntado->availability > 0 && bloqueAux->nombreNodoC1==nodoApuntado->infoNodo.nombreNodo) ||
			(nodoApuntado->availability > 0 && bloqueAux->nombreNodoC2==nodoApuntado->infoNodo.nombreNodo)){
		if(bloqueAux->nombreNodoC1==nodoApuntado->infoNodo.nombreNodo ){
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC1;
		}
		if(bloqueAux->nombreNodoC2==nodoApuntado->infoNodo.nombreNodo ){
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC2;
		}
		mergeBloque(bloqueRet,nodoApuntado,bloqueAux);
		nodoApuntado->availability-=1;

		avanzarClock(listaWorkersPlanificacion);

	}else{
		Tplanificacion *siguienteNodo = getSiguienteNodoDisponible(listaWorkersPlanificacion,bloqueAux->nombreNodoC1,bloqueAux->nombreNodoC2);

		if(siguienteNodo->infoNodo.nombreNodo == bloqueAux->nombreNodoC1){
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC1;
		}else{
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC2;
		}

		mergeBloque(bloqueRet,siguienteNodo,bloqueAux);
		siguienteNodo->availability-=1;

	}
	aumentarHistoricoEn(bloqueRet->nombreNodo,1);
	actualizarCargaWorkerEn(bloqueRet->nombreNodo,1);


	return bloqueRet;
}


void avanzarClock(t_list *listaWorkersPlanificacion){
	int i;
	Tplanificacion *siguiente;
	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux=list_get(listaWorkersPlanificacion,i);
		if(aux->clock){
			if(i+1!=list_size(listaWorkersPlanificacion)){
				siguiente=list_get(listaWorkersPlanificacion,i+1);
			}else{
				siguiente=list_get(listaWorkersPlanificacion,0);
			}
			aux->clock=false;
			siguiente->clock=true;
			if(siguiente->availability==0){
				siguiente->availability=siguiente->disponibilidadBase;
				avanzarClock(listaWorkersPlanificacion);
			}
			break;
		}
	}
}
void mergeBloque(TpackInfoBloque *bloqueRet,Tplanificacion *nodoApuntado,TpackageUbicacionBloques *bloqueAux){



	bloqueRet->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
	bloqueRet->nombreNodo=nodoApuntado->infoNodo.nombreNodo;
	bloqueRet->tamanioNombre=nodoApuntado->infoNodo.tamanioNombre;
	bloqueRet->nombreTemporal=malloc(TAMANIO_NOMBRE_TEMPORAL);
	bloqueRet->nombreTemporal=generarNombreTemporal();
	bloqueRet->nombreTemporalLen=strlen(bloqueRet->nombreTemporal)+1;
	bloqueRet->bytesOcupados=bloqueAux->finBloque;
	bloqueRet->ipWorker=malloc(MAXIMA_LONGITUD_IP);
	bloqueRet->ipWorker=nodoApuntado->infoNodo.ipNodo;
	bloqueRet->tamanioIp=nodoApuntado->infoNodo.tamanioIp;
	bloqueRet->puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
	bloqueRet->puertoWorker=nodoApuntado->infoNodo.puertoWorker;
	bloqueRet->tamanioPuerto=nodoApuntado->infoNodo.tamanioPuerto;
	bloqueRet->bloqueDelArchivo = bloqueAux->bloque;

}

Tplanificacion * getNodoApuntado(t_list * listaWorkersPlanificacion){
	//todo chequeo errores
	int i;
	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux= list_get(listaWorkersPlanificacion,i);
		if(aux->clock){
			return aux;
		}
	}
	return NULL;
}
Tplanificacion * getSiguienteNodoDisponible(t_list * listaWorkersPlanificacion,char * nombreNodo1,char* nombreNodo2){
	//todo chequeo errores
	int i;

	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux= list_get(listaWorkersPlanificacion,i);
		if(aux->clock){
			Tplanificacion *siguienteDisponible = getSiguienteConDisponibilidadPositivaPosible(listaWorkersPlanificacion,i,nombreNodo1,nombreNodo2);
			return siguienteDisponible;

		}
	}


	return NULL;
}


Tplanificacion * getSiguienteConDisponibilidadPositivaPosible(t_list * listaWorkersPlanificacion, int indice,char * nombre1,char * nombre2){

	int i;
	for(i=indice+1;i<list_size(listaWorkersPlanificacion)+indice;i++){
		if(i>=list_size(listaWorkersPlanificacion)){
			i=0;
		}
		Tplanificacion *aux = list_get(listaWorkersPlanificacion,i);
		if(aux->availability>0){
			if(i!= indice){
				if(aux->infoNodo.nombreNodo == nombre1 ||aux->infoNodo.nombreNodo == nombre2 ){
					return aux;
				}
			}else{
				sumarDisponibilidadBaseATodos(listaWorkersPlanificacion);
				i=indice+1;
			}
		}
	}

	return NULL;
}

void sumarDisponibilidadBaseATodos(t_list * listaWorkersPlanificacion){
	int i;
	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux = list_get(listaWorkersPlanificacion,i);
		aux->availability+=aux->disponibilidadBase;
	}
}
int posicionarClock(t_list *listaWorkers){

	int i;
	int disponibilidadMasAlta = -1;
	bool empate=false;
	for(i=0;i<list_size(listaWorkers);i++){
		Tplanificacion *aux = list_get(listaWorkers,i);
		if(aux->availability > disponibilidadMasAlta){
			disponibilidadMasAlta=aux->availability;
			empate=false;
		}else if(aux->availability==disponibilidadMasAlta){
			empate=true;
		}
	}
	if(empate){
		desempatarClock(disponibilidadMasAlta,listaWorkers);
	}



	return 0;
}

int desempatarClock(int disponibilidadMasAlta,t_list * listaWorkers){

	int i;
	int historico1=-1;
	int historico2=-1;
	int indiceAModificar;
	Tplanificacion *aux;
	for(i=0;i<list_size(listaWorkers);i++){
		aux = list_get(listaWorkers,i);
		if(aux->availability == disponibilidadMasAlta){
			historico1=getHistorico(aux);
		}
		if(historico1>historico2){ //todo cambio (?????????????)
			historico2=historico1;
			indiceAModificar=i;
		}

	}
	aux=list_get(listaWorkers,indiceAModificar);
	aux->clock=true;

	return 0;
}

int getHistorico(Tplanificacion *infoWorker){
	int i;
	MUX_LOCK(&mux_listaHistorica);
	for(i=0;i<list_size(listaHistoricaTareas);i++){
		ThistorialTareas *aux = list_get(listaHistoricaTareas,i);
		if(infoWorker->infoNodo.nombreNodo==aux->nombreNodo){
			MUX_UNLOCK(&mux_listaHistorica);
			return aux->tareasRealizadas;
		}
	}
	MUX_UNLOCK(&mux_listaHistorica);
	return 0;
}

int getCargaWorker(char * nombreWorker){
	int i;
	int cargaRet =0;
	for(i=0;i<list_size(listaCargaGlobal);i++){
		TcargaGlobal * cargaWorker = list_get(listaCargaGlobal,i);
		if(string_equals_ignore_case(cargaWorker->nombreNodo,nombreWorker)){
			cargaRet=cargaWorker->cargaGlobal;
		}
	}

	return cargaRet;
}

int getMayorCargaAllWorkers(){

	int mayorCarga=0;
	int i;
	for(i=0;i<list_size(listaCargaGlobal);i++){
		TcargaGlobal * cargaWorker = list_get(listaCargaGlobal,i);
		if(cargaWorker->cargaGlobal>mayorCarga){
			mayorCarga=cargaWorker->cargaGlobal;
		}
	}

	return mayorCarga;

}

void asignarNodoElegido(t_list * listaReduccionGlobal){
	TinfoNodoReduccionGlobal *aux = list_get(listaReduccionGlobal,0);
	char * nodoElegido=aux->nombreNodo;
	int menorCarga =getCargaWorker(nodoElegido);
	int cargaAux;
	int i;
	for(i=0;i<list_size(listaReduccionGlobal);i++){
		aux=list_get(listaReduccionGlobal,i);
		cargaAux=getCargaWorker(aux->nombreNodo);
		if(cargaAux<menorCarga){
			menorCarga=cargaAux;
			nodoElegido=aux->nombreNodo;
		}
	}

	for(i=0;i<list_size(listaReduccionGlobal);i++){
		aux=list_get(listaReduccionGlobal,i);
		if(aux->nombreNodo==nodoElegido){
			aux->nodoEncargado=1;
		}
	}

	printf("Nodo encargado: %s. menor carga: %d \n",nodoElegido,menorCarga);

	return;
}
int getCargaReduccionGlobal(int job){
	int carga=0;
	int i;
	TpackTablaEstados * estado;
	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
		estado = list_get(listaEstadoFinalizadoOK,i);
		if(estado->etapa==REDUCCIONLOCAL){
			carga++;
		}
	}

	printf("Libero carga en %d\n",divideYRedondea(carga,2));
	return divideYRedondea(carga,2);
}
