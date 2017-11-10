#include "lib/funcionesFS.h"


int main(int argc, char* argv[]) {

	int estado,
		socketDeEscuchaYama,
		socketYama,
		cantNodosPorConectar;

	estable = 0;
	TfileSystem * fileSystem;
	pthread_t consolaThread, datanodesThread;

	fd_set readFD, masterFD;

	Theader *head = malloc(sizeof(Theader));
	char * mensaje = malloc(100);
	char * rutaArchivo;


	listaDeNodos = list_create();
	listaDeNodosDesconectados = list_create();
	listaTablaDirectorios = list_create();

	/*if(argc != 1){
		puts("Error en la cantidad de parametros.");
		return EXIT_FAILURE;
	}
	*/
	if(argc == 2){
		char * flag = malloc(10);
		strcpy(flag,"--clean");
		if(string_equals_ignore_case(argv[1],flag)){
			inicializarTablaDirectorios();
			inicializarBitmaps();
		}else{
			puts("La flag ingresada no es válida");
			return EXIT_FAILURE;
		}
		free(flag);
	}else {
		levantarEstadoAnteriorDeLaTablaDeNodos(listaDeNodosDesconectados);
	}

	inicializarArchivoDeLogs("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/FileSystem.log");
	logger = log_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/FileSystem.log", "FileSystem", false, LOG_LEVEL_ERROR);
	fileSystem = obtenerConfiguracionFS("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/config_filesystem");
	mostrarConfiguracion(fileSystem);
	cantNodosPorConectar = fileSystem->cant_nodos;

	inicializarTablaDeNodos();
	levantarTablasDirectorios();

	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);
	crearHilo(&consolaThread, (void *)consolaFS, NULL);
	crearHilo(&datanodesThread, (void*)conexionesDatanode, (void*)fileSystem);
	socketDeEscuchaYama = crearSocketDeEscucha(fileSystem->puerto_yama);
	while (listen(socketDeEscuchaYama, 1) == -1) {
		log_error(logger,"Fallo al escuchar el socket servidor de file system.");
		puts("Reintentamos...");
	}
		socketYama = aceptarCliente(socketDeEscuchaYama);

	//ACA VA UN WAIT PARA QUE NO EMPIECE HASTA QUE FS ESTE ESTABLE

	while(1){
		puts("Recibiendo...");

		estado = recv(socketYama, head, sizeof(Theader), 0);

		if(estado == -1){
			log_error(logger, "Error al recibir información de un cliente.");
			break;
		}
		else if( estado == 0){
			sprintf(mensaje, "Se desconecto YAMA fd: %d.", socketYama);
			log_error(logger, mensaje);
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
				char * ruta;
				Tarchivo * archivo = malloc(sizeof(Tarchivo));

				head->tipo_de_proceso = FILESYSTEM;
				head->tipo_de_mensaje = INFO_ARCHIVO;

				rutaArchivo = recvRutaArchivo(socketYama);

				puts(rutaArchivo);
				//verifico que la ruta que me manda yama sea valida
				if(verificarRutaArchivo(rutaArchivo)){
					puts("La ruta del archivo que mando yama es valida");
					ruta = obtenerRutaLocalDeArchivo(rutaArchivo);
					levantarTablaArchivo(archivo,ruta);

					TinfoArchivoFSYama *infoSend;

					infoSend = crearListaTablaArchivoParaYama(archivo);
					head->tipo_de_mensaje=INFO_ARCHIVO;
					head->tipo_de_proceso=FILESYSTEM;

					puts("voy a serializar la info del archivo");
					int packSize;
					Theader head2;
					head2.tipo_de_proceso=FILESYSTEM;
					head2.tipo_de_mensaje=INFO_ARCHIVO;
					char * buffer2=serializarInfoArchivoYamaFS(head2,infoSend,&packSize);

					puts("serialice la info del archivo");

					if ((estado = send(socketYama, buffer2 , packSize, 0)) == -1){
						logAndExit("Fallo al enviar la informacion de un archivo");
					}
					free(ruta);

					puts("envie info del archivo");
					//envio la info del nodo

					enviarInfoNodoAYama(socketYama);
					puts("envie info del nodo");
				}else {
					//si no es valida se manda esto
					puts("la ruta no es valida");
					head->tipo_de_mensaje=ARCH_NO_VALIDO;
					head->tipo_de_proceso = FILESYSTEM;
					if ((estado = send(socketYama, head , HEAD_SIZE, 0)) == -1){
						 logAndExit("Fallo al enviar la informacion de un archivo");
					}
				}


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
