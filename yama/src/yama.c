#include "lib/funcionesYM.h"

int idMasterGlobal,idJobGlobal,idTareaGlobal,retardoPlanificacion;
float retardoPlanificacionSegs;
t_list *listaJobFinalizados,* listaHistoricaTareas,*listaCargaGlobal,* listaEstadoEnProceso,*listaEstadoError,*listaEstadoFinalizadoOK;
pthread_mutex_t mux_listaJobFinalizados,mux_idTareaGlobal,mux_listaHistorica,mux_listaCargaGlobal,mux_idGlobal,mux_listaEnProceso,mux_listaError,mux_listaFinalizado,mux_jobIdGlobal;
Tyama *yama;
int main(int argc, char* argv[]){
	int estado,
		socketMasters,
		sockMaster,
		tamanioCliente;



	signal(SIGUSR1, (void*)sigusr1Handler);

	//pthread_t master_thread;

	pthread_attr_t attr_ondemand;
	pthread_attr_init(&attr_ondemand);
	pthread_attr_setdetachstate(&attr_ondemand, PTHREAD_CREATE_DETACHED);


	struct sockaddr client;
	Theader head;

//	TpackageRutas * estructuraDeRutas = malloc(sizeof(TpackageRutas));

	listaHistoricaTareas=list_create();
	listaCargaGlobal = list_create();
	listaJobFinalizados= list_create();

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


	if(argc!=1){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}

	inicializarArchivoDeLogs("/home/utnso/tp-2017-2c-Los-Ritchines/yama/yama.log");
	logger = log_create("/home/utnso/tp-2017-2c-Los-Ritchines/yama/yama.log", "yama.log", false, LOG_LEVEL_ERROR);
	yama=obtenerConfiguracionYama("/home/utnso/tp-2017-2c-Los-Ritchines/yama/config_yama");
	mostrarConfiguracion(yama);
	tamanioCliente = sizeof(client);

	//yama cliente
	conectarAFS(yama);

	//yama como servidor
	socketMasters = crearSocketDeEscucha(yama->puerto_entrada);

	if ((estado = listen(socketMasters , BACKLOG)) == -1){
		logAndExit("No se pudo hacer el listen al socket.");
	}
	//acepta y escucha comunicaciones

	puts("Esperando comunicaciones entrantes...");

	while((sockMaster = accept(socketMasters, &client, (socklen_t*) &tamanioCliente)) != -1){

		puts("Conexion aceptada");

		if (recv(sockMaster, &head, sizeof(Theader), 0) < 0){
			logAndExit("Error en la recepcion del header de master.");
		}

		switch(head.tipo_de_proceso){

		case MASTER:
			puts("Se conecto master, creamos hilo manejador");


			pthread_t masterthread;
			TatributosHiloMaster * attrHilo = malloc(sizeof (TatributosHiloMaster));
			attrHilo->fdMaster=sockMaster;
			if( pthread_create(&masterthread, &attr_ondemand, (void*) masterHandler, (void*) attrHilo) < 0){
				//log_error(logTrace,"no pudo creasr hilo");
				perror("no pudo crear hilo. error");
				return FALLO_GRAL;
			}



			break;
		default:
			puts("Trato de conectarse algo no manejado!");
			printf("El tipo de proceso y mensaje son: %d y %d\n", head.tipo_de_proceso, head.tipo_de_mensaje);
			printf("Se recibio esto del socket: %d\n", sockMaster);
			return CONEX_INVAL;

		}



	}




	log_error(logger, "Fallo el accept de master.");

	//liberarConfiguracionYama();
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
