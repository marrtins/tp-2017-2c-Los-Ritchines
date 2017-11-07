
#include "funcionesYM.h"

extern int retardoPlanificacion;





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






int divideYRedondea(x, y){
   int a = (x -1)/y +1;

   return a;
}






