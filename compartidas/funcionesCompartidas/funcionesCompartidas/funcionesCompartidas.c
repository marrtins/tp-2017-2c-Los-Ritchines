#include "funcionesCompartidas.h"

/*void logMensaje(char * mensaje){
	FILE * archivoLog = fopen("/home/utnso/buenasPracticas/fileSystem/src/logs/logs.txt", "w");
	char * mensajeAGuardar = malloc(strlen(mensaje) + 2);
	strcpy(mensajeAGuardar, mensaje);
	strcat(mensajeAGuardar, "\n");
	fputs(mensajeAGuardar, archivoLog);
	free(mensajeAGuardar);

}*/

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

	//dada la estructura addrinfo, le cargo los demas datos que paso por parametro. Es una función inicializadora
	//los otros parametros son: la familia de socket que se va a utilizar (TCP/IP) u otro. Null es Default
	// Tipo de puerto, puede ser stream o datagram (elegimos stream)
	// Flags, frecuentemente se pone en null, pero en este caso le pusimos passive que no me acuerdo que significaba
	// mala mia
	setupHints(&hints, AF_INET, SOCK_STREAM, AI_PASSIVE);

	//getaddrinfo está explicada en utns.com
	//primer parametro es la ip, si es null, toma el localhost (ip local)
	//el segundo parametro es el puerto
	// el tercero es la estructura que inicializamos anteriormente, lo que hace la función es parsear
	// la direccion y el puerto, y tomar los otros datos para armarme la estructura que le paso por 4to parametro
	if ((estado = getaddrinfo(NULL, puertoDeEscucha, &hints, &infoDelServer)) != 0){
		//fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(estado));
		logAndExit("No se pudo crear el adrrinfo.");
	}
	//creo el socket, le mando la familia de protocolos (TCP/IP), el tipo de socket(stream) y el protocolo que
	//para esta familia puede ser creo que pone el cero el file descriptor, no entiendo muy bienTCP o UPC, elegimos TCP
	if ((socketDeEscucha = socket(infoDelServer->ai_family, infoDelServer->ai_socktype, infoDelServer->ai_protocol)) == -1){
		logAndExit("No se pudo crear el socket.");
	}

	//reservamos el puerto que nos enviaron (127.0.0.1:puerto)
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
	int socketDestino; // file descriptor para el socket del destino a conectar
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
