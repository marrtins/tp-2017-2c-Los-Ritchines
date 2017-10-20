#include "lib/funcionesWK.h"


int main(int argc, char* argv[]){

	Tworker *worker;
	Theader * head = malloc(sizeof(Theader));
	int estado,
		socketDeMaster,
		client_sock,
		clientSize;
	struct sockaddr_in client;


	char * nombreTransformador;
	int cont = 0;
	char buffer[BUFSIZ];
	int file_size,len;
    int remain_data = 0;
	pid_t pid;
	int pidStat;


	FILE *transformadorFile;

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

			if(head->tipo_de_mensaje==TRANSFORMADORLEN){
				puts("llego trasnformador len");
			}

			nombreTransformador=string_new();
			string_append(&nombreTransformador,"/home/utnso/ScriptTransformadorNro");
			cont++;
			string_append(&nombreTransformador,string_itoa(cont));
			string_append(&nombreTransformador,worker->nombre_nodo);
			string_append(&nombreTransformador,".sh");
			if(head->tipo_de_mensaje==TRANSFORMADORLEN){
				puts("llego trasnformador len");


				/*file size */
				recv(client_sock, buffer, sizeof(int), 0);
				file_size = atoi(buffer);
				fprintf(stdout, "\nFile size : %d\n", file_size);

				transformadorFile = fopen(nombreTransformador, "w");
				if (transformadorFile == NULL){
					fprintf(stderr, "Fallo open transformador file --> %s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}

				remain_data = file_size;

				while (remain_data > 0){//todo:cheq
					len = recv(client_sock, buffer, 1024, 0);

					fwrite(buffer, sizeof(char), len, transformadorFile);
					remain_data -= len;
					fprintf(stdout, "Recinidos %d bytes y espero :- %d bytes\n", len, remain_data);
					}
				fclose(transformadorFile);
				puts("recibi archivo transformador, ahora forkeo");

				if ( (pid=fork()) == 0 )
				{ /* hijo */
					printf("Soy el hijo (%d, hijo de %d)\n", getpid(),getppid());
					printf("%d\n",cont);
					char * str = string_new();
					string_append(&str,"cat WBAN.csv | ./transformador.sh | sort  > /home/utnso/resultado");
					string_append(&str,string_itoa(cont));
					string_append(&str,worker->nombre_nodo);
					system(str);
					exit(0);

				}
				else
				{ /* padre */
					printf("Soy el padre (%d, hijo de %d)\n", getpid(),	getppid());
					printf("%d\n",cont);
					waitpid(pid,pidStat,0);
				}
				printf("Cierro fd %d\n",client_sock);
				close(client_sock);





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

