#include "lib/funcionesMS.h"
#include <sys/sendfile.h>
#include <fcntl.h>


char * rutaTransformador, * rutaReductor, *rutaResultado;

int maximoTransformacionesParalelas,maximoReduccionesParalelas,
cantTransformaciones,cantRL,cantFallos;
double duracionRG,duracionAlmacenado,duracionJob;
time_t horaInicio;
t_list * transfEjecutando,*transfFin,*rlEjecutando,*rlFin,*duracionTransformaciones,*duracionRL;
pthread_mutex_t mux_transfEjecutando,mux_transfFin,mux_rlEjecutando,mux_rlFin,mux_duracionTransformaciones,mux_duracionRL,mux_cantFallos;
int main(int argc, char* argv[]) {

	int sockYama,
		cantidadBytesEnviados,
		packSize,
		stat;

	Tmaster *master;
	Theader * head = malloc(sizeof(Theader));
	Theader headTmp;
//	int myId;
	t_list *bloquesTransformacion = list_create();
	maximoTransformacionesParalelas=-1;
	maximoReduccionesParalelas=-1;
	cantRL=0;
	cantTransformaciones=0;
	cantFallos=0;
	//metricasJob = malloc(sizeof(Tmetricas));
	rutaTransformador = string_new();
	rutaReductor = string_new();
	char *rutaArchivoAReducir = string_new();
	rutaResultado = string_new();
	char *buffer;
	transfEjecutando = list_create();
	pthread_mutex_init(&mux_transfEjecutando, NULL);
	transfFin= list_create();
	pthread_mutex_init(&mux_transfFin, NULL);
	rlEjecutando= list_create();
	pthread_mutex_init(&mux_rlEjecutando, NULL);
	rlFin= list_create();
	pthread_mutex_init(&mux_rlFin, NULL);
	duracionRL=list_create();
	pthread_mutex_init(&mux_duracionRL, NULL);
	duracionTransformaciones=list_create();
	pthread_mutex_init(&mux_duracionTransformaciones, NULL);

	pthread_mutex_init(&mux_cantFallos, NULL);

	if(argc != 5){
			puts("Error en la cantidad de parametros.");
	}

	/*char * rutaLogInfo = string_new();
	string_append(&rutaLogInfo,"home/utnso/tp-2017-2c-Los-Ritchines/master/");
	//string_append(&rutaLogInfo,argv[3]);
	string_append(&rutaLogInfo,"info.log");


	char * rutaLogError = string_new();
	string_append(&rutaLogError,"home/utnso/tp-2017-2c-Los-Ritchines/master/");
	//string_append(&rutaLogError,argv[3]);
	string_append(&rutaLogError,"error.log");


	inicializarArchivoDeLogs(rutaLogError);
	inicializarArchivoDeLogs(rutaLogInfo);
	logError = log_create(rutaLogError, "MASTER", false, LOG_LEVEL_ERROR);
	logInfo = log_create(rutaLogInfo, "MASTER", false, LOG_LEVEL_INFO);
*/


	inicializarArchivoDeLogs("/home/utnso/tp-2017-2c-Los-Ritchines/master/error.log");
	inicializarArchivoDeLogs("/home/utnso/tp-2017-2c-Los-Ritchines/master/info.log");
	logError = log_create("/home/utnso/tp-2017-2c-Los-Ritchines/master/error.log", "MASTER", false, LOG_LEVEL_ERROR);
	logInfo = log_create("/home/utnso/tp-2017-2c-Los-Ritchines/master/info.log", "MASTER", false, LOG_LEVEL_INFO);

	head->tipo_de_proceso = MASTER;
	head->tipo_de_mensaje = INICIOMASTER;

	// arg[0]: nombre de la funcion
	// arg[1]: ruta transformador
	// arg[2]: ruta reductor
	// arg[3]: ruta del archivo dentro del filesystem
	// arg[4]: ruta de destino del archivo final


	rutaTransformador=argv[1];
	rutaReductor=argv[2];
	rutaArchivoAReducir=argv[3];
	rutaResultado=argv[4];

	master = obtenerConfiguracionMaster("/home/utnso/tp-2017-2c-Los-Ritchines/master/config_master");
	mostrarConfiguracion(master);

	printf("Transformador Path: %s\n",rutaTransformador);
	printf("Reductor Path: %s\n",rutaReductor);
	printf("Archivo a reducir Path: %s\n",rutaArchivoAReducir);
	printf("Resultado Path: %s\n",rutaResultado);


	log_info(logInfo,"Transformador Path: %s\n",rutaTransformador);
	log_info(logInfo,"Reductor Path: %s\n",rutaReductor);
	log_info(logInfo,"Archivo a reducir Path: %s\n",rutaArchivoAReducir);
	log_info(logInfo,"Resultado Path: %s\n",rutaResultado);

	char buffInicio[100];

	time(&horaInicio);
	strftime (buffInicio, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&horaInicio));
	log_info(logInfo,"Hora de inicio del job: %s\n", buffInicio);

	sockYama = conectarAServidor(master->ipYama, master->puertoYama);
	if(sockYama < 0){
		logErrorAndExit("error al conectarse a yama");
	}
	if((cantidadBytesEnviados = enviarHeader(sockYama, head))<0){
		logErrorAndExit("error enviar header ayama");

	}

	//puts("Enviamos a YAMA las rutas a reducir y almacenar");



	headTmp.tipo_de_proceso = MASTER;
	headTmp.tipo_de_mensaje = PATH_RES_FILE ;
	packSize = 0;
	buffer=serializeBytes(headTmp,rutaResultado,(strlen(rutaResultado)+1),&packSize);
	if ((stat = send(sockYama, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar Path del aresultado a YAMA. ");
		return  FALLO_SEND;
	}

	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};

	headTmp.tipo_de_proceso = MASTER;
	headTmp.tipo_de_mensaje = PATH_FILE_TOREDUCE ;
	packSize = 0;
	buffer=serializeBytes(headTmp,rutaArchivoAReducir,(strlen(rutaArchivoAReducir)+1),&packSize);
	if ((stat = send(sockYama, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar Path del archivo a reducir a YAMA. ");
		return  FALLO_SEND;
	}


	while ((stat=recv(sockYama, &headRcv, HEAD_SIZE, 0)) > 0) {


		log_info(logInfo,"Recibimos un paquete de YAMA");

		TpackInfoBloque *infoBloque;
		TreduccionLocal *infoReduccionLocal;
		TreduccionGlobal *infoReduccionGlobal;
		TinfoAlmacenadoFinal *infoAlmacenado;
		switch (headRcv.tipo_de_mensaje) {

		case (INFOBLOQUE):
		log_info(logInfo,"Nos llega info de un bloque");

			if((infoBloque=recibirInfoBloque(sockYama))==NULL){
				puts("Error no pudimos recibir la info bloque. se cierra");
				return FALLO_CONEXION;
			}

			list_add(bloquesTransformacion,infoBloque);
			break;



		case (INFOULTIMOBLOQUE):
		log_info(logInfo,"Nos llega info del ultimo bloque relacionado con el archivo a reducir");

			if((infoBloque=recibirInfoBloque(sockYama))==NULL){
				puts("Error no pudimos recibir la info bloque. se cierra");
				return FALLO_CONEXION;
			}

			list_add(bloquesTransformacion,infoBloque);
			log_info(logInfo,"Ya nos llego toda la info relacionada al archivo a transformar. Cantidad de bloques a leer: %d\n",list_size(bloquesTransformacion));

			stat = conectarseAWorkersTransformacion(bloquesTransformacion,sockYama);
			if(stat <0 ){
				puts("error conectarse workers transformacion");
			}
			list_destroy(bloquesTransformacion);


			break;
		case(INFOBLOQUEREPLANIFICADO):
			log_info(logInfo,"nos llega info de un bloque a replanificar");
			if((infoBloque=recibirInfoBloque(sockYama))==NULL){
				puts("Error no pudimos recibir la info bloque. se cierra");
				return FALLO_CONEXION;
			}
			stat = conectarseAWorkerParaReplanificarTransformacion(infoBloque,sockYama);
			if(stat < 0){
				puts("error conectarse a workers xra transformacion replanificada");
			}


		break;

		case(INFOREDUCCIONLOCAL):
				if((infoReduccionLocal=recibirInfoReduccionLocal(sockYama))==NULL){
					puts("Error no pudimos recibir la info bloque. se cierra");
					return FALLO_CONEXION;
				}

			stat = conectarseAWorkerParaReduccionLocal(infoReduccionLocal,sockYama);
			if(stat < 0){
				puts("error conectarse a worker para redu local");
			}
		break;
		case(INFOREDUCCIONGLOBAL):
				if((infoReduccionGlobal=recibirInfoReduccionGlobal(sockYama))==NULL){
					puts("Error no pudimos recibir la info de la reduccion global. se cierra");
					return FALLO_CONEXION;
				}

			stat = conectarseAWorkerParaReduccionGlobal(infoReduccionGlobal,sockYama);
			if(stat < 0){
				puts("error conectarse a worker para reduccion global");
			}
		break;
		case(INFOALMACENADOFINAL):
			if((infoAlmacenado=recibirInfoAlmacenadoFinal(sockYama))==NULL){
				puts("Error no pudimos recibir la info de la reduccion global. se cierra");
				return FALLO_CONEXION;
			}

			stat = conectarseAWorkerParaAlmacenamientoFinal(infoAlmacenado,sockYama);
			if(stat<0){
				puts("error conectarse a worker para almacenamietno final");
			}
			break;
		case(ARCH_NO_VALIDO):
			puts("yama nos avisa q el archivo no es valido. fin deol proceso");
			close(sockYama);
			return 0;
			break;
		case FINJOB_ERRORREPLANIFICACION:


			mostrarMetricasJob();
			puts("\n\n #####yama nos avisa q termino el job x error de replanificadion######");
			log_info(logInfo,"yama nos avisa q terminoe l job x error de replanificaicion");
			return EXIT_SUCCESS;
		default:
			log_info(logInfo,"Proceso: %d \n", headTmp.tipo_de_proceso);
			log_info(logInfo,"Mensaje: %d \n", headTmp.tipo_de_mensaje);
			break;
		}

	}
	puts("fin conexion con yama");
	log_info(logInfo,"fin conexcion c yama");
	free(head);
	freeAndNULL((void **) &buffer);
	return EXIT_SUCCESS;
}



TpackInfoBloque *recibirInfoBloque(int sockYama){
	char * buffer;
	TpackInfoBloque *infoBloque;
	if ((buffer = recvGenericWFlags(sockYama,MSG_WAITALL)) == NULL){
		puts("Fallo recepcion de INFOBLOQUE");
		return NULL;
	}

	if ((infoBloque = deserializeInfoBloque(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del path_res_file");
		return NULL;
	}
	free(buffer);
	log_info(logInfo,"Nos llego info del bloque del archivo %d, en el databin %d \n",infoBloque->bloqueDelArchivo,infoBloque->bloqueDelDatabin);
	log_info(logInfo,"Nombre nodo;IPNodo;PuertoNodo;Bloque;BytesOcupados;NombreArchivotemporal;IDTAREA\n");
	log_info(logInfo,"%s,%s:%s,%d,%d,%s,%d\n",infoBloque->nombreNodo,infoBloque->ipWorker,infoBloque->puertoWorker,infoBloque->bloqueDelDatabin,
			infoBloque->bytesOcupados,infoBloque->nombreTemporal,infoBloque->idTarea);

	return infoBloque;
}

TreduccionLocal *recibirInfoReduccionLocal(int sockYama){
	char * buffer;
	TreduccionLocal *infoReduccion;
	if ((buffer = recvGenericWFlags(sockYama,MSG_WAITALL)) == NULL){
		puts("Fallo recepcion de INFOBLOQUE");
		return NULL;
	}

	if ((infoReduccion = deserializeInfoReduccionLocal(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del deserializar info reduccion local");
		return NULL;
	}
	free(buffer);
	log_info(logInfo,"Nos llego la info reduccion local de %s",infoReduccion->nombreNodo);
	log_info(logInfo,"job idtarea nombre nodo ipnodo puertonodo tempReductor tempTransf\n");
	log_info(logInfo,"%d\n%d\n%s\n%s\n%s\n%s\n",infoReduccion->job,infoReduccion->idTarea,infoReduccion->nombreNodo,
			infoReduccion->ipNodo,infoReduccion->puertoNodo,infoReduccion->tempRed);
	log_info(logInfo,"list size %d\n",infoReduccion->listaSize);

	int i;
	for(i=0;i<list_size(infoReduccion->listaTemporalesTransformacion);i++){
		TreduccionLista *infoAux = list_get(infoReduccion->listaTemporalesTransformacion,i);
		log_info(logInfo," nombre temp transformacion: %s \n",infoAux->nombreTemporal);
	}

	return infoReduccion;
}


TreduccionGlobal *recibirInfoReduccionGlobal(int sockYama){
	char * buffer;
	TreduccionGlobal *infoReduccionGlobal;
	if ((buffer = recvGenericWFlags(sockYama,MSG_WAITALL)) == NULL){
		puts("Fallo recepcion de INFOBLOQUE");
		return NULL;
	}

	if ((infoReduccionGlobal = deserializeInfoReduccionGlobal(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del deserializar info reduccion local");
		return NULL;
	}
	free(buffer);
	log_info(logInfo,"llego la info apra la reduccion global\n");
	log_info(logInfo,"job %d\n id %d\n tempred %s\n",infoReduccionGlobal->job,infoReduccionGlobal->idTarea,infoReduccionGlobal->tempRedGlobal);

	log_info(logInfo,"list size %d\n",infoReduccionGlobal->listaNodosSize);

	int i;
	for(i=0;i<list_size(infoReduccionGlobal->listaNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(infoReduccionGlobal->listaNodos,i);
		log_info(logInfo," nombre nodo: %s \n",infoNodo->nombreNodo);
		log_info(logInfo," ip nodo: %s \n",infoNodo->ipNodo);
		log_info(logInfo," peurto: %s \n",infoNodo->puertoNodo);
		log_info(logInfo," temp red loc: %s \n",infoNodo->temporalReduccion);
		log_info(logInfo," encargado: %d \n",infoNodo->nodoEncargado);
	}

	return infoReduccionGlobal;
}



TinfoAlmacenadoFinal *recibirInfoAlmacenadoFinal(int sockYama){
	char * buffer;
	TinfoAlmacenadoFinal *infoAlmacenado;
	if ((buffer = recvGenericWFlags(sockYama,MSG_WAITALL)) == NULL){
		puts("Fallo recepcion de INFOBLOQUE");
		return NULL;
	}

	if ((infoAlmacenado = deserializeInfoAlmacenadoFinal(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del deserializar info reduccion local");
		return NULL;
	}
	free(buffer);
	log_info(logInfo,"llego la info apra el almacenado final\n");
	log_info(logInfo,"job %d\n id %d\n tempred %s\n",infoAlmacenado->job,infoAlmacenado->idTarea,infoAlmacenado->nombreTempReduccion);
	log_info(logInfo," ip nodo: %s \n",infoAlmacenado->ipNodo);
	log_info(logInfo," peurto: %s \n",infoAlmacenado->puertoNodo);
	return infoAlmacenado;
}
