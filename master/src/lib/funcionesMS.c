#include "funcionesMS.h"

extern char * rutaTransformador;



int conectarseAWorkersTransformacion(t_list * bloquesTransformacion,int sockYama){


	int cantConexiones = list_size(bloquesTransformacion);

	int i;

	for(i=0;i< cantConexiones;i++){
		pthread_t workerThread[i];
		TpackInfoBloque *infoBloque=list_get(bloquesTransformacion,i);
		TatributosHilo * atributos = malloc(sizeof atributos);
		atributos->infoBloque.bloqueDelDatabin = infoBloque->bloqueDelDatabin;
		atributos->infoBloque.bloqueDelArchivo=infoBloque->bloqueDelArchivo;
		atributos->infoBloque.bytesOcupados = infoBloque->bytesOcupados;
		atributos->infoBloque.ipWorker=malloc(sizeof(infoBloque->tamanioIp));
		atributos->infoBloque.ipWorker = infoBloque->ipWorker;
		atributos->infoBloque.nombreNodo =malloc(sizeof( infoBloque->tamanioNombre));
		atributos->infoBloque.nombreNodo = infoBloque->nombreNodo;
		atributos->infoBloque.nombreTemporal =malloc(sizeof( infoBloque->nombreTemporalLen));
		atributos->infoBloque.puertoWorker = malloc(sizeof(infoBloque->tamanioPuerto));
		atributos->infoBloque.puertoWorker = infoBloque->puertoWorker;
		atributos->sockYama=sockYama;

		printf("creo hilo %d\n",i);
		crearHilo(&workerThread[i], (void*)workerHandler, (void*)atributos);
	}

	return 0;
}

void workerHandler(void *info){
	TatributosHilo *atributos = (TatributosHilo *)info;


	int sockYama = atributos->sockYama;


	int stat,sockWorker;
	int fdTransformador;
	int len,offset,remain_data,sent_bytes;
	struct stat file_stat;
	char file_size[sizeof(int)];
	char *buffer;
	int packSize;
	Theader *headEnvio = malloc(sizeof(headEnvio));
	bool finCorrecto = false;

	if((sockWorker = conectarAServidor(atributos->infoBloque.ipWorker, atributos->infoBloque.puertoWorker))<0){
		puts("No pudo conectarse a worker");
		return;
	}

	puts("Nos conectamos a worker");

	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	//enviarHeader(sockWorker,headEnvio);

	//Envio al worker el nro de bloque, el tamaño y el nombre temporal
	Theader headASerializar;
	headASerializar.tipo_de_mensaje=NUEVATRANSFORMACION;
	headASerializar.tipo_de_proceso=MASTER;

	buffer = serializarInfoTransformacionMasterWorker(headASerializar,atributos->infoBloque.bloqueDelDatabin,atributos->infoBloque.bytesOcupados,atributos->infoBloque.nombreTemporalLen,atributos->infoBloque.nombreTemporal,&packSize);


	printf("Info de la transformacion serializada, enviamos\n");
	if ((stat = send(sockWorker, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar info de la trasnformacion");
		return;
	}
	printf("se enviaron %d bytes de la info de la transformacion\n",stat);



	//envio el script
	fdTransformador = open(rutaTransformador, O_RDONLY);
	if (fdTransformador == -1){
		fprintf(stderr, "Error abriendo archivo transformador --> %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* file stats */
	if (fstat(fdTransformador, &file_stat) < 0){
		fprintf(stderr, "Error fstat --> %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

    fprintf(stdout, "File Size: \n %d bytes\n", file_stat.st_size);
	sprintf(file_size, "%d", file_stat.st_size);


	/* envio file size */
	len = send(sockWorker, file_size, sizeof(file_size), 0);
	if (len < 0){
		fprintf(stderr, "Error enviando filesize --> %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "Enviamos %d bytes del tamanio(%d) del script transformador \n", len,file_stat.st_size);

	offset = 0;
	remain_data = file_stat.st_size;
	/* envio script data */
	while (((sent_bytes = sendfile(sockWorker, fdTransformador, &offset, BUFSIZ)) > 0) && (remain_data > 0)){
		remain_data -= sent_bytes;
		fprintf(stdout, "2.enviados %d bytes de data, offset : %d and remain data = %d\n", sent_bytes, offset, remain_data);
	}

	close(fdTransformador);
	puts("al while");

	while ((stat=recv(sockWorker, &headRcv, HEAD_SIZE, 0)) > 0) {

		switch (headRcv.tipo_de_mensaje) {
		case(FIN_LOCALTRANSF):
			printf("Worker me avisa que termino de transformar el bloque %d\n",atributos->infoBloque.bloqueDelDatabin);
			finCorrecto = true;
			close(sockWorker);
		break;
		default:
			break;
		}


	}
	if(finCorrecto){
		puts("Termina la conexion con worker.. La transformacion salio OK. Le avisamos a yama ");
		headEnvio->tipo_de_proceso=MASTER;
		headEnvio->tipo_de_mensaje=FINTRANSFORMACIONLOCALOK;
		enviarHeader(sockYama,headEnvio);

	}else{
		puts("termino la conexion con worker de manera inesperada. Transformacion fallo. Le avisamos a yama");
		headEnvio->tipo_de_proceso=MASTER;
		headEnvio->tipo_de_mensaje=FINTRANSFORMACIONLOCALFAIL;
		enviarHeader(sockYama,headEnvio);
	}
	printf("fin thread de transfo del bloque del databin %d (bloque deol archivo :%d)\n",atributos->infoBloque.bloqueDelDatabin,atributos->infoBloque.bloqueDelArchivo);
}




Tmaster * obtenerConfiguracionMaster(char* ruta){
	Tmaster *master = malloc(sizeof(Tmaster));
	t_config * masterConfig = config_create(ruta);

	master->ipYama=malloc(MAXIMA_LONGITUD_IP);
	master->puertoYama = malloc(MAXIMA_LONGITUD_IP);

	strcpy(master->ipYama, config_get_string_value(masterConfig, "IP_YAMA"));
	strcpy(master->puertoYama, config_get_string_value(masterConfig, "PUERTO_YAMA"));

	master->tipoDeProceso = MASTER;

	config_destroy(masterConfig);
	return master;
}

void mostrarConfiguracion(Tmaster *master){

	printf("IP Yama %s\n",    master->ipYama);
	printf("Puerto Yama: %s\n",       master->puertoYama);
	printf("Tipo de proceso: %d\n", master->tipoDeProceso);
}
