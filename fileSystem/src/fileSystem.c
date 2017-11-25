#include "lib/funcionesFS.h"


int main(int argc, char* argv[]) {

	int estado,
		socketDeEscuchaYama,
		socketYama;

	esEstadoRecuperado = 1;
	TfileSystem * fileSystem;
	pthread_t consolaThread, datanodesThread;

	fd_set readFD, masterFD;

	sem_init(&yama,0,0);

	Theader *head = malloc(sizeof(Theader));
	char * mensaje = malloc(100);
	char * rutaArchivo;


	listaDeNodos = list_create();
	listaDeNodosDesconectados = list_create();
	listaTablaDirectorios = list_create();
	listaInfoNodo = list_create();

	if(argc == 2){
		char * flag = malloc(10);
		strcpy(flag,"--clean");
		if(string_equals_ignore_case(argv[1],flag)){
			inicializarTablaDirectorios();
			inicializarBitmaps();
			esEstadoRecuperado = 0;
		}else{
			puts("La flag ingresada no es válida");
			log_error(logError,"La flag ingresada no es válida");
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

	inicializarTablaDeNodos();
	log_info(logInfo,"Se inicializa la tabla de nodos");
	levantarTablasDirectorios();
	log_info(logInfo,"Se levanta la tabla de directorios");
	crearDirectorioTemporal();

	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);
	crearHilo(&consolaThread, (void *)consolaFS, NULL);
	crearHilo(&datanodesThread, (void*)conexionesDatanode, (void*)fileSystem);
	socketDeEscuchaYama = crearSocketDeEscucha(fileSystem->puerto_yama);
	while (listen(socketDeEscuchaYama, 1) == -1) {
		log_error(logError,"Fallo al escuchar el socket servidor de file system.");
	}

		socketYama = aceptarCliente(socketDeEscuchaYama);
		log_info(logInfo,"Se acepto a YAMA");

	// WAIT PARA QUE NO EMPIECE HASTA QUE FS ESTE ESTABLE

		sem_wait(&yama);

		log_info(logInfo,"Paso el mutex de YAMA");

	while(1){

		estado = recv(socketYama, head, sizeof(Theader), 0);

		if(estado == -1){
			log_error(logError, "Error al recibir el header de YAMA.");
			break;
		}
		else if( estado == 0){
			log_error(logError, "Se desconecto YAMA con fd: %d.", socketYama);
			break;
		}
		if(head->tipo_de_proceso == YAMA){
		switch(head->tipo_de_mensaje){
			case INICIO_YAMA:
				log_info(logInfo,"Se conecto YAMA por primera vez.");
			break;

			case INFO_ARCHIVO:
				log_info(logInfo,"Recibiendo peticion de yama, de la informacion del archivo al que se le va a aplicar la transformacion.");
				char * ruta;
				Tarchivo * archivo = malloc(sizeof(Tarchivo));

				rutaArchivo = recvRutaArchivo(socketYama);
				log_info(logInfo, "Recibi la ruta %s.", rutaArchivo);

				//verifico que la ruta que me manda yama sea valida
				if(verificarRutaArchivo(rutaArchivo)){
					ruta = obtenerRutaLocalDeArchivo(rutaArchivo);
					levantarTablaArchivo(archivo,ruta);

					TinfoArchivoFSYama *infoSend;

					infoSend = crearListaTablaArchivoParaYama(archivo);
					head->tipo_de_proceso=FILESYSTEM;
					head->tipo_de_mensaje=INFO_ARCHIVO;

					int packSize;

					log_info(logInfo, "Serializando info del archivo");
					char * buffer2 = serializarInfoArchivoYamaFS(*head,infoSend,&packSize);
					log_info(logInfo, "Info de archivo serializada.");

					if ((estado = send(socketYama, buffer2 , packSize, 0)) == -1){
						logErrorAndExit("Fallo al enviar la informacion de un archivo");
					}
					log_info(logInfo, "Enviando info a yama.");

					//envio la info del nodo

					enviarInfoNodoAYama(socketYama, archivo);
					log_info(logInfo,"Se envió a yama, la información del archivo que solicito.");

					liberarTablaDeArchivo(archivo);
					free(ruta);

					//todo: Free(infoSend)

					free(buffer2);


				}else {
					//si no es valida se manda esto
					log_error(logError,"La ruta del archivo que solicito yama NO es valida.");

					head->tipo_de_proceso = FILESYSTEM;
					head->tipo_de_mensaje=ARCH_NO_VALIDO;

					if ((estado = send(socketYama, head , HEAD_SIZE, 0)) == -1){
						 logErrorAndExit("Fallo al enviar header a YAMA");
					}
				}

				free(rutaArchivo);




			break;
			default:
			break;
				}
		}
		//esto no iria aca
		if(head->tipo_de_proceso == WORKER){
			switch(head->tipo_de_mensaje){
			default:
				break;
			}

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
