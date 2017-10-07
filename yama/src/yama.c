#include "lib/funcionesYM.h"


int main(int argc, char* argv[]){
	int estado,
	    socketFS,
		socketMasters,
		tamanioCliente;
	Tyama *yama;
	pthread_t fs_thread;
	pthread_t master_thread;
	struct sockaddr client;
	Theader *head;
	TpackageRutas * estructuraDeRutas = malloc(sizeof(TpackageRutas));

	if(argc!=1){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}


	logger = log_create("yama.log", "yama.log", false, LOG_LEVEL_INFO);
	yama=obtenerConfiguracion("/home/utnso/tp-2017-2c-Los-Ritchines/yama/config_yama");
	mostrarConfiguracion(yama);
	tamanioCliente = sizeof(client);

	//yama cliente
	conectarAFS(&socketFS,yama);

	//yama como servidor
	socketMasters = crearSocketDeEscucha(yama->puerto_entrada);

	if ((estado = listen(socketMasters , BACKLOG)) == -1){
		logAndExit("No se pudo hacer el listen al socket.");
	}
	//acepta y escucha comunicaciones

	puts("Esperando comunicaciones entrantes...");
	while((socketMasters = accept(socketMasters, &client, (socklen_t*) &tamanioCliente)) != -1){
		puts("Conexion aceptada");
		if (recv(socketMasters, &estructuraDeRutas->head, sizeof(Theader), 0) < 0){
			logAndExit("Error en la recepcion del header de master.");
		}

		switch(estructuraDeRutas->head.tipo_de_proceso){

		case MASTER:
			//puts("Se conecto master, creamos hilo manejador");
			/*if(pthread_create(&master_thread, NULL, (void*) masterHandler,(void*) socketCliente) < 0){
				perror("No pudo crear hilo. error");
				return FALLO_GRAL;
			}*/

			desempaquetarRutasYamafs(estructuraDeRutas, socketMasters);

			puts("Desempaquete el mensaje.");

			puts("Proceso: Master");
			printf("Mensaje: %d \n", estructuraDeRutas->head.tipo_de_mensaje);
			printf("Ruta Origen: %s\n", estructuraDeRutas->rutaOrigen);
			printf("Ruta Resultado: %s\n", estructuraDeRutas->rutaResultado);

			/*head->tipo_de_proceso = YAMA;
			head->tipo_de_mensaje = INFO_NODO;

			enviarHeader(socketMasters, head);*/

			break;
		default:
			logAndExit("Se conecto a yama un infiltrado. Abortando proceso.");
		}
	}

	log_trace(logger, "Fallo el accept de master.");

	//liberarConfiguracionYama();
	return 0;
}
