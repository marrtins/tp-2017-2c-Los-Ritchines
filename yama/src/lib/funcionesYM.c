
#include "funcionesYM.h"

extern int retardoPlanificacion;
extern t_list * listaJobsMaster;




Tyama *obtenerConfiguracionYama(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	Tyama *yama = malloc(sizeof(Tyama));

	yama->ip_filesystem       =    malloc(MAXIMA_LONGITUD_IP);
	yama->puerto_entrada = malloc(MAXIMA_LONGITUD_PUERTO);
	yama->puerto_master = malloc(MAXIMA_LONGITUD_PUERTO);
	yama->puerto_filesystem = malloc(MAXIMA_LONGITUD_PUERTO);

	t_config *yamaConfig = config_create(ruta);

	strcpy(yama->ip_filesystem, config_get_string_value(yamaConfig, "IP_FILESYSTEM"));
	strcpy(yama->puerto_entrada, config_get_string_value(yamaConfig, "PUERTO_YAMA"));
	strcpy(yama->puerto_master, config_get_string_value(yamaConfig, "PUERTO_MASTER"));
	strcpy(yama->puerto_filesystem, config_get_string_value(yamaConfig, "PUERTO_FILESYSTEM"));


	yama->retardo_planificacion = config_get_int_value(yamaConfig, "RETARDO_PLANIFICACION");
	yama->algoritmo_balanceo =          config_get_int_value(yamaConfig, "ALGORITMO_BALANCEO");
	yama->disponibilidadBase =          config_get_int_value(yamaConfig, "DISPONIBILIDAD_BASE");
	yama->tipo_de_proceso = YAMA;

	retardoPlanificacion=yama->retardo_planificacion;
	setRetardoPlanificacion();

	config_destroy(yamaConfig);
	return yama;
}

void mostrarConfiguracion(Tyama *yama){

	printf("Puerto Entrada: %s\n",  yama->puerto_entrada);
	printf("IP Filesystem %s\n",    yama->ip_filesystem);
	printf("Puerto Master: %s\n",       yama->puerto_master);
	printf("Puerto Filesystem: %s\n", yama->puerto_filesystem);
	printf("Retardo Planificacion: %d\n",   yama->retardo_planificacion);
	printf("Algoritmo Balanceo: %s\n",getAlgoritmoBalanceo(yama->algoritmo_balanceo));
	printf("Retardo: %d\n",yama->retardo_planificacion);
	printf("Disponibilidad: %d\n",yama->disponibilidadBase);
	printf("Tipo de proceso: %d\n", yama->tipo_de_proceso);
}
char * getAlgoritmoBalanceo(int algoritmo){
	char * ret = string_new();
	if(algoritmo==CLOCK){
		string_append(&ret,"CLOCK");
	}else if(algoritmo==WCLOCK){
		string_append(&ret,"WCLOCK");
	}else{
		string_append(&ret,"VALOR INVALIDO");
	}
	return ret;
}


int conectarAFS(Tyama *yama){

	Theader *head = malloc(sizeof(Theader));
	char * mensaje = malloc(100);
	int socketFS;
	head->tipo_de_proceso=YAMA;
	head->tipo_de_mensaje=INICIO_YAMA;

	//Theader head;
	// Se trata de conectar con FS
	if ((socketFS = conectarAServidor(yama->ip_filesystem, yama->puerto_filesystem)) < 0){
		sprintf(mensaje, "No se pudo conectar con FS! sock_fs: %d\n", socketFS);
		logErrorAndExit(mensaje);
	}


	/*// No permitimos continuar la ejecucion hasta lograr un handshake con FS
	if ((estado = send(socketFS, head, sizeof(Theader), 0)) == -1){
		sprintf(mensaje, "Fallo send() al socket: %d\n", socketFS);
		logAndExit(mensaje);
	}

	printf("Se enviaron: %d bytes a FS del handshake \n", estado);
*/
	return socketFS;
}






int divideYRedondea(x, y){
   int a = (x -1)/y +1;

   return a;
}

TpackInfoBloqueDN * recvInfoNodoYAMA(int socketFS){
	int estado;
	TpackInfoBloqueDN * infoBloque = malloc(sizeof(TpackInfoBloqueDN));
	char * nombreNodo;
	char * ipNodo;
	char * puertoNodo;
	int databinEnMB;

	//Recibo el tamaño del nombre del nodo
	if ((estado = recv(socketFS, &infoBloque->tamanioNombre, sizeof(int), 0)) == -1) {
		logErrorAndExit("Error al recibir el tamanio del nombre del nodo");
		}
	printf("Para el tamaño del nombre recibi %d bytes\n", estado);
	nombreNodo = malloc(infoBloque->tamanioNombre);

	//Recibo el nombre del nodo
	if ((estado = recv(socketFS, nombreNodo, infoBloque->tamanioNombre, 0)) == -1) {
		logErrorAndExit("Error al recibir el nombre del nodo");
		}

	printf("Para el nombre del nodo recibi %d bytes\n", estado);

	//Recibo el tamanio de la ip del nodo
	if ((estado = recv(socketFS, &infoBloque->tamanioIp, sizeof(int), 0)) == -1) {
		logErrorAndExit("Error al recibir el tamanio del ip del nodo");
		}
	printf("Para el tamaño de la ip recibi %d bytes\n", estado);

	ipNodo = malloc(infoBloque->tamanioIp);

	//Recibo la ip del nodo
	if ((estado = recv(socketFS, ipNodo, infoBloque->tamanioIp, 0)) == -1) {
		logErrorAndExit("Error al recibir el ip del nodo");
		}

	printf("Para el la ip recibi %d bytes\n", estado);

	//Recibo el tamanio del puerto del nodo
	if ((estado = recv(socketFS, &infoBloque->tamanioPuerto, sizeof(int), 0)) == -1) {
		logErrorAndExit("Error al recibir el tamanio del puerto del nodo");
		}
	printf("Para el tamaño del puerto recibi %d bytes\n", estado);

	puertoNodo = malloc(infoBloque->tamanioPuerto);

	//Recibo el puerto del nodo
	if ((estado = recv(socketFS, puertoNodo, infoBloque->tamanioPuerto, 0)) == -1) {
		logErrorAndExit("Error al recibir el puerto del nodo");
		}

	//Recibo el databin en MB
	if ((estado = recv(socketFS, &databinEnMB, sizeof(int), 0)) == -1) {
		logErrorAndExit("Error al recibir el tamanio del databin");
		}

	printf("Para el tamanio del databin recibi %d bytes\n", estado);

	infoBloque = desempaquetarInfoNodo(infoBloque, nombreNodo, ipNodo, puertoNodo);
	free(nombreNodo);
	free(ipNodo);
	free(puertoNodo);

	return infoBloque;
}

char * recibirPathArchivo(int sockMaster){
	char * buffer;
	TpackBytes *pathArchivo;
	Theader head;
	int stat;
	//stat = recv(sockMaster, &head, sizeof(Theader), 0);


	puts("Nos llega el path del archivo");

	if ((buffer = recvGeneric(sockMaster)) == NULL){
		puts("Fallo recepcion de PATH_FILE_TOREDUCE");
		return NULL;
	}

	if ((pathArchivo = (TpackBytes *) deserializeBytes(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del path arch a reducir");
		return NULL;
	}

	printf("Path archivo: %s\n",pathArchivo->bytes);

	return pathArchivo->bytes;
}

TjobMaster * getJobPorMasterID(int masterId){
	int i;
	for(i=0;i<list_size(listaJobsMaster);i++){
		TjobMaster *ret = list_get(listaJobsMaster,i);
		if(ret->masterId==masterId){
			return ret;
		}
	}

	return NULL;
}

TjobMaster * getJobPorNroJob(int nroJob){
	int i;
	for(i=0;i<list_size(listaJobsMaster);i++){
		TjobMaster *ret = list_get(listaJobsMaster,i);
		if(ret->nroJob==nroJob){
			return ret;
		}
	}

	return NULL;
}





