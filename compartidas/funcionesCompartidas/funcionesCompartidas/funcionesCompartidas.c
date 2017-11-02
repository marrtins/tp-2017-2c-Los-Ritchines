#include "funcionesCompartidas.h"

void logAndExit(char * mensaje){
	log_error(logger,mensaje);
	exit(-1);
}

int contarPunteroDePunteros(char ** puntero){
	char ** aux = puntero;
	int contador = 0;
	while(*aux != NULL){
		contador++;
		aux++;
	}
	return contador;
}

void liberarPunteroDePunterosAChar(char** palabras){
	int i = 0;
	while(palabras[i] != NULL){
		free(palabras[i]);
		i++;
	}

}

char * obtenerUltimoElementoDeUnSplit(char ** palabras){
	char * ultimaPalabra;
	int i = 0;
	while(palabras[i] != NULL){
		ultimaPalabra = palabras[i];
		i++;
	}
	return strdup(ultimaPalabra);
}

char* deUnsignedLongLongAString(int number) {
    return string_from_format("%ull", number);
}

char * obtenerExtensionDeUnArchivo(char * nombreArchivoConExtension){
	char ** palabras = string_split(nombreArchivoConExtension, ".");
	char * extension = strdup(palabras[1]);
	liberarPunteroDePunterosAChar(palabras);
	free(palabras);
	return extension;

}

char * obtenerNombreDeArchivoSinExtension(char * nombreDeArchivoConExtension){
	char ** palabras = string_split(nombreDeArchivoConExtension, ".");
	char * nombreArchivo = strdup(palabras[0]);
	liberarPunteroDePunterosAChar(palabras);
	free(palabras);
	return nombreArchivo;
}

int cantidadParametros(char ** palabras){
	int i = 1;
	while(palabras[i] != NULL){
		i++;
	}
	return i-1;
}

unsigned long long tamanioArchivo(FILE * archivo){
	int tamanio;
	fseek(archivo, 0L, SEEK_END);
	tamanio = ftell(archivo);
	fseek(archivo, 0L, SEEK_SET);
	return tamanio;
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


void freeAndNULL(void **ptr){
	free(*ptr);
	*ptr = NULL;
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

	pack_size -= (sizeof(Theader) + sizeof(int)); // ya se recibieron estas dos cantidades
	printf("Paquete de size: %d\n", pack_size);

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

int obtenerBloqueDisponible(Tnodo * nodo){
	unsigned int i = 0;
	while(i < nodo->cantidadBloquesTotal){
		if(bitarray_test_bit(nodo->bitmap, i) == 0){
			return i;
		}
		i++;
	}
	return -1;
}

