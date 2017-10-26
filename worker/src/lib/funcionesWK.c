#include "funcionesWK.h"
#include <commons/config.h>

extern Tworker *worker;

int cont=0;





void manejarConexionMaster(Theader *head, int client_sock){
	char * nombreScriptTransformador;
	char * rutaScriptTransformador ;
	char * lineaDeEjecucion;
	char * rutaResultadoTransformacion;
	char * lineaPermisoEjecucion;
	char buffer[BUFSIZ];
	int file_size,len;
	int remain_data = 0;
	int stat;
	pid_t pid;
	//int pidStat;

	Theader *headEnvio  = malloc(sizeof headEnvio);



	FILE *transformadorFile;

	TpackDatosTransformacion *datosTransf;
	char * buff;


	nombreScriptTransformador=string_new();
	rutaScriptTransformador  = string_new();
	string_append(&rutaScriptTransformador,"/home/utnso/");
	string_append(&nombreScriptTransformador,"transformador");
	cont++;
	string_append(&nombreScriptTransformador,string_itoa(cont));
	string_append(&nombreScriptTransformador,worker->nombre_nodo);
	string_append(&nombreScriptTransformador,".sh");
	string_append(&rutaScriptTransformador,nombreScriptTransformador);


	printf("Ruta Script Transformador %s\n",rutaScriptTransformador);
	printf("Nombre Script Transformador %s\n",nombreScriptTransformador);



	if(head->tipo_de_mensaje==NUEVATRANSFORMACION){
		puts("llego solicitud para nueva transformacion. recibimos bloque cant bytes y nombre temporal..");

		if ((buff = recvGenericWFlags(client_sock,0)) == NULL){
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

		transformadorFile = fopen(rutaScriptTransformador, "w");
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
			//if(len<0) break;
		}
		fclose(transformadorFile);

		puts("recibi archivo transformador");
		lineaPermisoEjecucion=string_new();
		string_append(&lineaPermisoEjecucion,"chmod +x ");
		string_append(&lineaPermisoEjecucion,rutaScriptTransformador);
		printf("%s \n",lineaPermisoEjecucion);
		stat=system(lineaPermisoEjecucion);
		printf("Stat lineaPermisoEjecucion :%d \n",stat);
		puts("Permisos de ejecucion otorgados al script transformador");
		puts("Forkeo");

		if ( (pid=fork()) == 0 )
		{ /* hijo */
			printf("Soy el hijo (%d, hijo de %d)\n", getpid(),getppid());
			printf("%d\n",cont);


			lineaDeEjecucion = string_new();
			rutaResultadoTransformacion=string_new();



			//todo: reemplazar cat wban.csv por el bloque de codigo que nos interesa trasnformar.
			string_append(&lineaDeEjecucion,"cat WBAN.csv | ./");
			string_append(&lineaDeEjecucion,nombreScriptTransformador);
			string_append(&lineaDeEjecucion, " | sort  > ");
			string_append(&rutaResultadoTransformacion,"/home/utnso/");
			string_append(&rutaResultadoTransformacion,datosTransf->nombreTemporal);
			string_append(&lineaDeEjecucion,rutaResultadoTransformacion);

			printf("linea de eecucion %s\n",lineaDeEjecucion);
			printf("Ruta resutlado Transformador %s\n",rutaResultadoTransformacion);



			stat = system(lineaDeEjecucion);
			printf("Stat lineaDeEjecucion :%d \n",stat);

			headEnvio->tipo_de_proceso = WORKER;
			headEnvio->tipo_de_mensaje = FIN_LOCALTRANSF;

			puts("Envio header. fin transfo ok");
			enviarHeader(client_sock,headEnvio);
			//close(client_sock);
			exit(0);

		}
		else
		{ /* padre */
			printf("Soy el padre (%d, hijo de %d)\n", getpid(),	getppid());
			printf("%d\n",cont);
			//waitpid(pid,pidStat,0);
		}
		//printf("Cierro fd %d\n",client_sock);
		//close(client_sock);

	}



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

