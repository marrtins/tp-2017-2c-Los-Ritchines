#include "lib/funcionesYM.h"

int socketFS, idMasterGlobal;
int main(int argc, char* argv[]){
	int estado,
		socketMasters,
		sockMaster,
		tamanioCliente;
	Tyama *yama;
	pthread_t fs_thread;
	pthread_t master_thread;
	struct sockaddr client;
	Theader head;
	TpackageRutas * estructuraDeRutas = malloc(sizeof(TpackageRutas));

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
			if(pthread_create(&master_thread, NULL, (void*) masterHandler,(void*) sockMaster) < 0){
				perror("No pudo crear hilo. error");
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




	log_trace(logger, "Fallo el accept de master.");

	//liberarConfiguracionYama();
	return 0;
}
