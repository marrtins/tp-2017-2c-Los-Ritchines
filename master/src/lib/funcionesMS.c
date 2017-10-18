#include "funcionesMS.h"

extern char * rutaTransformador;



int conectarseAWorkersTransformacion(t_list * bloquesTransformacion){


	int cantConexiones = list_size(bloquesTransformacion);

	int i;

	for(i=0;i< cantConexiones;i++){
		pthread_t workerThread[i];
		TpackInfoBloque *infoBloque=list_get(bloquesTransformacion,i);
		printf("creo hilo %d\n",i);
		crearHilo(&workerThread[i], (void*)workerHandler, (void*)infoBloque);
	}

	return 0;
}

void workerHandler(void *info){
	TpackInfoBloque *infoBloque = (TpackInfoBloque *)info;

	int stat,sockWorker;
	int fdTransformador;
	int len,offset,remain_data,sent_bytes;
	struct stat file_stat;
	char file_size[256];


	if((sockWorker = conectarAServidor(infoBloque->ipWorker, infoBloque->puertoWorker))<0){
		puts("No pudo conectarse a worker");
		return;
	}

	puts("Nos conectamos a worker");

	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};

	Theader *headEnvio=malloc(sizeof headEnvio);
	headEnvio->tipo_de_proceso=MASTER;
	headEnvio->tipo_de_mensaje=TRANSFORMADORLEN;

	enviarHeader(sockWorker,headEnvio);


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
						printf("Worker me avisa que termino de transformar el bloque %d\n",infoBloque->bloque);
		break;
		default:
			break;
		}


	}

	printf("fin thread de transfo del bloque %d\n",infoBloque->bloque);
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
