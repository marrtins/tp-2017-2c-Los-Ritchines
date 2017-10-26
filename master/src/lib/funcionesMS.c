#include "funcionesMS.h"


int enviarScript(char * rutaScript,int sockDestino){

	int fdScript;
	int len,remain_data,sent_bytes;
	off_t offset;
	struct stat file_stat;
	char file_size[sizeof(int)];
	fdScript = open(rutaScript, O_RDONLY);
	if (fdScript == -1){
		fprintf(stderr, "Error abriendo archivo transformador --> %s", strerror(errno));
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
	len = send(sockDestino, file_size, sizeof(file_size), 0);
	if (len < 0){
		fprintf(stderr, "Error enviando filesize --> %s", strerror(errno));
		return FALLO_GRAL;
	}

	fprintf(stdout, "Enviamos %d bytes del tamanio(%ld) del script transformador \n", len,(long)file_stat.st_size);

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
