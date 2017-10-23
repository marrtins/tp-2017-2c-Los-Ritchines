#include "lib/funcionesYM.h"

int socketFS, idMasterGlobal;
t_list * listaHistoricaTareas,*listaCargaGlobal;
pthread_mutex_t mux_listaHistorica,mux_listaCargaGlobal;
int main(int argc, char* argv[]){
	int estado,
		socketMasters,
		sockMaster,
		tamanioCliente;
	Tyama *yama;

	pthread_t master_thread;
	struct sockaddr client;
	Theader head;
//	TpackageRutas * estructuraDeRutas = malloc(sizeof(TpackageRutas));

	listaHistoricaTareas=list_create();
	listaCargaGlobal = list_create();

	pthread_mutex_init(&mux_listaHistorica, NULL);
	pthread_mutex_init(&mux_listaCargaGlobal,NULL);


	if(argc!=1){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}


	logger = log_create("yama.log", "yama.log", false, LOG_LEVEL_INFO);
	yama=obtenerConfiguracionYama("/home/utnso/tp-2017-2c-Los-Ritchines/yama/config_yama");
	mostrarConfiguracion(yama);
	tamanioCliente = sizeof(client);

	//yama cliente
	//conectarAFS(&socketFS,yama);

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

			crearHilo(&master_thread, (void*)masterHandler, (void*)sockMaster);

			break;
		default:
			puts("Trato de conectarse algo no manejado!");
			printf("El tipo de proceso y mensaje son: %d y %d\n", head.tipo_de_proceso, head.tipo_de_mensaje);
			printf("Se recibio esto del socket: %d\n", sockMaster);
			return CONEX_INVAL;

		}



	}




	log_trace(logger, "Fallo el accept de master.");

	//liberarConfiguracionYama();
	return 0;
}

