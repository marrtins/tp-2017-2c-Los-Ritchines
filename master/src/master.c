#include "lib/funcionesMS.h"
#include <sys/sendfile.h>
#include <fcntl.h>


char * rutaTransformador, * rutaReductor, *rutaResultado;


Tmetricas *metricasJob;

int main(int argc, char* argv[]) {

	int sockYama,
		cantidadBytesEnviados,
		packSize,
		stat;

	Tmaster *master;
	Theader * head = malloc(sizeof(Theader));
	Theader headTmp;

	t_list *bloquesTransformacion = list_create();

	metricasJob = malloc(sizeof(Tmetricas));
	rutaTransformador = string_new();
	rutaReductor = string_new();
	char *rutaArchivoAReducir = string_new();
	rutaResultado = string_new();
	char *buffer;

	if(argc != 5){
			puts("Error en la cantidad de parametros.");
	}


	inicializarArchivoDeLogs("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/FileSystem.log");
	logger = log_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/master.log", "master.log", false, LOG_LEVEL_INFO);

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

	char buffInicio[100];

	time(&metricasJob->horaInicio);
	strftime (buffInicio, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&metricasJob->horaInicio));
	printf ("Hora de inicio del job: %s\n", buffInicio);

	sockYama = conectarAServidor(master->ipYama, master->puertoYama);

	if((cantidadBytesEnviados = enviarHeader(sockYama, head))<0){
		printf("Error enviar header");
	}

	puts("Enviamos a YAMA las rutas a reducir y almacenar");

	headTmp.tipo_de_proceso = MASTER;
	headTmp.tipo_de_mensaje = PATH_FILE_TOREDUCE ;
	packSize = 0;
	buffer=serializeBytes(headTmp,rutaArchivoAReducir,(strlen(rutaArchivoAReducir)+1),&packSize);
	//puts("Path del archivo a reducir serializado; lo enviamos");

	if ((stat = send(sockYama, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar Path del archivo a reducir a YAMA. ");
		return  FALLO_SEND;
	}

	//printf("se enviaron %d bytes del Path del archivo a reducir a YAMA\n",stat);


	headTmp.tipo_de_proceso = MASTER; headTmp.tipo_de_mensaje = PATH_RES_FILE ;packSize = 0;
	buffer=serializeBytes(headTmp,rutaResultado,(strlen(rutaResultado)+1),&packSize);
	//puts("Path del resultado serializado; lo enviamos");


	if ((stat = send(sockYama, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar Path del aresultado a YAMA. ");
		return  FALLO_SEND;
	}
	//printf("se enviaron %d bytes del Path del resultado a YAMA\n",stat);

	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};



	while ((stat=recv(sockYama, &headRcv, HEAD_SIZE, 0)) > 0) {


		//puts("Recibimos un paquete de YAMA");

		TpackInfoBloque *infoBloque;
		TreduccionLocal *infoReduccionLocal;
		TreduccionGlobal *infoReduccionGlobal;
		TinfoAlmacenadoFinal *infoAlmacenado;
		switch (headRcv.tipo_de_mensaje) {

		case (INFOBLOQUE):
			//puts("Nos llega info de un bloque");

			if((infoBloque=recibirInfoBloque(sockYama))==NULL){
				puts("Error no pudimos recibir la info bloque. se cierra");
				return FALLO_CONEXION;
			}

			list_add(bloquesTransformacion,infoBloque);
			break;



		case (INFOULTIMOBLOQUE):
			//puts("Nos llega info del ultimo bloque relacionado con el archivo a reducir");

			if((infoBloque=recibirInfoBloque(sockYama))==NULL){
				puts("Error no pudimos recibir la info bloque. se cierra");
				return FALLO_CONEXION;
			}

			list_add(bloquesTransformacion,infoBloque);
			printf("Ya nos llego toda la info relacionada al archivo a transformar. Cantidad de bloques a leer: %d\n",list_size(bloquesTransformacion));

			stat = conectarseAWorkersTransformacion(bloquesTransformacion,sockYama);


			break;
		case(INFOBLOQUEREPLANIFICADO):
		//	puts("nos llega info de un bloque a replanificar");
			if((infoBloque=recibirInfoBloque(sockYama))==NULL){
				puts("Error no pudimos recibir la info bloque. se cierra");
				return FALLO_CONEXION;
			}
			stat = conectarseAWorkerParaReplanificarTransformacion(infoBloque,sockYama);



		break;

		case(INFOREDUCCIONLOCAL):
				if((infoReduccionLocal=recibirInfoReduccionLocal(sockYama))==NULL){
					puts("Error no pudimos recibir la info bloque. se cierra");
					return FALLO_CONEXION;
				}

			stat = conectarseAWorkerParaReduccionLocal(infoReduccionLocal,sockYama);
		break;
		case(INFOREDUCCIONGLOBAL):
				if((infoReduccionGlobal=recibirInfoReduccionGlobal(sockYama))==NULL){
					puts("Error no pudimos recibir la info de la reduccion global. se cierra");
					return FALLO_CONEXION;
				}

			stat = conectarseAWorkerParaReduccionGlobal(infoReduccionGlobal,sockYama);
		break;
		case(INFOALMACENADOFINAL):
			if((infoAlmacenado=recibirInfoAlmacenadoFinal(sockYama))==NULL){
				puts("Error no pudimos recibir la info de la reduccion global. se cierra");
				return FALLO_CONEXION;
			}

			stat = conectarseAWorkerParaAlmacenamientoFinal(infoAlmacenado,sockYama);
			break;
		default:
			printf("Proceso: %d \n", headTmp.tipo_de_proceso);
			printf("Mensaje: %d \n", headTmp.tipo_de_mensaje);
			break;
		}

	}

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
//	printf("Nos llego info del bloque del archivo %d, en el databin %d \n",infoBloque->bloqueDelArchivo,infoBloque->bloqueDelDatabin);
//	printf("Nombre nodo;IPNodo;PuertoNodo;Bloque;BytesOcupados;NombreArchivotemporal;IDTAREA\n");
//	printf("%s,%s:%s,%d,%d,%s,%d\n",infoBloque->nombreNodo,infoBloque->ipWorker,infoBloque->puertoWorker,infoBloque->bloqueDelDatabin,
//			infoBloque->bytesOcupados,infoBloque->nombreTemporal,infoBloque->idTarea);

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

//	printf("Nos llego la info reduccion local de %s",infoReduccion->nombreNodo);
//	printf("job idtarea nombre nodo ipnodo puertonodo tempReductor tempTransf\n");
//	printf("%d\n%d\n%s\n%s\n%s\n%s\n",infoReduccion->job,infoReduccion->idTarea,infoReduccion->nombreNodo,
//			infoReduccion->ipNodo,infoReduccion->puertoNodo,infoReduccion->tempRed);
//	printf("list size %d\n",infoReduccion->listaSize);

//	int i;
//	for(i=0;i<list_size(infoReduccion->listaTemporalesTransformacion);i++){
//		TreduccionLista *infoAux = list_get(infoReduccion->listaTemporalesTransformacion,i);
//	//	printf(" nombre temp transformacion: %s \n",infoAux->nombreTemporal);
//	}

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
	printf("llego la info apra la reduccion global\n");
	printf("job %d\n id %d\n tempred %s\n",infoReduccionGlobal->job,infoReduccionGlobal->idTarea,infoReduccionGlobal->tempRedGlobal);

	printf("list size %d\n",infoReduccionGlobal->listaNodosSize);

	int i;
	for(i=0;i<list_size(infoReduccionGlobal->listaNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(infoReduccionGlobal->listaNodos,i);
		printf(" nombre nodo: %s \n",infoNodo->nombreNodo);
		printf(" ip nodo: %s \n",infoNodo->ipNodo);
		printf(" peurto: %s \n",infoNodo->puertoNodo);
		printf(" temp red loc: %s \n",infoNodo->temporalReduccion);
		printf(" encargado: %d \n",infoNodo->nodoEncargado);
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
	printf("llego la info apra el almacenado final\n");
	printf("job %d\n id %d\n tempred %s\n",infoAlmacenado->job,infoAlmacenado->idTarea,infoAlmacenado->nombreTempReduccion);
	printf(" ip nodo: %s \n",infoAlmacenado->ipNodo);
	printf(" peurto: %s \n",infoAlmacenado->puertoNodo);
	return infoAlmacenado;
}
