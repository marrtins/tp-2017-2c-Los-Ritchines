#include "funcionesWK.h"
#include <commons/config.h>

extern Tworker *worker;


char *recvGenericWFlags2(int sock_in, int flags){
	//printf("Se recibe el paquete serializado, usando flags %x\n", flags);

	int stat, pack_size;
	char *p_serial;

	if ((stat = recv(sock_in, &pack_size, sizeof(int), flags)) == -1){
		perror("Fallo de recv. error");
		return NULL;

	} else if (stat == 0){
		printf("El proceso del socket %d se desconecto. No se pudo completar recvGenerico\n", sock_in);
		return NULL;
	}

	pack_size -= (sizeof(Theader) + sizeof(int)); // ya se recibieron estas dos cantidades
	printf("Paquete de size: %d\n", pack_size);

	if ((p_serial = malloc(pack_size)) == NULL){
		printf("No se pudieron mallocar %d bytes para paquete generico\n", pack_size);
		return NULL;
	}

	if ((stat = recv(sock_in, p_serial, pack_size, flags)) == -1){
		perror("Fallo de recv. error");
		return NULL;

	} else if (stat == 0){
		printf("El proceso del socket %d se desconecto. No se pudo completar recvGenerico\n", sock_in);
		return NULL;
	}

	return p_serial;
}


Tworker *obtenerConfiguracionWorker(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	Tworker *worker = malloc(sizeof(Tworker));

	worker->ip_filesystem       =    malloc(MAXIMA_LONGITUD_IP);
	worker->puerto_entrada= malloc(MAXIMA_LONGITUD_PUERTO);
	worker->puerto_master = malloc(MAXIMA_LONGITUD_PUERTO);
	worker->puerto_filesystem = malloc(MAXIMA_LONGITUD_PUERTO);
	worker->ruta_databin=malloc(MAXIMA_LONGITUD_RUTA);
	worker->nombre_nodo=malloc(MAXIMA_LONGITUD_NOMBRE);

	t_config *workerConfig = config_create(ruta);

	strcpy(worker->ip_filesystem, config_get_string_value(workerConfig, "IP_FILESYSTEM"));
	strcpy(worker->puerto_entrada, config_get_string_value(workerConfig, "PUERTO_WORKER"));
	strcpy(worker->puerto_master, config_get_string_value(workerConfig, "PUERTO_MASTER"));
	strcpy(worker->puerto_filesystem, config_get_string_value(workerConfig, "PUERTO_FILESYSTEM"));
	strcpy(worker->ruta_databin, config_get_string_value(workerConfig, "RUTA_DATABIN"));
	strcpy(worker->nombre_nodo, config_get_string_value(workerConfig, "NOMBRE_NODO"));

	//worker->tipo_de_proceso = DATANODE;

	config_destroy(workerConfig);
	return worker;
}

void mostrarConfiguracion(Tworker *worker){

	printf("Puerto Entrada: %s\n",  worker->puerto_entrada);
	printf("IP Filesystem %s\n",    worker->ip_filesystem);
	printf("Puerto Master: %s\n",       worker->puerto_master);
	printf("Puerto Filesystem: %s\n", worker->puerto_filesystem);
	printf("Ruta Databin: %s\n", worker->ruta_databin);
	printf("Nombre Nodo: %s\n", worker->nombre_nodo);
	printf("Tipo de proceso: %d\n", worker->tipo_de_proceso);
}

void manejarConexionMaster(Theader *head, int client_sock){
	char * nombreScriptTransformador;
	int cont = 0;
	char buffer[BUFSIZ];
	int file_size,len;
	int remain_data = 0;
	pid_t pid;
	//int pidStat;


	FILE *transformadorFile;

	TpackDatosTransformacion *datosTransf;
	char * buff;


	nombreScriptTransformador=string_new();
	string_append(&nombreScriptTransformador,"/home/utnso/ScriptTransformadorNro");
	cont++;
	string_append(&nombreScriptTransformador,string_itoa(cont));
	string_append(&nombreScriptTransformador,worker->nombre_nodo);
	string_append(&nombreScriptTransformador,".sh");

	if(head->tipo_de_mensaje==NUEVATRANSFORMACION){
		puts("llego solicitud para nueva transformacion. recibimos bloque cant bytes y nombre temporal..");

		if ((buff = recvGenericWFlags2(client_sock,0)) == NULL){
			puts("Fallo recepcion de datos de la transformacion");
			return ;
		}

		if ((datosTransf = deserializarInfoTransformacionMasterWorker(buff)) == NULL){
			puts("Fallo deserializacion de Bytes de los datos de la transformacion");
			return ;
		}

		printf("Se nos pide operar sobre el bloque %d, que ocupa %d bytes y guardarlo en el temporal %s \n",datosTransf->nroBloque,
				datosTransf->bytesOcupadosBloque,datosTransf->nombreTemporal);

		/*file size */
		recv(client_sock, buffer, sizeof(int), 0);
		file_size = atoi(buffer);
		fprintf(stdout, "\nFile size : %d\n", file_size);

		transformadorFile = fopen(nombreScriptTransformador, "w");
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
			//waitpid(pid,pidStat,0);
		}
		printf("Cierro fd %d\n",client_sock);
		close(client_sock);





	}



}

