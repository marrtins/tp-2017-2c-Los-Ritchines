#include "lib/funcionesFS.h"


int main(int argc, char* argv[]) {

	int estado,
		socketDeEscuchaYama,
		socketYama,
		cantNodosPorConectar;

	TfileSystem * fileSystem;

	pthread_t consolaThread, datanodesThread;

	fd_set readFD, masterFD;

	Theader *head = malloc(sizeof(Theader));
	char * mensaje = malloc(100);

	if(argc != 1){
		puts("Error en la cantidad de parametros.");
		return EXIT_FAILURE;
	}

	logger = log_create("FileSystem.log", "FileSystem.log", false, LOG_LEVEL_ERROR);
	fileSystem = obtenerConfiguracionFS("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/config_filesystem");
	mostrarConfiguracion(fileSystem);
	cantNodosPorConectar = fileSystem->cant_nodos;

	listaDeNodos = list_create();
	listaDeNodosDesconectados = list_create();
	listaTablaDirectorios = list_create();


	inicializarTablaDirectorios();
	inicializarTablaDeNodos();
	levantarTablasDirectorios();

	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);

	crearHilo(&consolaThread, (void *)consolaFS, NULL);
	crearHilo(&datanodesThread, (void*)conexionesDatanode, (void*)fileSystem);

	socketDeEscuchaYama = crearSocketDeEscucha(fileSystem->puerto_yama);
	while (listen(socketDeEscuchaYama, 1) == -1) {
		log_trace(logger,"Fallo al escuchar el socket servidor de file system.");
		puts("Reintentamos...");
	}
		socketYama = aceptarCliente(socketDeEscuchaYama);

	//ACA VA UN WAIT PARA QUE NO EMPIECE HASTA QUE FS ESTE ESTABLE

	while(1){
		puts("Recibiendo...");

		estado = recv(socketYama, head, sizeof(Theader), 0);

		if(estado == -1){
			log_trace(logger, "Error al recibir información de un cliente.");
			break;
		}
		else if( estado == 0){
			sprintf(mensaje, "Se desconecto YAMA fd: %d.", socketYama);
			log_trace(logger, mensaje);
			break;
		}
		if(head->tipo_de_proceso == YAMA){
		switch(head->tipo_de_mensaje){
			case INICIO_YAMA:
				puts("Es yama");
				if (cantNodosPorConectar == 0) {
					puts("Filesystem estable");
				}
			break;

			case INFO_ARCHIVO:
				puts("Es yama y quiere informacion sobre un archivo");


			break;
			default:
			break;
				}
		}
		if(head->tipo_de_proceso == WORKER){
			switch(head->tipo_de_mensaje){
			default:
				break;
			}
			//NO está manejada la conexion con worker
		}

	}

	//	FALTA free tablanodos

	//listas
	list_destroy(listaDeNodos);

	//otros
	free(mensaje);
	free(head);
	return EXIT_SUCCESS;

}
