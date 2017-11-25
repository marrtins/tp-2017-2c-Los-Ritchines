#include "funcionesWK.h"
#include <commons/config.h>
extern char * archivoMapeado;

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
	//fprintf(stdout, "\nFile size : %d\n", file_size);
	log_info(logInfo,"50");
	//free(buffer);
	log_info(logInfo,"51");
	scriptFile = fopen(rutaAAlmacenar, "w");
	if (scriptFile == NULL){
		fprintf(stderr, "Fallo open script file --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	remain_data = file_size;
	log_info(logInfo,"en recv script");
	while (remain_data > 0){//todo:cheq
		log_info(logInfo,"adentro recv script");
		len = recv(client_sock, buffer, 1024, 0);
		fwrite(buffer, sizeof(char), len, scriptFile);
		remain_data -= len;
		log_info(logInfo,"52");
		//free(buffer);
		log_info(logInfo,"53");
		//fprintf(stdout, "Recibidos %d bytes y espero :- %d bytes\n", len, remain_data);
		//if(len<0) break;
	}
	fclose(scriptFile);
	log_info(logInfo,"pase recv script");

	log_info(logInfo,"recibi archivo");
	log_info(logInfo,"Ruta Script  %s\n",rutaAAlmacenar);



	lineaPermisoEjecucion=string_new();
	string_append(&lineaPermisoEjecucion,"chmod 777 ");
	string_append(&lineaPermisoEjecucion,rutaAAlmacenar);
	log_info(logInfo,"%s \n",lineaPermisoEjecucion);
	stat=system(lineaPermisoEjecucion);
	//log_info(logInfo,"54");
	free(lineaPermisoEjecucion);
	//log_info(logInfo,"55");
	if(stat != 0){
		puts("error al dar chmod 777");
		return -1;
	}
	log_info(logInfo,"Stat lineaPermisoEjecucion :%d \n",stat);
	log_info(logInfo,"Permisos de ejecucion otorgados al script recibido");

	return 0;

}


int recibirYAlmacenarArchivo(int client_sock,char * rutaAAlmacenar){


	char buffer[BUFSIZ];
	int len;
	int remain_data = 0;
	int stat;
	   char *ptr;

	long file_size2;
	FILE *scriptFile;


	/*file size */
	stat=recv(client_sock, buffer, sizeof(long)*2, 0);
	log_info(logInfo,"size rec stat %d\n",stat);
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

	log_info(logInfo,"recibi archivo");
	log_info(logInfo,"Ruta archivo  %s\n",rutaAAlmacenar);




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
	//printf("Ruta del archivo de configuracion: %s\n", ruta);

	Tworker *worker = malloc(sizeof(Tworker));

	worker->ip_filesystem       =    malloc(MAXIMA_LONGITUD_IP);
	worker->ip_nodo       =    malloc(MAXIMA_LONGITUD_IP);
	worker->puerto_entrada = malloc(MAXIMA_LONGITUD_PUERTO);
	worker->puerto_master = malloc(MAXIMA_LONGITUD_PUERTO);
	worker->puerto_datanode = malloc(MAXIMA_LONGITUD_PUERTO);
	worker->puerto_filesystem = malloc(MAXIMA_LONGITUD_PUERTO);
	worker->ruta_databin=malloc(MAXIMA_LONGITUD_RUTA);
	worker->nombre_nodo=malloc(MAXIMA_LONGITUD_RUTA);

	t_config *workerConfig = config_create(ruta);

	strcpy(worker->ip_filesystem, config_get_string_value(workerConfig, "IP_FILESYSTEM"));
	strcpy(worker->ip_nodo, config_get_string_value(workerConfig, "IP_NODO"));
	strcpy(worker->puerto_entrada, config_get_string_value(workerConfig, "PUERTO_WORKER"));
	strcpy(worker->puerto_datanode, config_get_string_value(workerConfig, "PUERTO_DATANODE"));
	strcpy(worker->puerto_master, config_get_string_value(workerConfig, "PUERTO_MASTER"));
	strcpy(worker->puerto_filesystem, config_get_string_value(workerConfig, "PUERTO_FILESYSTEM"));
	strcpy(worker->ruta_databin, config_get_string_value(workerConfig, "RUTA_DATABIN"));
	strcpy(worker->nombre_nodo, config_get_string_value(workerConfig, "NOMBRE_NODO"));
	worker->tamanio_databin_mb = config_get_int_value(workerConfig, "TAMANIO_DATABIN_MB");

	config_destroy(workerConfig);
	return worker;
}

void liberarConfiguracionWorker(Tworker *worker){
	free(worker->ip_filesystem);
	free(worker->ip_nodo);
	free(worker->puerto_entrada);
	free(worker->puerto_master);
	free(worker->puerto_filesystem);
	free(worker->puerto_datanode);
	free(worker->ruta_databin);
	free(worker->nombre_nodo);
	free(worker);
}

void mostrarConfiguracion(Tworker *worker){

	printf("Puerto Entrada: %s\n",  worker->puerto_entrada);
	printf("IP Filesystem %s\n",    worker->ip_filesystem);
	printf("IP Nodo %s\n",    worker->ip_nodo);
	printf("Puerto Master: %s\n",       worker->puerto_master);
	printf("Puerto Filesystem: %s\n", worker->puerto_filesystem);
	printf("Puerto Worker: %s\n", worker->puerto_datanode);
	printf("Ruta Databin: %s\n", worker->ruta_databin);
	printf("Nombre Nodo: %s\n", worker->nombre_nodo);
	printf("Tamanio databin en MB: %d\n", worker->tamanio_databin_mb);
	printf("Tipo de proceso: %d\n", worker->tipo_de_proceso);
}

Tbuffer * empaquetarArchivoFinal(Theader * header, char * rutaArchivo, char * contenidoArchivo, unsigned long long tamanioArchivoFinal){
	Tbuffer * buffer = malloc(sizeof(Tbuffer));
	buffer->tamanio = sizeof(Theader) + sizeof(int) + (strlen(rutaArchivo) + 1) + sizeof(unsigned long long) + tamanioArchivoFinal;
	buffer->buffer = malloc(buffer->tamanio);
	int tamanioRuta = strlen(rutaArchivo) + 1;
	char * p = buffer->buffer;
	memcpy(p, header, sizeof(Theader));
	p += sizeof(Theader);
	memcpy(p, &tamanioRuta, sizeof(tamanioRuta) );
	p += sizeof(tamanioRuta);
	memcpy(p, rutaArchivo, strlen(rutaArchivo) + 1);
	p += strlen(rutaArchivo) + 1;
	memcpy(p, &tamanioArchivoFinal, sizeof(tamanioArchivoFinal));
	p += sizeof(tamanioArchivoFinal);
	memcpy(p, contenidoArchivo, tamanioArchivoFinal);

	return buffer;

}

char * getBloqueWorker(int posicion){
	//todo: revisar esta funcion
	/*FILE * archivo = fopen(worker->ruta_databin, "rb");

	int fd;

	fd = fileno(archivo);

	if ((archivoMapeado = mmap(NULL, worker->tamanio_databin_mb*BLOQUE_SIZE, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		logErrorAndExit("Error al hacer mmap");
	}
	fclose(archivo);
	close(fd);
*/

	char * bloque= malloc(BLOQUE_SIZE);
	memcpy(bloque, archivoMapeado + posicion*BLOQUE_SIZE,BLOQUE_SIZE);
	return bloque;

}
