#include "funcionesWK.h"
#include <commons/config.h>

extern Tworker *worker;
extern int cantApareosGlobal;
extern int cont=0;



void manejarConexionWorker(Theader *head, int client_sock){

	char * buffer;
	TpackBytes *pathArchivoTemporal;
	int stat;
	if(head->tipo_de_mensaje==GIVE_TMPREDUCCIONLOCAL){

		puts("Nos llega el path del archivo temporal que precisa");


		if ((buffer = recvGeneric(client_sock)) == NULL){
			puts("Fallo recepcion del path del archivo temporal");
			return;
		}

		if ((pathArchivoTemporal =  deserializeBytes(buffer)) == NULL){
			puts("Fallo deserializacion de Bytes del path arch a reducir");
			return;
		}

		printf("Path archivo que vamos a enviarle: %s\n",pathArchivoTemporal->bytes);

		stat = enviarArchivo(pathArchivoTemporal->bytes,client_sock);
		if(stat<0){
			puts("error al enviar archivo");
		}

	}


}

void manejarConexionMaster(Theader *head, int client_sock){


	int stat;


	Theader *headEnvio  = malloc(sizeof headEnvio);


	if(head->tipo_de_mensaje==NUEVATRANSFORMACION){

		if((stat = realizarTransformacion(client_sock))<0){
			puts("Fallo al recibir algun paquete de la transf");
		}

	}else if(head->tipo_de_mensaje==INICIARREDUCCIONLOCAL){

		if((stat = realizarReduccionLocal(client_sock))<0){
					puts("Fallo al recibir algun paquete de la transf");
				}

	}else if(head->tipo_de_mensaje==INICIARREDUCCIONGLOBAL){
		if((stat = realizarReduccionGlobal(client_sock))<0){
					puts("Fallo al recibir algun paquete de la transf");
				}
	}



}
int realizarReduccionGlobal(client_sock){

	char * buffer;
	char * nombreScriptReductor;
	char * rutaScriptReductor;
	int stat;
	TreduccionGlobal *infoReduccionGlobal;
	if ((buffer = recvGenericWFlags(client_sock,MSG_WAITALL)) == NULL){
		puts("Fallo recepcion de INFOBLOQUE");
		return FALLO_RECV;
	}

	if ((infoReduccionGlobal = deserializeInfoReduccionGlobal(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del deserializar info reduccion local");
		return FALLO_GRAL;
	}
	printf("llego la info apra la reduccion global\n");
	printf("job %d\n id %d\n tempred %s\n",infoReduccionGlobal->job,infoReduccionGlobal->idTarea,infoReduccionGlobal->tempRedGlobal);

	printf("list size %d\n",infoReduccionGlobal->listaNodosSize);

	int i;
	for(i=0;i<list_size(infoReduccionGlobal->listaNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(infoReduccionGlobal->listaNodos,i);
		printf(" nombre nodo: %s \n",infoNodo->nombreNodo);
		printf(" ip nodo: %s \n",infoNodo->ipNodo);
		printf(" peurto: %s \n",infoNodo->puertoNodo);
		printf(" temp red loc: %s \n",infoNodo->temporalReduccion);
		printf(" encargado: %d \n",infoNodo->nodoEncargado);
	}


	puts("Ahora recibo el script reductor");


	nombreScriptReductor=string_new();
	rutaScriptReductor  = string_new();
	string_append(&rutaScriptReductor,"/home/utnso/");
	string_append(&nombreScriptReductor,"reductorGlobal");
	cont++;
	string_append(&nombreScriptReductor,string_itoa(cont));
	string_append(&nombreScriptReductor,worker->nombre_nodo);
	string_append(&nombreScriptReductor,".sh");
	string_append(&rutaScriptReductor,nombreScriptReductor);


	stat = recibirYAlmacenarScript(client_sock,rutaScriptReductor);


	//le pido a todos los workers que me pasen sus reducciones locales.

	int sockWorkerVecino,packSize2;
	Theader headEnvio;
	headEnvio.tipo_de_proceso=WORKER;
	headEnvio.tipo_de_mensaje=GIVE_TMPREDUCCIONLOCAL;
	char * buffer2;
	char * nombreArchivoReduccionPedido;
	char *rutaArchivoReduccionPedido;
	char * temporalEnEsteNodo=string_new();
	t_list * listaTemporales = list_create();
	for(i=0;i<list_size(infoReduccionGlobal->listaNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(infoReduccionGlobal->listaNodos,i);
		if(infoNodo->nodoEncargado!=1){//no soy yo.(soy el unico encargado(==1)
			printf("me conecto con y le pido su reduccion local del job %d\n",infoReduccionGlobal->job);
			printf(" nombre nodo: %s \n",infoNodo->nombreNodo);
			printf(" ip nodo: %s \n",infoNodo->ipNodo);
			printf(" peurto: %s \n",infoNodo->puertoNodo);
			printf(" temp red loc: %s \n",infoNodo->temporalReduccion);
			printf(" encargado: %d \n",infoNodo->nodoEncargado);
			sockWorkerVecino=conectarAServidor(infoNodo->ipNodo,infoNodo->puertoNodo);
			buffer2=serializeBytes(headEnvio,infoNodo->temporalReduccion,infoNodo->temporalReduccionLen,&packSize2);
			if ((stat = send(sockWorkerVecino, buffer2, packSize2, 0)) == -1){
				puts("no se pudo enviar path del archivo temporal que necesitamos. ");
				return  FALLO_SEND;
			}
			nombreArchivoReduccionPedido=string_new();
			rutaArchivoReduccionPedido = string_new();
			string_append(&rutaArchivoReduccionPedido,"tmp/");
			string_append(&nombreArchivoReduccionPedido,"temporalRLPedidoA");
			string_append(&nombreArchivoReduccionPedido,worker->nombre_nodo);
			string_append(&nombreArchivoReduccionPedido,"-nro");
			cont++;
			string_append(&nombreArchivoReduccionPedido,string_itoa(cont));
			string_append(&rutaArchivoReduccionPedido,nombreArchivoReduccionPedido);

			stat = recibirYAlmacenarArchivo(sockWorkerVecino,rutaArchivoReduccionPedido);
			list_add(listaTemporales,rutaArchivoReduccionPedido);
			puts("recibi el temporal");
			close(sockWorkerVecino);
		}else{
			string_append(&temporalEnEsteNodo,infoNodo->temporalReduccion);
		}
	}


	puts("Ya tengo todos los reductores. apareo todos");
	char * lineaDeEjecucionApareo;
	char * rutaDeApareoGlobal;
	lineaDeEjecucionApareo=string_new();
	string_append(&lineaDeEjecucionApareo,"sort -m");

	//agrego el temporal que esta en este nodo:
	string_append(&lineaDeEjecucionApareo," /home/utnso/");
	string_append(&lineaDeEjecucionApareo,temporalEnEsteNodo);

	for(i=0;i<list_size(listaTemporales);i++){
		char * ruta = list_get(listaTemporales,i);
		printf("Nombre del archivo %d a reducir: %s\n",i,ruta);
		string_append(&lineaDeEjecucionApareo," /home/utnso/");
		string_append(&lineaDeEjecucionApareo,ruta);

	}
	rutaDeApareoGlobal=string_new();
	//nombreArchivoTemporalApareado=string_new();
	string_append(&rutaDeApareoGlobal,"/home/utnso");
	string_append(&rutaDeApareoGlobal,"/tmp/ApareoGlobalnro");
	cont ++;
	string_append(&rutaDeApareoGlobal,string_itoa(cont));
	string_append(&rutaDeApareoGlobal,worker->nombre_nodo);

	string_append(&lineaDeEjecucionApareo," > ");
	string_append(&lineaDeEjecucionApareo,rutaDeApareoGlobal);

	printf("linea de ejec apareo global %s \n",lineaDeEjecucionApareo);
	stat=system(lineaDeEjecucionApareo);

	printf("Stat apareo global:%d\n",stat);

	puts("hago reduccion global\n");







	puts("Forkeo");
	char * lineaDeEjecucionReduccionGlobal;
	char * rutaResultadoReduccionGlobal;
	pid_t pidRedGl;
	if ( (pidRedGl=fork()) == 0 )
	{ /* hijo */
		//	printf("Soy el hijo (%d, hijo de %d)\n", getpid(),getppid());
		//	printf("%d\n",cont);


		lineaDeEjecucionReduccionGlobal = string_new();
		rutaResultadoReduccionGlobal=string_new();




		string_append(&lineaDeEjecucionReduccionGlobal,"cat ");
		string_append(&lineaDeEjecucionReduccionGlobal,rutaDeApareoGlobal);
		string_append(&lineaDeEjecucionReduccionGlobal," | ./");
		string_append(&lineaDeEjecucionReduccionGlobal,nombreScriptReductor);
		string_append(&lineaDeEjecucionReduccionGlobal, " > /home/utnso");
		string_append(&rutaResultadoReduccionGlobal,infoReduccionGlobal->tempRedGlobal);
		string_append(&lineaDeEjecucionReduccionGlobal,rutaResultadoReduccionGlobal);

		printf("linea de eecucion red global %s\n",lineaDeEjecucionReduccionGlobal);
		printf("Ruta resutlado reduccion %s\n",rutaResultadoReduccionGlobal);



		stat = system(lineaDeEjecucionReduccionGlobal);
		printf("Stat lineaDeEjecucion :%d \n",stat);
		Theader *head = malloc(sizeof head);
		head->tipo_de_proceso = WORKER;
		head->tipo_de_mensaje = FIN_REDUCCIONGLOBALOK;

		puts("Envio header. fin reduccion global ok");
		enviarHeader(client_sock,head);
		//close(client_sock);
		exit(0);

	}
	else
	{ /* padre */
		//	printf("Soy el padre (%d, hijo de %d)\n", getpid(),	getppid());
		//	printf("%d\n",cont);
		//waitpid(pid,pidStat,0);
	}
	return 0;











}

int realizarTransformacion(int client_sock){
	TpackDatosTransformacion *datosTransf;
			char * buff;
			char * lineaDeEjecucionTransformacion;
			char * rutaResultadoTransformacion;
			char * nombreScriptTransformador;
			char * rutaScriptTransformador ;
			Theader *headEnvio  = malloc(sizeof headEnvio);
			int stat;
			pid_t pid;

			puts("llego solicitud para nueva transformacion. recibimos bloque cant bytes y nombre temporal..");

			if ((buff = recvGenericWFlags(client_sock,0)) == NULL){
				puts("Fallo recepcion de datos de la transformacion");
				return FALLO_RECV;
			}

			if ((datosTransf = deserializarInfoTransformacionMasterWorker(buff)) == NULL){
				puts("Fallo deserializacion de Bytes de los datos de la transformacion");
				return FALLO_GRAL;
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

				printf("linea de eecucion %s\n",lineaDeEjecucionTransformacion);
	//			printf("Ruta resutlado Transformador %s\n",rutaResultadoTransformacion);



				stat = system(lineaDeEjecucionTransformacion);
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
			//	printf("Soy el padre (%d, hijo de %d)\n", getpid(),	getppid());
			//	printf("%d\n",cont);
				//waitpid(pid,pidStat,0);
			}
			//printf("Cierro fd %d\n",client_sock);
			//close(client_sock);
			return 0;
}

int realizarReduccionLocal(int client_sock){
	TinfoReduccionLocalMasterWorker *infoReduccion;
	char * bufferReduccion;
	char * nombreScriptReductor;
	char * rutaScriptReductor;
	char  *rutaResultadoReduccion;
	char  *rutaTemporalesApareados;
	//char * nombreArchivoTemporalApareado;
	char * lineaDeEjecucionReduccion;
	char * lineaDeEjecucionApareo;

	int stat;
	pid_t pidRed;
	Theader *headEnvio  = malloc(sizeof headEnvio);
	puts("Llego solicitud de inicio para reduccion local");


	if ((bufferReduccion = recvGenericWFlags(client_sock,0)) == NULL){
		puts("Fallo recepcion de datos de la reduccion local");
		return FALLO_RECV;
	}

	if ((infoReduccion = deserializarInfoReduccionLocalMasterWorker(bufferReduccion)) == NULL){
		puts("Fallo deserializacion de Bytes de los datos de la reduccion local");
		return FALLO_GRAL;
	}


	printf("\n\n\n esta e sla infoq  me llego");
	printf("Nombre temporal de la reduccion: %s\n",infoReduccion->nombreTempReduccion);

	int i;

	printf("LIST SIZE %d\n",infoReduccion->listaSize);
	lineaDeEjecucionApareo=string_new();
	string_append(&lineaDeEjecucionApareo,"sort -m");

	for(i=0;i<infoReduccion->listaSize;i++){
		TreduccionLista *infoAux = list_get(infoReduccion->listaTemporales,i);
		printf("Nombre del archivo %d a reducir: %s\n",i,infoAux->nombreTemporal);
		string_append(&lineaDeEjecucionApareo," /home/utnso/");
		string_append(&lineaDeEjecucionApareo,infoAux->nombreTemporal);

	}
	rutaTemporalesApareados=string_new();
	//nombreArchivoTemporalApareado=string_new();
	string_append(&rutaTemporalesApareados,"/home/utnso/tmp/apareoLocal");
	string_append(&rutaTemporalesApareados,worker->nombre_nodo);
	string_append(&rutaTemporalesApareados,"nro");
	string_append(&rutaTemporalesApareados,string_itoa(cantApareosGlobal++));

	string_append(&lineaDeEjecucionApareo," > ");
	string_append(&lineaDeEjecucionApareo,rutaTemporalesApareados);

	printf("linea de ejec apareo %s \n",lineaDeEjecucionApareo);
	system(lineaDeEjecucionApareo);
	puts("Ahora recibo el script reductor");


	nombreScriptReductor=string_new();
	rutaScriptReductor  = string_new();
	string_append(&rutaScriptReductor,"/home/utnso/");
	string_append(&nombreScriptReductor,"reductorLocal");
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




		string_append(&lineaDeEjecucionReduccion,"cat ");
		string_append(&lineaDeEjecucionReduccion,rutaTemporalesApareados);
		string_append(&lineaDeEjecucionReduccion," | ./");
		string_append(&lineaDeEjecucionReduccion,nombreScriptReductor);
		string_append(&lineaDeEjecucionReduccion, " > /home/utnso/");
		string_append(&rutaResultadoReduccion,infoReduccion->nombreTempReduccion);
		string_append(&lineaDeEjecucionReduccion,rutaResultadoReduccion);

		printf("linea de eecucion %s\n",lineaDeEjecucionReduccion);
		printf("Ruta resutlado reduccion %s\n",rutaResultadoReduccion);



		stat = system(lineaDeEjecucionReduccion);
		printf("Stat lineaDeEjecucion :%d \n",stat);

		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_REDUCCIONLOCALOK;

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
	return 0;
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
		fprintf(stdout, "Recibidos %d bytes y espero :- %d bytes\n", len, remain_data);
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
	printf("Stat lineaPermisoEjecucion :%d \n",stat);
	puts("Permisos de ejecucion otorgados al script recibido");

	return 0;

}


int recibirYAlmacenarArchivo(int client_sock,char * rutaAAlmacenar){


	char buffer[BUFSIZ];
	int len;
	int remain_data = 0;
	int stat;
	   char *ptr;

	long file_size2;
	//char file_size2[sizeof(int)];
	FILE *scriptFile;


	/*file size */
	stat=recv(client_sock, buffer, sizeof(long)*2, 0);
	printf("size rec stat %d\n",stat);
	//file_size2 = strtol(buffer);
	file_size2=strtol(buffer,&ptr,10);
	fprintf(stdout, "\nFile size : %lu\n", file_size2);

	scriptFile = fopen(rutaAAlmacenar, "w");
	if (scriptFile == NULL){
		fprintf(stderr, "Fallo open archivo file --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	remain_data = file_size2;

	while (remain_data > 0){//todo:cheq
		len = recv(client_sock, buffer, 1024, 0);
		fwrite(buffer, sizeof(char), len, scriptFile);
		remain_data -= len;
		fprintf(stdout, "Recibidos %d bytes y espero :- %d bytes\n", len, remain_data);
		//if(len<0) break;
	}
	fclose(scriptFile);

	puts("recibi archivo");
	printf("Ruta archivo  %s\n",rutaAAlmacenar);




	return 0;

}

int enviarArchivo(char * rutaArchivo,int sockDestino){

	int fdScript;
	int len,remain_data,sent_bytes;
	off_t offset;
	struct stat file_stat;
	char file_size[20];
	fdScript = open(rutaArchivo, O_RDONLY);
	if (fdScript == -1){
		fprintf(stderr, "Error abriendo archivo  --> %s", strerror(errno));
		return FALLO_GRAL;
	}

	/* file stats */
	if (fstat(fdScript, &file_stat) < 0){
		fprintf(stderr, "Error fstat --> %s", strerror(errno));
		return FALLO_GRAL;
	}

	fprintf(stdout, "File Size: \n %ld bytes\n",(long) file_stat.st_size);
	sprintf(file_size, "%ld",(long) file_stat.st_size);


	/* envio file size */
	len = send(sockDestino, file_size, sizeof(long)*2, 0);
	if (len < 0){
		fprintf(stderr, "Error enviando filesize --> %s", strerror(errno));
		return FALLO_GRAL;
	}

	fprintf(stdout, "Enviamos %d bytes del tamanio(%ld) del archivo \n", len,(long)file_stat.st_size);

	offset = 0;
	remain_data = file_stat.st_size;
	/* envio script data */

	while (((sent_bytes = sendfile(sockDestino, fdScript, &offset, 1024)) > 0) && (remain_data > 0)){
		remain_data -= sent_bytes;
		fprintf(stdout, "2.enviados %d bytes de data, offset : %ld and remain data = %d\n", sent_bytes,(long) offset, remain_data);
	}

	close(fdScript);
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

