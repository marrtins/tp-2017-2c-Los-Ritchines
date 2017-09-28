#include "funcionesYM.h"

void logAndExit(char * mensaje){
	log_error(logger,mensaje);
	exit(-1);
}

Tyama *obtenerConfiguracion(char* ruta){
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

int conectarAFS(int* socketFS, Tyama *yama){
	int estado;
	Theader *head = malloc(HEAD_SIZE);

	//Theader head;
	// Se trata de conectar con FS
	if ((*socketFS = conectarAServidor(yama->ip_filesystem, yama->puerto_filesystem)) < 0){
		fprintf(stderr, "No se pudo conectar con FS! sock_fs: %d\n", *socketFS);
		return FALLO_CONEXION;
	}

	head->tipo_de_proceso=YAMA;
	head->tipo_de_mensaje=INICIOYAMA;


	// No permitimos continuar la ejecucion hasta lograr un handshake con FS
	if ((estado = send(*socketFS, head, HEAD_SIZE, 0)) == -1){
		perror("Fallo send de handshake. error");
		printf("Fallo send() al socket: %d\n", *socketFS);
		return FALLO_SEND;
	}

	printf("Se enviaron: %d bytes a FS del handshake \n", estado);
	return 0;
}

char *recvGenericWFlags(int sock_in, int flags){
	//printf("Se recibe el paquete serializado, usando flags %x\n", flags);

	int stat, pack_size;
	char *p_serial;

	if ((stat = recv(sock_in, &pack_size, sizeof(int), flags)) == -1){
		perror("Fallo de recv. error");
		return NULL;

	} else if (stat == 0){
		printf("El proceso del socket %d se desconecto. No se pudo completar recvGenerico\n", sock_in);
		return NULL;
	}

	pack_size -= (HEAD_SIZE + sizeof(int)); // ya se recibieron estas dos cantidades
	//printf("Paquete de size: %d\n", pack_size);

	if ((p_serial = malloc(pack_size)) == NULL){
		printf("No se pudieron mallocar %d bytes para paquete generico\n", pack_size);
		return NULL;
	}

	if ((stat = recv(sock_in, p_serial, pack_size, flags)) == -1){
		perror("Fallo de recv. error");
		return NULL;

	} else if (stat == 0){
		printf("El proceso del socket %d se desconecto. No se pudo completar recvGenerico\n", sock_in);
		return NULL;
	}

	return p_serial;
}

char *recvGeneric(int sock_in){
	return recvGenericWFlags(sock_in, 0);
}

TpackBytes *deserializeBytes(char *bytes_serial){

	int off;
	TpackBytes *pbytes;

	if ((pbytes = malloc(sizeof *pbytes)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}

	off = 0;
	memcpy(&pbytes->bytelen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((pbytes->bytes = malloc(pbytes->bytelen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", pbytes->bytelen);
		return NULL;
	}

	memcpy(pbytes->bytes, bytes_serial + off, pbytes->bytelen);
	off += pbytes->bytelen;

	return pbytes;
}

void freeAndNULL(void **ptr){
	free(*ptr);
	*ptr = NULL;
}

void masterHandler(void *client_sock){
	int sock_master = (int *)client_sock;
	int estado;
	Theader * head = malloc(HEAD_SIZE);
	TpackSrcCode *entradaTransformador;
	TpackSrcCode *entradaReductor;
	TpackBytes *pathArchivoAReducir;
	TpackBytes *pathResultado;
	char* buffer;
	head->tipo_de_proceso = MASTER;
	head->tipo_de_mensaje = 0;
	puts("Nuevo hilo MASTERHANDLER creado");
	puts("Esperando solicitud de master");
	while((estado = recv(sock_master, &head, HEAD_SIZE, 0)) > 0){
			puts("Se recibio un paquete de Master");
			printf("proc %d \t msj %d \n", head->tipo_de_proceso, head->tipo_de_mensaje);

			switch(head->tipo_de_mensaje){
			case SRC_CODE_TRANSF:
					puts("Master quiere iniciar un JOB");
					puts("Nos llega el codigo fuente del script TRANSFORMDOR");

					if ((buffer = recvGeneric(sock_master)) == NULL){
						puts("Fallo recepcion de SRC_CODE TRANSFORMADOR");
						return;
					}

					if ((entradaTransformador = (TpackSrcCode *) deserializeBytes(buffer)) == NULL){

						puts("Fallo deserializacion de Bytes del src_transformador");
						return;
					}
					printf("Codigo fuente Transformador: %s \n",entradaTransformador->bytes);
					freeAndNULL((void **) &buffer);
					puts("fin case src_code_trans");
					break;
			case SRC_CODE_RED:

				puts("Nos llega el codigo fuente del script REDUCTOR");


				if ((buffer = recvGeneric(sock_master)) == NULL){
					puts("Fallo recepcion de SRC_CODE REDUCTOR");

					return;
				}

				if ((entradaReductor = (TpackSrcCode *) deserializeBytes(buffer)) == NULL){

					puts("Fallo deserializacion de Bytes del src_reductor");
					return;
				}

				printf("Codigo fuente reductor: %s \n",entradaReductor->bytes);
				freeAndNULL((void **) &buffer);
				puts("fin case src_code_red");

				break;
			case PATH_FILE_TOREDUCE:

				puts("Nos llega el path del archivo a reducir");


				if ((buffer = recvGeneric(sock_master)) == NULL){
					puts("Fallo recepcion de PATH_FILE_TOREDUCE");

					return;
				}

				if ((pathArchivoAReducir = (TpackBytes *) deserializeBytes(buffer)) == NULL){

					puts("Fallo deserializacion de Bytes del path arch a reducir");
					return;
				}

				printf("Path archivo a reducir: : %s\n",pathArchivoAReducir->bytes);
				freeAndNULL((void **) &buffer);
				puts("fin case PATH_FILE_TOREDUCE");

				break;
			case PATH_RES_FILE:
							puts("Nos llega el path del resultado");
							if ((buffer = recvGeneric(sock_master)) == NULL){
								puts("Fallo recepcion de PATH_RES_FILE");
								return;
							}
							if ((pathResultado = (TpackBytes *) deserializeBytes(buffer)) == NULL){
								puts("Fallo deserializacion de Bytes del path_res_file");
								return;
							}
							printf("Path del resultado: : %s\n",pathResultado->bytes);
							freeAndNULL((void **) &buffer);
							puts("fin case PATH_RES_FILE");
							//Como este es el último atributo que recibimos de master.  Aca le enviamos la "info" de los workers a los que se tiene que conectar
							//Por el momento info hardcode y sera la info de los unicos 2 workers conectados. Se la pedimos al filesystem y cuando nos la devuelve, le reenviamos a master.
							/* pido info, la proceso y ahora se a que workers el master se va a conectar y se los paso...*/
							//x ahora solo le aviso q inice la transf local
							//Theader * head; esto no sabemos por que va
							int stat;
							head->tipo_de_proceso=YAMA;
							head->tipo_de_mensaje=START_LOCALTRANSF;
							if((stat=enviarHeader(sock_master, head))<0){
								puts("No se pudo enviar el header de inicio");
							}
							break;
			default: break;
			}
		}
}
