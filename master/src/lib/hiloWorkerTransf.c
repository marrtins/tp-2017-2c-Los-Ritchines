/*
 * workerTransf.c
 *
 *  Created on: 26/10/2017
 *      Author: utnso
 */

#include "funcionesMS.h"

extern char * rutaTransformador;

void hiloWorkerTransformacion(void *info){
	TatributosHilo *atributos = (TatributosHilo *)info;


	int sockYama = atributos->sockYama;


	int stat,sockWorker,bloqueDelArchivo,idTarea;
//	int fdTransformador;
//	int len,remain_data,sent_bytes;
//	off_t offset;
//	struct stat file_stat;
//	char file_size[sizeof(int)];
	char *buffer;
	int packSize;
	Theader *headEnvio = malloc(sizeof(headEnvio));
	bool finCorrecto = false;

	bloqueDelArchivo=atributos->infoBloque.bloqueDelArchivo;
	idTarea=atributos->infoBloque.idTarea;

	printf("Hilo del bloque del archivo %d\n",bloqueDelArchivo);
	printf("ID tarea%d\n",idTarea);
	printf("Nombre nodo %s\n ip nodo %s\n puerto nodo %s\n Nombre tempo %s\n",atributos->infoBloque.nombreNodo,atributos->infoBloque.ipWorker,atributos->infoBloque.puertoWorker,atributos->infoBloque.nombreTemporal);

	if((sockWorker = conectarAServidor(atributos->infoBloque.ipWorker, atributos->infoBloque.puertoWorker))<0){
		puts("No pudo conectarse a worker");
		return;
	}

	puts("Nos conectamos a worker");

	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//enviarHeader(sockWorker,headEnvio);

	//Envio al worker el nro de bloque, el tamaño y el nombre temporal
	Theader headASerializar;
	headASerializar.tipo_de_mensaje=NUEVATRANSFORMACION;
	headASerializar.tipo_de_proceso=MASTER;

	buffer = serializarInfoTransformacionMasterWorker(headASerializar,atributos->infoBloque.bloqueDelDatabin,atributos->infoBloque.bytesOcupados,atributos->infoBloque.nombreTemporalLen,atributos->infoBloque.nombreTemporal,&packSize);


	printf("Info de la transformacion serializada, enviamos\n");
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la trasnformacion");
		return;
	}
	printf("se enviaron %d bytes de la info de la transformacion\n",stat);



	//envio el script
	stat=enviarScript(rutaTransformador,sockWorker);
	if(stat<0){
		puts("Error al enviar el script transformador");
	}
	puts("al while");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {
		case(FIN_LOCALTRANSF):
			printf("Worker me avisa que termino de transformar el bloque %d\n",atributos->infoBloque.bloqueDelArchivo);
			finCorrecto = true;
			close(sockWorker);
		break;
		default:
			break;
		}


	}
	if(finCorrecto){
		puts("Termina la conexion con worker.. La transformacion salio OK. Le avisamos a yama ");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONLOCALOK;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);


	}else{
		puts("termino la conexion con worker de manera inesperada. Transformacion fallo. Le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONLOCALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);

	}
	printf("fin thread de transfo del bloque del databin %d (bloque deol archivo :%d)\n",atributos->infoBloque.bloqueDelDatabin,atributos->infoBloque.bloqueDelArchivo);
}

void hiloWorkerReduccionLocal(void *info){
	TatributosHiloReduccionLocal *atributos = (TatributosHiloReduccionLocal *)info;


	int sockYama = atributos->sockYama;


	int stat,sockWorker,idTarea;
//	int fdTransformador;
//	int len,remain_data,sent_bytes;
//	off_t offset;
//	struct stat file_stat;
//	char file_size[sizeof(int)];
	char *buffer;
	int packSize;
	Theader *headEnvio = malloc(sizeof(headEnvio));
	bool finCorrecto = false;


	idTarea=atributos->infoReduccion.idTarea;


	printf("ID tarea%d\n",idTarea);
	printf("Nombre nodo %s\n ip nodo %s\n puerto nodo %s\n Nombre tempored %s \n",
			atributos->infoReduccion.nombreNodo,atributos->infoReduccion.ipNodo,atributos->infoReduccion.puertoNodo,
			atributos->infoReduccion.tempRed);


	int i;
	for(i=0;i<list_size(atributos->infoReduccion.listaTemporalesTransformacion);i++){
		TreduccionLista *infoAux = list_get(atributos->infoReduccion.listaTemporalesTransformacion,i);
		printf(" nombre temp transformacion: %s \n",infoAux->nombreTemporal);
	}


	if((sockWorker = conectarAServidor(atributos->infoReduccion.ipNodo, atributos->infoReduccion.puertoNodo))<0){
		puts("No pudo conectarse a worker");
		return;
	}

	puts("Nos conectamos a worker");
	while(1);
	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//enviarHeader(sockWorker,headEnvio);

	//Envio al worker el nro de bloque, el tamaño y el nombre temporal
	Theader headASerializar;
	headASerializar.tipo_de_mensaje=NUEVATRANSFORMACION;
	headASerializar.tipo_de_proceso=MASTER;

	//buffer = serializarInfoTransformacionMasterWorker(headASerializar,atributos->infoBloque.bloqueDelDatabin,atributos->infoBloque.bytesOcupados,atributos->infoBloque.nombreTemporalLen,atributos->infoBloque.nombreTemporal,&packSize);


	printf("Info de la transformacion serializada, enviamos\n");
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la trasnformacion");
		return;
	}
	printf("se enviaron %d bytes de la info de la transformacion\n",stat);



	//envio el script
	stat=enviarScript(rutaTransformador,sockWorker);
	if(stat<0){
		puts("Error al enviar el script transformador");
	}
	puts("al while");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {

		default:
			break;
		}


	}
	if(finCorrecto){
		puts("Termina la conexion con worker.. La transformacion salio OK. Le avisamos a yama ");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONLOCALOK;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);


	}else{
		puts("termino la conexion con worker de manera inesperada. Transformacion fallo. Le avisamos a yama");
		headASerializar.tipo_de_proceso=MASTER;
		headASerializar.tipo_de_mensaje=FINTRANSFORMACIONLOCALFAIL;
		enviarHeaderYValor(headASerializar,idTarea,sockYama);

	}
	printf("fin thread de reduccion local del nodo %s \n",atributos->infoReduccion.nombreNodo);
}




int conectarseAWorkerParaReplanificarTransformacion(TpackInfoBloque *infoBloque,int sockYama){
	pthread_t workerThread;
	TatributosHilo * atributos = malloc(sizeof atributos);
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

	pthread_attr_t attr_ondemand;
	pthread_attr_init(&attr_ondemand);
	pthread_attr_setdetachstate(&attr_ondemand, PTHREAD_CREATE_DETACHED);

	int cantConexiones = list_size(bloquesTransformacion);

	int i;

	for(i=0;i< cantConexiones;i++){

		TpackInfoBloque *infoBloque=list_get(bloquesTransformacion,i);
		TatributosHilo * atributos = malloc(sizeof atributos);
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

		printf("creo hilo %d\n",i);
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



int conectarseAWorkerParaReduccionLocal(TreduccionLocal *infoReduccion,int sockYama){


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

	printf("creo hilo \n");

	pthread_t workerReduccionLocalThread;
	crearHilo(&workerReduccionLocalThread, (void*)hiloWorkerReduccionLocal, (void*)atributos);


	return 0;
}


