#include "funcionesCompartidas.h"

void logAndExit(char * mensaje){
	log_error(logger,mensaje);
	exit(-1);
}

void setupHints(struct addrinfo *hints, int address_family, int socket_type, int flags){
	memset(hints, 0, sizeof *hints);
	hints->ai_family = address_family;
	hints->ai_socktype = socket_type;
	hints->ai_flags = flags;
}

int crearSocketDeEscucha(char * puertoDeEscucha){

	int estado, socketDeEscucha;
	struct addrinfo hints, * infoDelServer;
	char * mensaje = malloc(50);

	setupHints(&hints, AF_INET, SOCK_STREAM, AI_PASSIVE);

	if ((estado = getaddrinfo(NULL, puertoDeEscucha, &hints, &infoDelServer)) != 0){
		logAndExit("No se pudo crear el adrrinfo.");
	}

	if ((socketDeEscucha = socket(infoDelServer->ai_family, infoDelServer->ai_socktype, infoDelServer->ai_protocol)) == -1){
		logAndExit("No se pudo crear el socket.");
	}

	if ((bind(socketDeEscucha, infoDelServer->ai_addr, infoDelServer->ai_addrlen)) == -1){
		sprintf(mensaje, "Fallo al reservar el puerto %s. Es posible que el puerto este ocupado." , puertoDeEscucha);
		logAndExit(mensaje);
	}

	freeaddrinfo(infoDelServer);
	free(mensaje);
	return socketDeEscucha;
}

void crearHilo(pthread_t * nombreHilo, void * nombreFuncion, void * parametros){
	if(pthread_create(nombreHilo, NULL, nombreFuncion, parametros) < 0){
				logAndExit("No se pudo crear el hilo.");
		}
}

int aceptarCliente(int fileDescriptor){
	int socketAceptado;
	char * mensaje = malloc(50);
	struct sockaddr direccionDeCliente;
	socklen_t tamanioDeCliente = sizeof(direccionDeCliente);


	if((socketAceptado = accept(fileDescriptor, (struct sockaddr*) &direccionDeCliente, &tamanioDeCliente)) == -1){
		sprintf(mensaje, "Fallo la aceptacion del cliente, del FD servidor (fileSystem): %d", fileDescriptor);
		logAndExit(mensaje);
	}

	printf("el socket acceptado es: %d", socketAceptado);
	free(mensaje);
	return socketAceptado;
}

int conectarAServidor(char *ipDestino, char *puertoDestino){

	int estado;
	int socketDestino;
	struct addrinfo hints, *infoServer;

	setupHints(&hints, AF_INET, SOCK_STREAM, 0);

	if ((estado = getaddrinfo(ipDestino, puertoDestino, &hints, &infoServer)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(estado));
		return FALLO_GRAL;
	}

	if ((socketDestino = socket(infoServer->ai_family, infoServer->ai_socktype, infoServer->ai_protocol)) == -1){
		perror("No se pudo crear socket. error.");
		return FALLO_GRAL;
	}

	if ((estado = connect(socketDestino, infoServer->ai_addr, infoServer->ai_addrlen)) == -1){
		perror("No se pudo establecer conexion, fallo connect(). error");
		printf("Fallo conexion con destino IP: %s PORT: %s\n", ipDestino, puertoDestino);
		return FALLO_CONEXION;
	}

	freeaddrinfo(infoServer);

	return socketDestino;
}

int enviarHeader(int socketDestino,Theader * head){

	int estado;

	if ((estado = send(socketDestino, head, sizeof(Theader), 0)) == -1){
		logAndExit("Fallo al enviar el header");
	}

	return estado;
}

//FUNCIONES DE (DE)SERIALIZACION

char *serializeBytes(Theader head, char* buffer, int buffer_size, int *pack_size){

	char *bytes_serial;
	if ((bytes_serial = malloc(HEAD_SIZE + sizeof(int) + sizeof(int) + buffer_size)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}

	*pack_size = 0;
	memcpy(bytes_serial + *pack_size, &head, HEAD_SIZE);
	*pack_size += HEAD_SIZE;

	// hacemos lugar para el payload_size
	*pack_size += sizeof(int);

	memcpy(bytes_serial + *pack_size, &buffer_size, sizeof buffer_size);
	*pack_size += sizeof (int);
	memcpy(bytes_serial + *pack_size, buffer, buffer_size);
	*pack_size += buffer_size;

	memcpy(bytes_serial + HEAD_SIZE, pack_size, sizeof(int));

	return bytes_serial;
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

