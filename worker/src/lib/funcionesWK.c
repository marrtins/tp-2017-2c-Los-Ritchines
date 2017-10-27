#include "funcionesWK.h"
#include <commons/config.h>

extern Tworker *worker;

int cont=0;

TinfoReduccionLocalMasterWorker *deserializarInfoReduccionLocalMasterWorker2(char *bytes_serial){

	int off;
	TinfoReduccionLocalMasterWorker *datosReduccion;

	if ((datosReduccion = malloc(sizeof *datosReduccion)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete datos reduccion\n");
		return NULL;
	}

	off = 0;
	memcpy(&datosReduccion->nombreTempReduccionLen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((datosReduccion->nombreTempReduccion = malloc(datosReduccion->nombreTempReduccionLen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", datosReduccion->nombreTempReduccionLen);
		return NULL;
	}

	memcpy(datosReduccion->nombreTempReduccion, bytes_serial + off, datosReduccion->nombreTempReduccionLen);
	off += datosReduccion->nombreTempReduccionLen;



	memcpy(&datosReduccion->listaSize, bytes_serial + off, sizeof (int));
	off += sizeof (int);


	int i;
	t_list * listaRet = list_create();
	for(i=0;i<datosReduccion->listaSize;i++){

		TreduccionLista *aux = malloc(sizeof aux);
		memcpy(&aux->nombreTemporalLen, bytes_serial + off, sizeof (int));
		off += sizeof (int);


		if ((aux->nombreTemporal = malloc(aux->nombreTemporalLen)) == NULL){
			printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", aux->nombreTemporalLen);
			return NULL;
		}

		memcpy(aux->nombreTemporal, bytes_serial + off, aux->nombreTemporalLen);
		off += aux->nombreTemporalLen;
		list_add(listaRet,aux);
	}

	datosReduccion->listaTemporales=listaRet;



	return datosReduccion;
}



void manejarConexionMaster(Theader *head, int client_sock){

	pid_t pid,pidRed;
	int stat;
	//int pidStat;

	Theader *headEnvio  = malloc(sizeof headEnvio);





	if(head->tipo_de_mensaje==NUEVATRANSFORMACION){
		TpackDatosTransformacion *datosTransf;
		char * buff;TinfoReduccionLocalMasterWorker *deserializarInfoReduccionLocalMasterWorker(char *bytes_serial){

			int off;
			TinfoReduccionLocalMasterWorker *datosReduccion;

			if ((datosReduccion = malloc(sizeof *datosReduccion)) == NULL){
				fprintf(stderr, "No se pudo mallocar espacio para paquete datos reduccion\n");
				return NULL;
			}

			off = 0;
			memcpy(&datosReduccion->nombreTempReduccionLen, bytes_serial + off, sizeof (int));
			off += sizeof (int);

			if ((datosReduccion->nombreTempReduccion = malloc(datosReduccion->nombreTempReduccionLen)) == NULL){
				printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", datosReduccion->nombreTempReduccionLen);
				return NULL;
			}

			memcpy(datosReduccion->nombreTempReduccion, bytes_serial + off, datosReduccion->nombreTempReduccionLen);
			off += datosReduccion->nombreTempReduccionLen;



			memcpy(&datosReduccion->listaSize, bytes_serial + off, sizeof (int));
			off += sizeof (int);


			int i;
			t_list * listaRet = list_create();
			for(i=0;i<datosReduccion->listaSize;i++){

				TreduccionLista *aux = malloc(sizeof aux);
				memcpy(&aux->nombreTemporalLen, bytes_serial + off, sizeof (int));
				off += sizeof (int);


				if ((aux->nombreTemporal = malloc(aux->nombreTemporalLen)) == NULL){
					printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", aux->nombreTemporalLen);
					return NULL;
				}

				memcpy(aux->nombreTemporal, bytes_serial + off, aux->nombreTemporalLen);
				off += aux->nombreTemporalLen;
				list_add(listaRet,aux);
			}

			datosReduccion->listaTemporales=listaRet;



			return datosReduccion;
		}
		char * lineaDeEjecucionTransformacion;
		char * rutaResultadoTransformacion;
		char * nombreScriptTransformador;
		char * rutaScriptTransformador ;

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

		//Recibimos el script


		nombreScriptTransformador=string_new();
		rutaScriptTransformador  = string_new();
		string_append(&rutaScriptTransformador,"/home/utnso/");
		string_append(&nombreScriptTransformador,"transformador");
		cont++;
		string_append(&nombreScriptTransformador,string_itoa(cont));
		string_append(&nombreScriptTransformador,worker->nombre_nodo);
		string_append(&nombreScriptTransformador,".sh");
		string_append(&rutaScriptTransformador,nombreScriptTransformador);


		stat = recibirYAlmacenarScript(client_sock,rutaScriptTransformador);



	//	puts("Forkeo");
		if ( (pid=fork()) == 0 )
		{ /* hijo */
		//	printf("Soy el hijo (%d, hijo de %d)\n", getpid(),getppid());
		//	printf("%d\n",cont);


			lineaDeEjecucionTransformacion = string_new();
			rutaResultadoTransformacion=string_new();



			//todo: reemplazar cat wban.csv por el bloque de codigo que nos interesa trasnformar.
			string_append(&lineaDeEjecucionTransformacion,"cat WBAN.csv | ./");
			string_append(&lineaDeEjecucionTransformacion,nombreScriptTransformador);
			string_append(&lineaDeEjecucionTransformacion, " | sort  > ");
			string_append(&rutaResultadoTransformacion,"/home/utnso/");
			string_append(&rutaResultadoTransformacion,datosTransf->nombreTemporal);
			string_append(&lineaDeEjecucionTransformacion,rutaResultadoTransformacion);

			//printf("linea de eecucion %s\n",lineaDeEjecucionTransformacion);
//			printf("Ruta resutlado Transformador %s\n",rutaResultadoTransformacion);



			stat = system(lineaDeEjecucionTransformacion);
	//		printf("Stat lineaDeEjecucion :%d \n",stat);

			headEnvio->tipo_de_proceso = WORKER;
			headEnvio->tipo_de_mensaje = FIN_LOCALTRANSF;

			puts("Envio header. fin transfo ok");
			enviarHeader(client_sock,headEnvio);
			//close(client_sock);
			exit(0);

		}
		else
		{ /* padre */
		//	printf("Soy el padre (%d, hijo de %d)\n", getpid(),	getppid());
		//	printf("%d\n",cont);
			//waitpid(pid,pidStat,0);
		}
		//printf("Cierro fd %d\n",client_sock);
		//close(client_sock);

	}else if(head->tipo_de_mensaje==INICIARREDUCCIONLOCAL){
		TinfoReduccionLocalMasterWorker *infoReduccion;
		char * bufferReduccion;
		char * nombreScriptReductor;
		char * rutaScriptReductor;
		char  *rutaResultadoReduccion;
		char * lineaDeEjecucionReduccion;
		puts("Llego solicitud de inicio para reduccion local");


		if ((bufferReduccion = recvGenericWFlags(client_sock,0)) == NULL){
			puts("Fallo recepcion de datos de la reduccion local");
			return ;
		}

		if ((infoReduccion = deserializarInfoReduccionLocalMasterWorker2(bufferReduccion)) == NULL){
			puts("Fallo deserializacion de Bytes de los datos de la reduccion local");
			return ;
		}


		printf("\n\n\n esta e sla infoq  me llego");
		printf("Nombre temporal de la reduccion: %s\n",infoReduccion->nombreTempReduccion);

		int i;
		printf("LIST SIZE %d\n",infoReduccion->listaSize);
		for(i=0;i<infoReduccion->listaSize;i++){
			TreduccionLista *infoAux = list_get(infoReduccion->listaTemporales,i);
			printf("Nombre del archivo %d a reducir: %s\n",i,infoAux->nombreTemporal);
		}
		puts("Ahora recibo el script reductor");


		nombreScriptReductor=string_new();
		rutaScriptReductor  = string_new();
		string_append(&rutaScriptReductor,"/home/utnso/");
		string_append(&nombreScriptReductor,"reductor");
		cont++;
		string_append(&nombreScriptReductor,string_itoa(cont));
		string_append(&nombreScriptReductor,worker->nombre_nodo);
		string_append(&nombreScriptReductor,".sh");
		string_append(&rutaScriptReductor,nombreScriptReductor);


		stat = recibirYAlmacenarScript(client_sock,rutaScriptReductor);



		puts("Forkeo");

		if ( (pidRed=fork()) == 0 )
		{ /* hijo */
			//	printf("Soy el hijo (%d, hijo de %d)\n", getpid(),getppid());
			//	printf("%d\n",cont);


			lineaDeEjecucionReduccion = string_new();
			rutaResultadoReduccion=string_new();



			//todo: reemplazar cat wban.csv por el bloque de codigo que nos interesa trasnformar.
			string_append(&lineaDeEjecucionReduccion,"cat WBAN.csv | ./");
			string_append(&lineaDeEjecucionReduccion,nombreScriptReductor);
			string_append(&lineaDeEjecucionReduccion, " | sort  > ");
			string_append(&rutaResultadoReduccion,"/home/utnso/");
			string_append(&rutaResultadoReduccion,infoReduccion->nombreTempReduccion);
			string_append(&lineaDeEjecucionReduccion,rutaResultadoReduccion);

			//printf("linea de eecucion %s\n",lineaDeEjecucionTransformacion);
			//			printf("Ruta resutlado Transformador %s\n",rutaResultadoTransformacion);



			stat = system(lineaDeEjecucionReduccion);
			//		printf("Stat lineaDeEjecucion :%d \n",stat);

			headEnvio->tipo_de_proceso = WORKER;
			headEnvio->tipo_de_mensaje = FIN_REDUCCIONLOCAL;

			puts("Envio header. fin reduccion ok");
			enviarHeader(client_sock,headEnvio);
			//close(client_sock);
			exit(0);

		}
		else
		{ /* padre */
			//	printf("Soy el padre (%d, hijo de %d)\n", getpid(),	getppid());
			//	printf("%d\n",cont);
			//waitpid(pid,pidStat,0);
		}


	}



}

int recibirYAlmacenarScript(int client_sock,char * rutaAAlmacenar){



	char * lineaPermisoEjecucion;
	char buffer[BUFSIZ];
	int file_size,len;
	int remain_data = 0;
	int stat;

	FILE *scriptFile;


	/*file size */
	recv(client_sock, buffer, sizeof(int), 0);
	file_size = atoi(buffer);
	fprintf(stdout, "\nFile size : %d\n", file_size);

	scriptFile = fopen(rutaAAlmacenar, "w");
	if (scriptFile == NULL){
		fprintf(stderr, "Fallo open transformador file --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	remain_data = file_size;

	while (remain_data > 0){//todo:cheq
		len = recv(client_sock, buffer, 1024, 0);
		fwrite(buffer, sizeof(char), len, scriptFile);
		remain_data -= len;
		fprintf(stdout, "Recinidos %d bytes y espero :- %d bytes\n", len, remain_data);
		//if(len<0) break;
	}
	fclose(scriptFile);

	//puts("recibi archivo");
	printf("Ruta Script  %s\n",rutaAAlmacenar);



	lineaPermisoEjecucion=string_new();
	string_append(&lineaPermisoEjecucion,"chmod +x ");
	string_append(&lineaPermisoEjecucion,rutaAAlmacenar);
	printf("%s \n",lineaPermisoEjecucion);
	stat=system(lineaPermisoEjecucion);
//	printf("Stat lineaPermisoEjecucion :%d \n",stat);
	puts("Permisos de ejecucion otorgados al script recibido");

	return 0;

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

