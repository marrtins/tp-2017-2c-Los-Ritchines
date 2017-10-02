#include "lib/funcionesFS.h"


int main(int argc, char* argv[]) {

	int estado,
		socketDeEscuchaYama,
		socketYama,
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
	crearHilo(&datanodesThread, (void*)conexionesDatanode, (void*)fileSystem);

	socketDeEscuchaYama = crearSocketDeEscucha(fileSystem->puerto_yama);
	while (listen(socketDeEscuchaYama, 1) == -1) {
		log_trace(logger,"Fallo al escuchar el socket servidor de file system.");
		puts("Reintentamos...");
	}
		socketYama = aceptarCliente(socketDeEscuchaYama);

	while(1){
		puts("Recibiendo...");

		estado = recv(socketYama, head, HEAD_SIZE, 0);

		if(estado == -1){
			log_trace(logger, "Error al recibir información de un cliente.");
			break;
		}
		else if( estado == 0){
			sprintf(mensaje, "Se desconecto YAMA fd: %d.", socketYama);
			log_trace(logger, mensaje);
			break;
		}
		switch(head->tipo_de_proceso){
			case YAMA:
				puts("Es YAMA");
				if (cantNodosPorConectar == 0) {
					puts("Filesystem estable");


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

						//NO está manejada la conexion con worker
					case WORKER:
						puts("Es worker");
						break;

					default:
						puts("Hacker detected");
						break;
					}

	}puts("sali del while");

	//tabla de archivos
	liberarTablaDeArchivos(tablaArchivos);

	//otros
	free(mensaje);
	free(listaBitmaps);
	//free(listaDeNodos); HAY QUE HACER EL DESTROY LIST DE GASTON
	free(head);
	free(tablaArchivos);
	return EXIT_SUCCESS;
}



