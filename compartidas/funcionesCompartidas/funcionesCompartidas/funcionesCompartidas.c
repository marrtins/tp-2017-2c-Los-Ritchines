#include "funcionesCompartidas.h"

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
