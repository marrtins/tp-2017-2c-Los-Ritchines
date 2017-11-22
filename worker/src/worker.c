#include "lib/funcionesWK.h"

Tworker *worker;
int cantApareosGlobal;
int cont;
t_list * listaTemporalesAsociadosAJob;
char * archivoMapeado;
int main(int argc, char* argv[]){


	Theader * head = malloc(sizeof(Theader));
	int estado,
		listenSock,
		client_sock,
		clientSize;
	struct sockaddr_in client;

	cantApareosGlobal=0;

	cont=0;
	clientSize = sizeof client;

	if(argc!=2){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}

	worker=obtenerConfiguracionWorker(argv[1]);
	mostrarConfiguracion(worker);


	FILE * archivo = fopen(worker->ruta_databin, "rb");

	int fd;

	fd = fileno(archivo);
	if ((archivoMapeado = mmap(NULL, worker->tamanio_databin_mb*BLOQUE_SIZE, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		logErrorAndExit("Error al hacer mmap");
	}
	fclose(archivo);
	close(fd);
	int asd=system("export LC_ALL=C");
	printf("Hice LC_ALL. stat: %d\n",asd);
	mkdir("/home/utnso/tmp/", 0777);

	char * rutaLogInfo = string_new();
	char * rutaLogError=string_new();
	string_append(&rutaLogInfo,"/home/utnso/tp-2017-2c-Los-Ritchines/worker/");
	string_append(&rutaLogInfo,worker->nombre_nodo);
	string_append(&rutaLogInfo,"-info.log");

	string_append(&rutaLogError,"/home/utnso/tp-2017-2c-Los-Ritchines/worker/");
	string_append(&rutaLogError,worker->nombre_nodo);
	string_append(&rutaLogError,"-error.log");
	inicializarArchivoDeLogs(rutaLogError);
	inicializarArchivoDeLogs(rutaLogInfo);

	logError = log_create(rutaLogError, "WORKER", false, LOG_LEVEL_ERROR);
	logInfo = log_create(rutaLogInfo, "WORKER", false, LOG_LEVEL_INFO);


	listaTemporalesAsociadosAJob=list_create();
	if((listenSock = crearSocketDeEscucha(worker->puerto_entrada))<0){
		return FALLO_CONEXION;
	}

	//Listen
	while ((estado = listen(listenSock , BACKLOG)) < 0){
		log_error(logError,"No se pudo escuchar el puerto.");
	}

	//acepta y escucha
	log_info(logInfo,"esperando comunicaciones entrantes...");
	while((client_sock = accept(listenSock, (struct sockaddr*) &client, (socklen_t*) &clientSize)) != -1){
		log_info(logInfo,"Conexion aceptada");
		while ((estado = recv(client_sock, head, sizeof(Theader), 0)) < 0){
			log_error(logError,"Error en la recepcion del header.");
		}

		//printf("Cantidad de bytes recibidos: %d\n", estado);
		//printf("El tipo de proceso es %d y el mensaje es %d\n",	head->tipo_de_proceso, head->tipo_de_mensaje);

		switch(head->tipo_de_proceso){

		case MASTER:
			log_info(logInfo,"Es master");
			manejarConexionMaster(head,client_sock);


			break;
		case WORKER:
			log_info(logInfo,"es worker");
			manejarConexionWorker(head,client_sock);
			break;
		default:
			printf("El tipo de proceso es %d y el mensaje es %d\n", head->tipo_de_proceso, head->tipo_de_mensaje);
			return CONEX_INVAL;
		}
	}

	// Si salio del ciclo es porque fallo el accept()

	log_info(logInfo,"Fallo el accept(). error");


	liberarConfiguracionWorker(worker);


	free(head);

	return 0;
}

