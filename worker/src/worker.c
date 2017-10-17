#include "lib/funcionesWK.h"
#include<sys/sendfile.h>

int main(int argc, char* argv[]){

	Tworker *worker;
	Theader * head = malloc(sizeof(Theader));
	int estado,
		socketDeMaster,
		client_sock,
		clientSize;
	struct sockaddr_in client;
	char  *buffer;
	TpackSrcCode *entradaTransformador;

	clientSize = sizeof client;

	/*if(argc!=2){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}*/

	logger = log_create("worker.log", "worker.log", false, LOG_LEVEL_INFO);

	worker=obtenerConfiguracionWorker(argv[1]);
	mostrarConfiguracion(worker);

	socketDeMaster = crearSocketDeEscucha(worker->puerto_entrada);

	//Listen
	while ((estado = listen(socketDeMaster , BACKLOG)) < 0){
		log_trace(logger,"No se pudo escuchar el puerto.");
	}

	//acepta y escucha
	puts("esperando comunicaciones entrantes...");
	while((client_sock = accept(socketDeMaster, (struct sockaddr*) &client, (socklen_t*) &clientSize)) != -1){
		puts("Conexion aceptada");
		while ((estado = recv(client_sock, head, sizeof(Theader), 0)) < 0){
			log_trace(logger,"Error en la recepcion del header.");
		}

		printf("Cantidad de bytes recibidos: %d\n", estado);
		printf("El tipo de proceso es %d y el mensaje es %d\n",	head->tipo_de_proceso, head->tipo_de_mensaje);
		int i=0;
		switch(head->tipo_de_proceso){

		case MASTER:
			puts("Es master");

			if(head->tipo_de_mensaje==TRANSFORMADORLEN){
				puts("llego trasnformador len");

			break;
		default:
			printf("El tipo de proceso es %d y el mensaje es %d\n", head->tipo_de_proceso, head->tipo_de_mensaje);
			return CONEX_INVAL;
		}
	}

	// Si salio del ciclo es porque fallo el accept()

	perror("Fallo el accept(). error");

	//liberarConfiguracionYama();



	free(head);

	return 0;
}
