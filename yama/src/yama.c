#include "lib/funcionesYM.h"

int idMasterGlobal,idJobGlobal,idTareaGlobal,retardoPlanificacion,idTempName;
float retardoPlanificacionSegs;
t_list *listaJobsMaster,*listaJobFinalizados,* listaHistoricaTareas,*listaCargaGlobal,* listaEstadoEnProceso,*listaEstadoError,*listaEstadoFinalizadoOK;
pthread_mutex_t mux_listaJobFinalizados,mux_idTareaGlobal,mux_listaHistorica,mux_listaCargaGlobal,mux_idGlobal,mux_listaEnProceso,mux_listaError,mux_listaFinalizado,mux_jobIdGlobal;
Tyama *yama;
int main(int argc, char* argv[]){
	int estado;
	signal(SIGUSR1, (void*)sigusr1Handler);

	//pthread_t master_thread;

	pthread_attr_t attr_ondemand;
	pthread_attr_init(&attr_ondemand);
	pthread_attr_setdetachstate(&attr_ondemand, PTHREAD_CREATE_DETACHED);



	//Theader head;

//	TpackageRutas * estructuraDeRutas = malloc(sizeof(TpackageRutas));

	listaHistoricaTareas=list_create();
	listaCargaGlobal = list_create();
	listaJobFinalizados= list_create();
	listaJobsMaster=list_create();
	listaEstadoEnProceso=list_create();
	listaEstadoError = list_create();
	listaEstadoFinalizadoOK=list_create();

	pthread_mutex_init(&mux_listaHistorica, NULL);
	pthread_mutex_init(&mux_listaCargaGlobal,NULL);
	pthread_mutex_init(&mux_idGlobal,   NULL);
	pthread_mutex_init(&mux_jobIdGlobal,   NULL);
	pthread_mutex_init(&mux_listaEnProceso,NULL);
	pthread_mutex_init(&mux_listaError,   NULL);
	pthread_mutex_init(&mux_listaFinalizado,   NULL);
	pthread_mutex_init(&mux_idTareaGlobal,   NULL);
	pthread_mutex_init(&mux_listaJobFinalizados,   NULL);

	idTempName=0;

	if(argc!=1){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}

	inicializarArchivoDeLogs("/home/utnso/tp-2017-2c-Los-Ritchines/yama/yama.log");
	logger = log_create("/home/utnso/tp-2017-2c-Los-Ritchines/yama/yama.log", "yama.log", false, LOG_LEVEL_ERROR);
	yama=obtenerConfiguracionYama("/home/utnso/tp-2017-2c-Los-Ritchines/yama/config_yama");
	mostrarConfiguracion(yama);



	Theader * head = malloc(sizeof(Theader));

	fd_set readFD, masterFD;
	int socketDeEscuchaMaster,
	fileDescriptorMax = -1,
	cantModificados = 0,
	nuevoFileDescriptor,
	fileDescriptor;

	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);

	//yama cliente

	int sockFS=conectarAFS(yama);
	//int sockFS=4;
	//fileDescriptorMax = MAXIMO(sockFS, fileDescriptorMax);
	//FD_SET(sockFS, &masterFD);




	socketDeEscuchaMaster = crearSocketDeEscucha(yama->puerto_entrada);

	fileDescriptorMax = MAXIMO(socketDeEscuchaMaster, fileDescriptorMax);
	puts("antes de entrar al while");

	while (listen(socketDeEscuchaMaster, BACKLOG) == -1){
		log_trace(logger, "Fallo al escuchar el socket servidor de file system.");
		puts("Reintentamos...");
	}


	FD_SET(socketDeEscuchaMaster, &masterFD);
	printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);

	while(1){

		readFD = masterFD;

		if((cantModificados = select(fileDescriptorMax + 1, &readFD, NULL, NULL, NULL)) == -1){
			logAndExit("Fallo el select.");
		}

		for(fileDescriptor = 3; fileDescriptor <= fileDescriptorMax; fileDescriptor++){

			if(FD_ISSET(fileDescriptor, &readFD)){
				printf("Hay un file descriptor listo. El id es: %d\n", fileDescriptor);

				if(fileDescriptor == socketDeEscuchaMaster){
					nuevoFileDescriptor = conectarNuevoCliente(fileDescriptor, &masterFD);
					printf("Nuevo nodo conectado: %d\n", nuevoFileDescriptor);
					fileDescriptorMax = MAXIMO(nuevoFileDescriptor, fileDescriptorMax);
					printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);
					break;
				}
				puts("Recibiendo...");


				if ((estado = recv(fileDescriptor, head, HEAD_SIZE, 0)) == -1){
					perror("Error en recv() de algun socket. error");
					break;

				} else if (estado == 0){
					printf("Se desconecto el socket %d\nLo sacamos del set listen...\n", fileDescriptor);
					clearAndClose(fileDescriptor, &masterFD);
					break;
				}

				if(head->tipo_de_proceso==MASTER){
					switch(head->tipo_de_mensaje){
					case INICIOMASTER:
						puts("se conecto master");
						break;

					case PATH_RES_FILE:

						iniciarNuevoJob(fileDescriptor,sockFS);
						break;
					case FINTRANSFORMACIONOK:
						manejarFinTransformacionOK(fileDescriptor);
						break;

					case FINTRANSFORMACIONFAIL:
						manejarFinTransformacionFail(fileDescriptor);
						break;

					case FIN_REDUCCIONLOCALOK:
						manejarFinReduccionLocalOK(fileDescriptor);
						break;

					case FIN_REDUCCIONLOCALFAIL:
						manejarFinReduccionLocalFail(fileDescriptor);
						break;

					case FIN_REDUCCIONGLOBALOK:
						manejarFinReduccionGlobalOK(fileDescriptor);
						break;

					case FIN_REDUCCIONGLOBALFAIL:
						manejarFinReduccionGlobalFail(fileDescriptor);
						break;
					case FIN_ALMACENAMIENTOFINALOK:
						manejarFinAlmacenamientoOK(fileDescriptor);
						break;
					case FIN_ALMACENAMIENTOFINALFAIL:
						manejarFinAlmacenamientoFail(fileDescriptor);
						break;

					default:
						puts("Tipo de Mensaje no encontrado en el protocolo");
						log_trace(logger, "LLego un tipo de mensaje, no especificado en el protocolo de filesystem.");
						break;
					}


					break;

				}
				else if(head->tipo_de_proceso == FILESYSTEM){
					switch(head->tipo_de_mensaje){
					//no tendria q entrar aca x ahora..
					default:
						log_trace(logger, "Tipo de mensaje no encontrado en el protocolo.");
						puts("Tipo de mensaje no encontrado en el protocolo.");
						break;
					}
				}
				else{
					printf("se quiso conectar el proceso: %d\n",head->tipo_de_proceso);
					puts("Hacker detected");
					log_trace(logger, "Se conecto a filesystem, un proceso que no es conocido/confiable. Expulsandolo...");
					clearAndClose(fileDescriptor, &masterFD);
				}

			}




		}

	log_error(logger, "Fallo el accept de master.");



	}

//todo liberar listast etc
return 0;
}

void sigusr1Handler(void){

	puts("Señal SIGUSR1 detectada");



	t_config *yamaConfig = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/yama/config_yama");

	if(config_has_property(yamaConfig, "RETARDO_PLANIFICACION")){
		yama->retardo_planificacion = config_get_int_value(yamaConfig, "RETARDO_PLANIFICACION");
	}
	if(config_has_property(yamaConfig, "ALGORITMO_BALANCEO")){
		yama->algoritmo_balanceo = config_get_int_value(yamaConfig, "ALGORITMO_BALANCEO");
	}
	if(config_has_property(yamaConfig, "DISPONIBILIDAD_BASE")){
		yama->disponibilidadBase = config_get_int_value(yamaConfig, "DISPONIBILIDAD_BASE");
	}
	retardoPlanificacion=yama->retardo_planificacion;
	setRetardoPlanificacion();

	config_destroy(yamaConfig);


	puts("nueva configuracion en yama");
	mostrarConfiguracion(yama);

	return;
}

void setRetardoPlanificacion(){
	retardoPlanificacionSegs = retardoPlanificacion / 1000.0;
	printf("Se cambio el retardo de planificacion a %f segundos\n", retardoPlanificacionSegs);
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
