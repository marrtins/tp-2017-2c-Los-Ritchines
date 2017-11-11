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
	listaInfoNodo = list_create();

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

	inicializarArchivoDeLogs("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/error.log");
	inicializarArchivoDeLogs("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/info.log");
	logError = log_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/error.log", "FileSystem", false, LOG_LEVEL_ERROR);
	logInfo = log_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/info.log", "FileSystem", false, LOG_LEVEL_INFO);
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
		log_error(logError,"Fallo al escuchar el socket servidor de file system.");
		puts("Reintentamos...");
	}
		socketYama = aceptarCliente(socketDeEscuchaYama);

	//ACA VA UN WAIT PARA QUE NO EMPIECE HASTA QUE FS ESTE ESTABLE

	while(1){
		puts("Recibiendo...");

		estado = recv(socketYama, head, sizeof(Theader), 0);

		if(estado == -1){
			log_error(logError, "Error al recibir información de un cliente.");
			break;
		}
		else if( estado == 0){
			sprintf(mensaje, "Se desconecto YAMA fd: %d.", socketYama);
			log_error(logError, mensaje);
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

				rutaArchivo = recvRutaArchivo(socketYama);

				puts(rutaArchivo);
				//verifico que la ruta que me manda yama sea valida
				if(verificarRutaArchivo(rutaArchivo)){
					puts("La ruta del archivo que mando yama es valida");
					ruta = obtenerRutaLocalDeArchivo(rutaArchivo);
					levantarTablaArchivo(archivo,ruta);

					TinfoArchivoFSYama *infoSend;

					infoSend = crearListaTablaArchivoParaYama(archivo);
					head->tipo_de_proceso=FILESYSTEM;
					head->tipo_de_mensaje=INFO_ARCHIVO;


					puts("voy a serializar la info del archivo");
					int packSize;

					char * buffer2 = serializarInfoArchivoYamaFS(*head,infoSend,&packSize);

					puts("Serialice la info del archivo");

					if ((estado = send(socketYama, buffer2 , packSize, 0)) == -1){
						logErrorAndExit("Fallo al enviar la informacion de un archivo");
					}


					puts("Envie info del archivo");
					//envio la info del nodo

					enviarInfoNodoAYama(socketYama, archivo);
					puts("Envie info del nodo");

					liberarTablaDeArchivo(archivo);
					free(ruta);
					free(buffer2);
					list_destroy_and_destroy_elements(infoSend->listaBloques, liberarTpackageUbicacionBloques);

				}else {
					//si no es valida se manda esto
					puts("La ruta no es valida");

					head->tipo_de_proceso = FILESYSTEM;
					head->tipo_de_mensaje=ARCH_NO_VALIDO;

					if ((estado = send(socketYama, head , HEAD_SIZE, 0)) == -1){
						 logErrorAndExit("Fallo al enviar la informacion de un archivo");
					}
				}

				free(rutaArchivo);



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
