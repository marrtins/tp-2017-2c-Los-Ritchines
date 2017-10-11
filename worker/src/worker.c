#include "lib/funcionesWK.h"

int main(int argc, char* argv[]){

	Tworker *worker;
	Theader * head = malloc(sizeof(Theader));
	int estado,
		socketDeMaster,
		client_sock,
		clientSize;
	struct sockaddr_in client;

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

		switch(head->tipo_de_proceso){

		case MASTER:
			puts("Es master");

			if(head->tipo_de_mensaje==START_LOCALTRANSF){
				puts("CASE INICIOYAMA --> FORK");

				while ((estado = recv(client_sock, head, sizeof(Theader), 0)) < 0){
							log_trace(logger,"Error en la recepcion del header.");
				}

				printf("Recibi %d\n",head->tipo_de_mensaje);

				puts("ejecuto system");
				char command[50];
				char * comando = string_new();
				int randn = rand();
				string_append(&comando,"echo ");
				string_append(&comando,string_itoa(head->tipo_de_mensaje));
				printf("%s\n",comando);
				system(comando);


				//system("echo hola pepe | ./script_transformacion.py > /tmp/resultado");
				  //int retorno = system("ls");

				//  printf("ls je %d\n",retorno);


				/*pid = fork();

				if(pid==0){

					puts("proceso hijo forkeado satisfactoriamente. ejecutamos la transf local...");
					puts("procesando...");


					sleep(4);
					puts("fin transf local, le avisamos a master y finalizamos ");

					Theader henvio;
					henvio.tipo_de_mensaje=FIN_LOCALTRANSF;
					henvio.tipo_de_proceso=WORKER;

				}
				else{
					puts("padre");
					waitpid(pid);
					break;
				}*/
				if ((estado = enviarHeader(client_sock, head)) < 0){
											fprintf(stderr, "No se pudo enviar aviso a master\n");

											return FALLO_GRAL;
										}
									puts("Mande algo");
			}


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
