#include "lib/funcionesFS.h"


int main(int argc, char* argv[]) {

	int estado,
		fileDescriptor,
		fileDescriptorMax = -1,
		socketDeEscuchaDatanodes,
		socketDeEscuchaYama,
		socketYama,
		nuevoFileDescriptor,
		cantModificados,
		cantNodosPorConectar;

	t_list * listaBitmaps = list_create();

	Tdirectorios tablaDirectorios[100];
	Tarchivos * tablaArchivos = malloc(sizeof(Tarchivos));

	t_list * listaDeNodos;
	TfileSystem * fileSystem;

	pthread_t consolaThread, datanodesThread;

	fd_set readFD, masterFD;

	Theader *head = malloc(HEAD_SIZE);
	char * mensaje = malloc(100	);


	/*
	if(argc != 2){
		puts("Error en la cantidad de parametros.");
		return EXIT_FAILURE;
	}*/

	logger = log_create("FileSystem.log", "FileSystem.log", false, LOG_LEVEL_ERROR);
	fileSystem = obtenerConfiguracion("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/config_filesystem");
	mostrarConfiguracion(fileSystem);
	cantNodosPorConectar = fileSystem->cant_nodos;

	list_create(listaBitmaps);
	levantarTablas(tablaDirectorios);
	levantarTablaArchivos(tablaArchivos);


	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);
	//list_create(listaDeNodos);

	crearHilo(&consolaThread, (void *)consolaFS, NULL);

	socketDeEscuchaDatanodes = crearSocketDeEscucha(fileSystem->puerto_datanode);
	fileDescriptorMax = MAXIMO(socketDeEscuchaDatanodes, fileDescriptorMax);

	while (listen(socketDeEscuchaDatanodes, BACKLOG) == -1){
			log_trace(logger, "Fallo al escuchar el socket servidor de file system.");
			puts("Reintentamos...");
	}

	FD_SET(socketDeEscuchaDatanodes, &masterFD);
	printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);

	socketDeEscuchaYama = crearSocketDeEscucha(fileSystem->puerto_yama);
	while (listen(socketDeEscuchaYama, 1) == -1) {
		log_trace(logger,"Fallo al escuchar el socket servidor de file system.");
		puts("Reintentamos...");
	}
	FD_SET(socketDeEscuchaYama, &masterFD);
	while(1){

		printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);

		puts("Entre al while");
		readFD = masterFD;
		puts("Voy a usar el select");
		if((cantModificados = select(fileDescriptorMax + 1, &readFD, NULL, NULL, NULL)) == -1){
			logAndExit("Fallo el select.");
		}
		printf("Cantidad de fd modificados: %d \n", cantModificados);
		puts("pude usar el select");

		for(fileDescriptor = 3; fileDescriptor <= fileDescriptorMax; fileDescriptor++){
			printf("Entre al for con el fd: %d\n", fileDescriptor);
			if(FD_ISSET(fileDescriptor, &readFD)){
				printf("Hay un file descriptor listo. El id es: %d\n", fileDescriptor);

				if(fileDescriptor == socketDeEscuchaDatanodes && cantNodosPorConectar > 0){
					nuevoFileDescriptor = conectarNuevoCliente(fileDescriptor, &masterFD);
					printf("Nuevo nodo conectado: %d\n", nuevoFileDescriptor);

					fileDescriptorMax = MAXIMO(nuevoFileDescriptor, fileDescriptorMax);
					printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);
					break;
				}

				if (fileDescriptor == socketDeEscuchaYama) {
					nuevoFileDescriptor = conectarNuevoCliente(fileDescriptor,
							&masterFD);
					printf("Nuevo nodo conectado: %d\n", nuevoFileDescriptor);
					fileDescriptorMax = MAXIMO(nuevoFileDescriptor,
							fileDescriptorMax);
					printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);
					break;
				}
					puts("Recibiendo...");

					estado = recv(fileDescriptor, head, HEAD_SIZE, 0);

					if(estado == -1){
						log_trace(logger, "Error al recibir información de un cliente.");
						break;
					}
					else if( estado == 0){
						sprintf(mensaje, "Se desconecto el cliente de fd: %d.", fileDescriptor);
						log_trace(logger, mensaje);
						clearAndClose(fileDescriptor, &masterFD);
					}
					switch(head->tipo_de_proceso){
						case YAMA:
							puts("Es YAMA");
							if (cantNodosPorConectar == 0) {
									puts("Filesystem estable");
									//socketYama = aceptarCliente(socketDeEscuchaYama);

									puts("Recibimos de YAMA");
									estado = recv(socketYama, head, HEAD_SIZE, 0);

									if (estado == -1) {
										log_trace(logger, "Error al recibir información de Yama.");
									} else if (estado == 0) {
										sprintf(mensaje, "Se desconecto el cliente de fd: %d.", socketYama);
										log_trace(logger, mensaje);
										close(socketYama);
									}
									printf("Recibi %d bytes\n", estado);
									printf("el proceso es %d\n", head->tipo_de_proceso);
									printf("el mensaje es %d\n", head->tipo_de_mensaje);
								}
							break;

						case DATANODE:
							puts("Es DATANODE");
							if(head->tipo_de_mensaje == INFO_NODO){
								cantNodosPorConectar--;

								list_add(listaBitmaps, crearBitmap(20)); //hardcodeado
								mostrarBitmap(list_get(listaBitmaps,0));

							}

							break;

						//NO está manejada la conexion con worker
						case WORKER:
							puts("Es worker");
							break;

						default:
							puts("Hacker detected");
							break;
					}
				printf("Recibi %d bytes\n",estado);
				printf("el proceso es %d\n", head->tipo_de_proceso);
				printf("el mensaje es %d\n", head->tipo_de_mensaje);
				break;



			} //termine el if


			puts("Sali del if de ISSET");

		} //termine el for

		puts("sali del for");

	} // termina el while


	free(mensaje);
	free(listaDeNodos);
	free(head);
	free(tablaArchivos);
	return EXIT_SUCCESS;
}



