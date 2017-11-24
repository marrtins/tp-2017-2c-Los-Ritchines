/*
 * workerTransf.c
 *
 *  Created on: 26/10/2017
 *      Author: utnso
 */

#include "funcionesMS.h"

extern char * rutaTransformador;

extern int cantFallos;
extern pthread_mutex_t mux_cantFallos;

void hiloWorkerTransformacion(void *info){
	TatributosHilo *atributos = (TatributosHilo *)info;
	char buffInicio[100];
	time_t horaInicio;
	time_t horaFin;
	time(&horaInicio);
	Theader headASerializar;

	strftime (buffInicio, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&horaInicio));
	log_info(logInfo,"Hora de inicio: %s\n", buffInicio);


	int sockYama = atributos->sockYama;


	int stat,sockWorker,bloqueDelArchivo,idTarea;
	char *buffer;
	int packSize;

	bool finCorrecto = false;
	bool finDesconexion=true;

	bloqueDelArchivo=atributos->infoBloque.bloqueDelArchivo;
	idTarea=atributos->infoBloque.idTarea;
	inicioEjecucionTransformacion(idTarea);

	printf("Hilo del bloque del archivo %d. id Tarea%d\n",bloqueDelArchivo,idTarea);
	log_info(logInfo,"Hilo del bloque del archivo %d. id Tarea%d\n",bloqueDelArchivo,idTarea);

	log_info(logInfo,"Nombre nodo %s\n ip nodo %s\n puerto nodo %s\n Nombre tempo %s\n",atributos->infoBloque.nombreNodo,atributos->infoBloque.ipWorker,atributos->infoBloque.puertoWorker,atributos->infoBloque.nombreTemporal);

	if((sockWorker = conectarAServidor(atributos->infoBloque.ipWorker, atributos->infoBloque.puertoWorker))<0){
		puts("No pudo conectarse a worker. le avisamos a yama");
		log_info(logInfo,"No pudo conectarse a worker. le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONFAILDESCONEXION;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		removerTransformacionFallida(idTarea);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
		return;
	}

	log_info(logInfo,"Nos conectamos a worker");

	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//enviarHeader(sockWorker,headEnvio);

	//Envio al worker el nro de bloque, el tamaño y el nombre temporal
	headASerializar.tipo_de_mensaje=NUEVATRANSFORMACION;
	headASerializar.tipo_de_proceso=MASTER;

	buffer = serializarInfoTransformacionMasterWorker(headASerializar,atributos->infoBloque.bloqueDelDatabin,atributos->infoBloque.bytesOcupados,atributos->infoBloque.nombreTemporalLen,atributos->infoBloque.nombreTemporal,&packSize);


	log_info(logInfo,"Info de la transformacion serializada, enviamos\n");
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la trasnformacion");
		log_info(logInfo,"no se pudo enviar info de la trasnformacion");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONFAILDESCONEXION;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		removerTransformacionFallida(idTarea);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);


		return;
	}
	free(buffer);
	log_info(logInfo,"se enviaron %d bytes de la info de la transformacion\n",stat);



	//envio el script
	stat=enviarScript(rutaTransformador,sockWorker);
	if(stat<0){
		puts("Error al enviar el script transformador");
		log_info(logInfo,"error al enviar el script transfo. no hay conexcion w worker. fin Transf Fail");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONFAILDESCONEXION;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		removerTransformacionFallida(idTarea);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);


		return;
	}
	//puts("al while");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {
		case(FIN_LOCALTRANSF):
			printf("Worker me avisa que termino de transformar el bloque %d\n",atributos->infoBloque.bloqueDelArchivo);
			log_info(logInfo,"Worker me avisa que termino de transformar el bloque %d\n",atributos->infoBloque.bloqueDelArchivo);
			finCorrecto = true;
			close(sockWorker);
		break;
		case FIN_LOCALTRANSFFAIL:
			printf("Worker me avisa que hubo un error en la transformacion del bloque %d. avisamos a yama para q replanifique\n",atributos->infoBloque.bloqueDelArchivo);
			log_info(logInfo,"Worker me avisa que hubo un error en la transformacion del bloque %d. avisamos a yama para q replanifique\n",atributos->infoBloque.bloqueDelArchivo);
			finCorrecto = false;
			finDesconexion=false;
			close(sockWorker);
			break;
		default:
			break;
		}


	}
	time(&horaFin);
	double diferencia = difftime(horaFin, horaInicio);
	if(finCorrecto){
		puts("Termina la conexion con worker.. La transformacion salio OK. Le avisamos a yama ");
		log_info(logInfo,"Termina la conexion con worker.. La transformacion salio OK. Le avisamos a yama ");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONOK;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		finEjecucionTransformacion(idTarea,diferencia);

	}else if(finDesconexion){
		puts("termino la conexion con worker de manera inesperada. Transformacion fallo. Le avisamos a yama");
		log_info(logInfo,"termino la conexion con worker de manera inesperada. Transformacion fallo. Le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONFAILDESCONEXION;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
		removerTransformacionFallida(idTarea);
	}else{
		puts("termino la conexion con worker que nosaviso q la Transformacion fallo. Le avisamos a yama");
		log_info(logInfo,"termino la conexion con worker que nosaviso q la Transformacion fallo. Le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);
		MUX_LOCK(&mux_cantFallos);
		cantFallos++;
		MUX_UNLOCK(&mux_cantFallos);
		removerTransformacionFallida(idTarea);
	}
	//printf("fin thread de transfo del bloque del databin %d (bloque deol archivo :%d). noodo: %s\n",atributos->infoBloque.bloqueDelDatabin,atributos->infoBloque.bloqueDelArchivo,atributos->infoBloque.nombreNodo);
	log_info(logInfo,"fin thread de transfo del bloque del databin %d (bloque deol archivo :%d). noodo: %s\n",atributos->infoBloque.bloqueDelDatabin,atributos->infoBloque.bloqueDelArchivo,atributos->infoBloque.nombreNodo);

	free(atributos->infoBloque.ipWorker);
	free(atributos->infoBloque.nombreNodo);
	free(atributos->infoBloque.nombreTemporal);
	free(atributos->infoBloque.puertoWorker);
	free(atributos);
}





int conectarseAWorkerParaReplanificarTransformacion(TpackInfoBloque *infoBloque,int sockYama){
	pthread_t workerThread;
	TatributosHilo * atributos = malloc(sizeof (TatributosHilo));
	atributos->infoBloque.idTarea=infoBloque->idTarea;
	atributos->infoBloque.bloqueDelDatabin = infoBloque->bloqueDelDatabin;
	atributos->infoBloque.bloqueDelArchivo=infoBloque->bloqueDelArchivo;
	atributos->infoBloque.bytesOcupados = infoBloque->bytesOcupados;
	atributos->infoBloque.ipWorker=malloc(MAXIMA_LONGITUD_IP);
	atributos->infoBloque.ipWorker = infoBloque->ipWorker;
	atributos->infoBloque.tamanioIp = infoBloque->tamanioIp;
	atributos->infoBloque.nombreNodo =malloc( TAMANIO_NOMBRE_NODO);
	atributos->infoBloque.nombreNodo = infoBloque->nombreNodo;
	atributos->infoBloque.tamanioNombre = infoBloque->tamanioNombre;
	atributos->infoBloque.nombreTemporal =malloc(TAMANIO_NOMBRE_TEMPORAL);
	atributos->infoBloque.nombreTemporal=infoBloque->nombreTemporal;
	atributos->infoBloque.nombreTemporalLen = infoBloque->nombreTemporalLen;
	atributos->infoBloque.puertoWorker = malloc(MAXIMA_LONGITUD_PUERTO);
	atributos->infoBloque.puertoWorker = infoBloque->puertoWorker;
	atributos->infoBloque.tamanioPuerto = infoBloque->tamanioPuerto;
	atributos->sockYama=sockYama;

	crearHilo(&workerThread, (void*)hiloWorkerTransformacion, (void*)atributos);

	return 0;
}

int conectarseAWorkersTransformacion(t_list * bloquesTransformacion,int sockYama){

	//time_t hrInicioTransf;
	//time(hrInicioTransf);

	pthread_attr_t attr_ondemand;
	pthread_attr_init(&attr_ondemand);
	pthread_attr_setdetachstate(&attr_ondemand, PTHREAD_CREATE_DETACHED);

	int cantConexiones = list_size(bloquesTransformacion);

	int i;

	for(i=0;i< cantConexiones;i++){

		TpackInfoBloque *infoBloque=list_get(bloquesTransformacion,i);
		TatributosHilo * atributos = malloc(sizeof (TatributosHilo));
		atributos->infoBloque.idTarea=infoBloque->idTarea;
		atributos->infoBloque.bloqueDelDatabin = infoBloque->bloqueDelDatabin;
		atributos->infoBloque.bloqueDelArchivo=infoBloque->bloqueDelArchivo;
		atributos->infoBloque.bytesOcupados = infoBloque->bytesOcupados;
		atributos->infoBloque.ipWorker=malloc(strlen(infoBloque->ipWorker)+1);
		atributos->infoBloque.ipWorker = infoBloque->ipWorker;
		atributos->infoBloque.tamanioIp = infoBloque->tamanioIp;
		atributos->infoBloque.nombreNodo =malloc(strlen(infoBloque->nombreNodo)+1);
		atributos->infoBloque.nombreNodo = infoBloque->nombreNodo;
		atributos->infoBloque.tamanioNombre = infoBloque->tamanioNombre;
		atributos->infoBloque.nombreTemporal =malloc(strlen(infoBloque->nombreTemporal)+1);
		atributos->infoBloque.nombreTemporal=infoBloque->nombreTemporal;
		atributos->infoBloque.nombreTemporalLen = infoBloque->nombreTemporalLen;
		atributos->infoBloque.puertoWorker = malloc(strlen(infoBloque->puertoWorker)+1);
		atributos->infoBloque.puertoWorker = infoBloque->puertoWorker;
		atributos->infoBloque.tamanioPuerto = infoBloque->tamanioPuerto;
		atributos->sockYama=sockYama;

	//	printf("creo hilo %d\n",i);
		pthread_t workerThread;
		if( pthread_create(&workerThread, &attr_ondemand, (void*) hiloWorkerTransformacion, (void*) atributos) < 0){
			//log_error(logTrace,"no pudo creasr hilo");
			perror("no pudo crear hilo. error");
			return FALLO_GRAL;
		}
		//pthread_t workerThread[i];
		//crearHilo(&workerThread[i], (void*)workerHandler, (void*)atributos);
	}

	return 0;
}






