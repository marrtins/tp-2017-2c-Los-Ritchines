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
		logAndExit("Error al hacer mmap");
	}
	fclose(archivo);
	close(fd);

	inicializarArchivoDeLogs("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/worker.log");
	logger = log_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/worker.log", "worker.log", true, LOG_LEVEL_INFO);


	listaTemporalesAsociadosAJob=list_create();
	if((listenSock = crearSocketDeEscucha(worker->puerto_entrada))<0){
		return FALLO_CONEXION;
	}

	//Listen
	while ((estado = listen(listenSock , BACKLOG)) < 0){
		log_trace(logger,"No se pudo escuchar el puerto.");
	}

	//acepta y escucha
	puts("esperando comunicaciones entrantes...");
	while((client_sock = accept(listenSock, (struct sockaddr*) &client, (socklen_t*) &clientSize)) != -1){
		//puts("Conexion aceptada");
		while ((estado = recv(client_sock, head, sizeof(Theader), 0)) < 0){
			log_trace(logger,"Error en la recepcion del header.");
		}

		//printf("Cantidad de bytes recibidos: %d\n", estado);
		//printf("El tipo de proceso es %d y el mensaje es %d\n",	head->tipo_de_proceso, head->tipo_de_mensaje);

		switch(head->tipo_de_proceso){

		case MASTER:
			puts("Es master");
			manejarConexionMaster(head,client_sock);

			break;
		case WORKER:
			puts("es worker");
			manejarConexionWorker(head,client_sock);
			break;
		default:
			printf("El tipo de proceso es %d y el mensaje es %d\n", head->tipo_de_proceso, head->tipo_de_mensaje);
			return CONEX_INVAL;
		}
	}

	// Si salio del ciclo es porque fallo el accept()

	perror("Fallo el accept(). error");

	//liberarConfiguracionYama();



	//free(head);

	return 0;
}

