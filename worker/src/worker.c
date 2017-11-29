#include "lib/funcionesWK.h"

Tworker *worker;
int cantApareosGlobal;
int cont;
t_list * listaApareos;
char * archivoMapeado;
int main(int argc, char* argv[]){
	//
	//signal(SIGCHLD, SIG_IGN);
	//signal(SA_NOCLDWAIT, SIG_IGN);
	int k=0;
	int p=0;
	Theader * head = malloc(sizeof(Theader));
	int estado;
	//	listenSock,
		//client_sock,
		//clientSize;
	//struct sockaddr_in client;

	cantApareosGlobal=0;
	listaApareos=list_create();
	cont=0;
	//clientSize = sizeof client;

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

	printf("HACER LC_ALL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
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
/////////////////////////////////////////////////////
	int socketDeEscucha;
	socketDeEscucha = crearSocketDeEscucha(worker->puerto_entrada);
	int fileDescriptorMax = -1,
			cantModificados = 0,
			nuevoFileDescriptor,
			fileDescriptor;
	fd_set readFD, masterFD;
	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);

	fileDescriptorMax = MAXIMO(socketDeEscucha, fileDescriptorMax);
	log_info(logInfo,"antes de entrar al while");

	while (listen(socketDeEscucha, BACKLOG) == -1){
		log_trace(logError, "Fallo al escuchar el socket servidor de file system.");
		puts("Reintentamos...");
	}


	FD_SET(socketDeEscucha, &masterFD);
	log_info(logInfo,"El FILEDESCRIPTORMAX es %d", fileDescriptorMax);

	while(1){

		readFD = masterFD;

		if((cantModificados = select(fileDescriptorMax + 1, &readFD, NULL, NULL, NULL)) == -1){
		//	puts("fallo el select");

			//logErrorAndExit("Fallo el select.");

		}

		for(fileDescriptor = 3; fileDescriptor <= fileDescriptorMax; fileDescriptor++){

			if(FD_ISSET(fileDescriptor, &readFD)){
				//log_info(logInfo,"Hay un file descriptor listo. El id es: %d\n", fileDescriptor);

				if(fileDescriptor == socketDeEscucha){
					nuevoFileDescriptor = conectarNuevoCliente(fileDescriptor, &masterFD);
					log_info(logInfo,"Nuevo nodo conectado: %d\n", nuevoFileDescriptor);
					fileDescriptorMax = MAXIMO(nuevoFileDescriptor, fileDescriptorMax);
					log_info(logInfo,"El FILEDESCRIPTORMAX es %d", fileDescriptorMax);
					break;
				}
				//log_info(logInfo,"Recibiendo...");


				if ((estado = recv(fileDescriptor, head, HEAD_SIZE, 0)) == -1){
					perror("Error en recv() de algun socket. error");
					break;

				} else if (estado == 0){
					//printf("Se desconecto el socket %d\nLo sacamos del set listen...\n", fileDescriptor);
					clearAndClose(fileDescriptor, &masterFD);
					break;
				}

				if(head->tipo_de_proceso==MASTER){

						log_info(logInfo,"Es master");
						manejarConexionMaster(head,fileDescriptor);

					break;

				}
				else if(head->tipo_de_proceso == WORKER){
					//printf("es worker. tipo de msj: %d\n",head->tipo_de_mensaje);
					//log_info(logInfo,"es worker");
					if(head->tipo_de_mensaje==GIVE_NEXTLINE)k++;
					if((k==4000) && (head->tipo_de_mensaje==GIVE_NEXTLINE)){
							printf("Realizando apareo(%d)\n",p);
							p++;
							k=0;
						}
						manejarConexionWorker(head,fileDescriptor);
						break;

				}
				else{
					printf("se quiso conectar el proceso: %d.msj:%d\n",head->tipo_de_proceso,head->tipo_de_mensaje);
					puts("Hacker detected");
					log_trace(logError, "Se conecto un proceso que no es conocido/confiable. Expulsandolo...");
					clearAndClose(fileDescriptor, &masterFD);
				}

			}




		}

	//log_error(logError, "Fallo el accept de master.");



	}



































	////////////////////////////////////////////////////////////////


	/*if((listenSock = crearSocketDeEscucha(worker->puerto_entrada))<0){
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

	log_info(logInfo,"Fallo el accept(). error");*/


	liberarConfiguracionWorker(worker);


	free(head);

	return 0;
}

int conectarNuevoCliente( int fileDescriptor, fd_set * bolsaDeFileDescriptors){
		int nuevoFileDescriptor = aceptarCliente(fileDescriptor);
		FD_SET(nuevoFileDescriptor, bolsaDeFileDescriptors);
		return nuevoFileDescriptor;
}

void clearAndClose(int fileDescriptor, fd_set* masterFD){
	FD_CLR(fileDescriptor, masterFD);
	close(fileDescriptor);
}

