#include "lib/funcionesYM.h"


int main(int argc, char* argv[]){
	int estado,
	    socketFS,
		socketMasters,
		socketMaster,
		tamanioCliente;
	Tyama *yama;
	pthread_t fs_thread;
	pthread_t master_thread;
	struct sockaddr client;
	Theader *head;

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
		perror("No se pudo hacer listen del socket. error");
		logAndExit("No se pudo hacer el listen al socket.");
	}
	//acepta y escucha comunicaciones

	puts("Esperando comunicaciones entrantes...");
	while((socketMaster = accept(socketMasters, &client, (socklen_t*) &tamanioCliente)) != -1){
		puts("Conexion aceptada");
		if ((estado = recv(socketMaster, head, HEAD_SIZE, 0)) < 0){
			logAndExit("Error en la recepcion del header de master.");
		}

		switch(head->tipo_de_proceso){

		case MASTER:
			//puts("Se conecto master, creamos hilo manejador");
			/*if(pthread_create(&master_thread, NULL, (void*) masterHandler,(void*) socketCliente) < 0){
				perror("No pudo crear hilo. error");
				return FALLO_GRAL;
			}*/
			puts("Proceso: Master");
			printf("Mensaje: %d \n", head->tipo_de_mensaje);

			head->tipo_de_proceso = YAMA;
			head->tipo_de_mensaje = INFO_NODO;

			enviarHeader(socketMaster, head);

			break;
		default:
			puts("Trato de conectarse algo que no era ni Kernel ni CPU!");
			printf("El tipo de proceso y mensaje son: %d y %d\n", head->tipo_de_proceso, head->tipo_de_mensaje);
			printf("Se recibio esto del socket: %d\n", socketMaster);
			return CONEX_INVAL;
		}
	}

	// Si salio del ciclo es porque fallo el accept()

	perror("Fallo el accept(). error");

	//liberarConfiguracionYama();
	return 0;
}
