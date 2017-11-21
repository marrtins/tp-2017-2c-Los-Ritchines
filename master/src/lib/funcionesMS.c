#include "funcionesMS.h"

extern t_list * transfEjecutando,*transfFin,*rlEjecutando,*rlFin,*duracionTransformaciones,*duracionRL;
extern pthread_mutex_t mux_transfEjecutando,mux_transfFin,mux_rlEjecutando,mux_rlFin,mux_duracionTransformaciones,mux_duracionRL;
extern int maximoTransformacionesParalelas,maximoReduccionesParalelas,cantTransformaciones,cantRL;
extern time_t horaInicio;
extern double duracionRG,duracionAlmacenado;
extern int cantFallos;

void mostrarMetricasJob(){
	time_t horaFin;
	time(&horaFin);
	double duracionJob = difftime(horaFin, horaInicio);
	double promedioTransformacion = getPromedioTransformaciones();
	double promedioRL = getPromedioRL();
	printf("tiempo total ejecucion del job: %.f segundos\n",duracionJob);
	printf("promedio de transformacion: %.f segundos\n",promedioTransformacion);
	printf("promedio de reducciones locales: %.f segundos\n",promedioRL);
	printf("tiempo de reduccion global: %.f segundos\n",duracionRG);
	printf("tiempo de almacenamiento finla: %.f segundos\n",duracionAlmacenado);
	printf("cantidad de transformaciones en paralelo: %d\n",maximoTransformacionesParalelas);
	printf("cantidad de reducciones locales en paralelo: %d\n",maximoReduccionesParalelas);
	printf("cantidad de transformaciones: %d\n",cantTransformaciones);
	printf("cantidad de reducciones: %d\n",cantRL);
	printf("cantidad de fallos: %d\n",cantFallos);


	log_info(logInfo,"tiempo total ejecucion del job: %.f segundos\n",duracionJob);
	log_info(logInfo,"promedio de transformacion: %.f segundos\n",promedioTransformacion);
	log_info(logInfo,"promedio de reducciones locales: %.f segundos\n",promedioRL);
	log_info(logInfo,"tiempo de reduccion global: %.f segundos\n",duracionRG);
	log_info(logInfo,"tiempo de almacenamiento finla: %.f segundos\n",duracionAlmacenado);
	log_info(logInfo,"cantidad de transformaciones en paralelo: %d\n",maximoTransformacionesParalelas);
	log_info(logInfo,"cantidad de reducciones locales en paralelo: %d\n",maximoReduccionesParalelas);
	log_info(logInfo,"cantidad de transformaciones: %d\n",cantTransformaciones);
	log_info(logInfo,"cantidad de reducciones: %d\n",cantRL);
	log_info(logInfo,"cantidad de fallos: %d\n",cantFallos);


}


void inicioEjecucionTransformacion(int id){

	MUX_LOCK(&mux_transfEjecutando);
	cantTransformaciones++;
	list_add(transfEjecutando,(void *)id);

	int transformacionesEnEsteMomento = list_size(transfEjecutando);

	if(transformacionesEnEsteMomento > maximoTransformacionesParalelas){
		maximoTransformacionesParalelas=transformacionesEnEsteMomento;
	}
	MUX_UNLOCK(&mux_transfEjecutando);

	return;

}

double getPromedioTransformaciones(){
	int i;
	int duracion=0;
	for(i=0;i<list_size(duracionTransformaciones);i++){
		int aux =(int)  list_get(duracionTransformaciones,i);
		duracion +=aux;
	}
	//return duracion;
	return (duracion / list_size(duracionTransformaciones));
}

double getPromedioRL(){
	int i;
	double duracion=0.0;
	for(i=0;i<list_size(duracionRL);i++){
		int aux =(int)  list_get(duracionTransformaciones,i);
				duracion +=aux;
	}
	return (duracion / list_size(duracionRL));
}
void finEjecucionTransformacion(int id,int segundosDuracion){
	MUX_LOCK(&mux_transfEjecutando);
	removerTransformacionDeLista(id);
	MUX_UNLOCK(&mux_transfEjecutando);

	MUX_LOCK(&mux_transfFin);
	list_add(transfFin,(void *)id);
	MUX_UNLOCK(&mux_transfFin);

	MUX_LOCK(&mux_duracionTransformaciones);
	list_add(duracionTransformaciones,(void *)segundosDuracion);
	MUX_UNLOCK(&mux_duracionTransformaciones);
	return;
}

void removerTransformacionFallida(int id){
	MUX_LOCK(&mux_transfEjecutando);
	int i;
	for(i=0;i<list_size(transfEjecutando);i++){
		int aux =(int) list_get(transfEjecutando,i);
		if(aux==id){
			list_remove(transfEjecutando,i);
			MUX_UNLOCK(&mux_transfEjecutando);
			return;
		}
	}
	MUX_UNLOCK(&mux_transfEjecutando);
	return;
}





void inicioEjecucionRL(int id){
	MUX_LOCK(&mux_rlEjecutando);
	cantRL++;
	list_add(rlEjecutando,(void *)id);

	int reduccionesEnEsteMomento = list_size(rlEjecutando);

	if(reduccionesEnEsteMomento > maximoReduccionesParalelas){
		maximoReduccionesParalelas=reduccionesEnEsteMomento;
	}
	MUX_UNLOCK(&mux_rlEjecutando);

	return;

}
void finEjecucionRL(int id,int segundosDuracion){
	MUX_LOCK(&mux_rlEjecutando);
	removerReduccionDeLista(id);
	MUX_UNLOCK(&mux_rlEjecutando);

	MUX_LOCK(&mux_rlFin);
	list_add(rlFin,(void *)id);
	MUX_UNLOCK(&mux_rlFin);

	MUX_LOCK(&mux_duracionRL);
	list_add(duracionRL,(void *)segundosDuracion);
	MUX_UNLOCK(&mux_duracionRL);
	return;

}

void removerTransformacionDeLista(int id){
	int i;
	for(i=0;i<list_size(transfEjecutando);i++){
		int aux = (int)list_get(transfEjecutando,i);
		if(aux==id){
			list_remove(transfEjecutando,i);
			return;
		}
	}
	return;
}

void removerReduccionDeLista(int id){
	int i;
	for(i=0;i<list_size(rlEjecutando);i++){
		int aux = (int)list_get(rlEjecutando,i);
		if(aux==id){
			list_remove(rlEjecutando,i);
			return;
		}
	}
	return;
}

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

	//fprintf(stdout, "File Size: \n %ld bytes\n",(long) file_stat.st_size);
	sprintf(file_size, "%ld",(long) file_stat.st_size);


	/* envio file size */
	len = send(sockDestino, file_size, sizeof(file_size), 0);
	if (len < 0){
		fprintf(stderr, "Error enviando filesize --> %s", strerror(errno));
		return FALLO_GRAL;
	}

	//fprintf(stdout, "Enviamos %d bytes del tamanio(%ld) del script transformador \n", len,(long)file_stat.st_size);

	offset = 0;
	remain_data = file_stat.st_size;
	/* envio script data */

	while (((sent_bytes = sendfile(sockDestino, fdScript, &offset, 1024)) > 0) && (remain_data > 0)){
		remain_data -= sent_bytes;
		//fprintf(stdout, "2.enviados %d bytes de data, offset : %ld and remain data = %d\n", sent_bytes,(long) offset, remain_data);
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
