
#include "funcionesYM.h"







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
	yama->tipo_de_proceso = YAMA;

	config_destroy(yamaConfig);
	return yama;
}

void mostrarConfiguracion(Tyama *yama){

	printf("Puerto Entrada: %s\n",  yama->puerto_entrada);
	printf("IP Filesystem %s\n",    yama->ip_filesystem);
	printf("Puerto Master: %s\n",       yama->puerto_master);
	printf("Puerto Filesystem: %s\n", yama->puerto_filesystem);
	printf("Retardo Planificacion: %d\n",   yama->retardo_planificacion);
	printf("Algoritmo Balanceo: %d\n", yama->algoritmo_balanceo);
	printf("Tipo de proceso: %d\n", yama->tipo_de_proceso);
}

void conectarAFS(Tyama *yama){
	int estado;
	Theader *head = malloc(sizeof(Theader));
	char * mensaje = malloc(100);

	head->tipo_de_proceso=YAMA;
	head->tipo_de_mensaje=INICIOYAMA;

	//Theader head;
	// Se trata de conectar con FS
	if ((socketFS = conectarAServidor(yama->ip_filesystem, yama->puerto_filesystem)) < 0){
		sprintf(mensaje, "No se pudo conectar con FS! sock_fs: %d\n", socketFS);
		logAndExit(mensaje);
	}


	// No permitimos continuar la ejecucion hasta lograr un handshake con FS
	if ((estado = send(socketFS, head, sizeof(Theader), 0)) == -1){
		sprintf(mensaje, "Fallo send() al socket: %d\n", socketFS);
		logAndExit(mensaje);
	}

	printf("Se enviaron: %d bytes a FS del handshake \n", estado);
}












