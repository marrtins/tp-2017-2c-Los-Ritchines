#include "../funcionesFS.h"

void conexionesDatanode(void * estructura){
	TfileSystem * fileSystem = (TfileSystem *) estructura;
	fd_set readFD, masterFD;
	int socketDeEscuchaDatanodes,
		fileDescriptorMax = -1,
		cantModificados = 0,
		nuevoFileDescriptor,
		fileDescriptor,
		cantNodosPorConectar = fileSystem->cant_nodos,
		estado;
	Theader * head = malloc(sizeof(Theader));
	char * mensaje = malloc(100);
	Tnodo * nuevoNodo;
	Tnodo * nodoEncontrado;
	TpackInfoBloqueDN * infoBloque;

	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);
	mostrarConfiguracion(fileSystem);
	socketDeEscuchaDatanodes = crearSocketDeEscucha(fileSystem->puerto_datanode);
	fileDescriptorMax = MAXIMO(socketDeEscuchaDatanodes, fileDescriptorMax);
	puts("antes de entrar al while");

	while (listen(socketDeEscuchaDatanodes, BACKLOG) == -1){
				log_trace(logger, "Fallo al escuchar el socket servidor de file system.");
				puts("Reintentamos...");
	}

	FD_SET(socketDeEscuchaDatanodes, &masterFD);
	printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);
	while(1){

			printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);

			puts("Entre al while");
			readFD = masterFD;
			puts("Voy a usar el select");
			if((cantModificados = select(fileDescriptorMax + 1, &readFD, NULL, NULL, NULL)) == -1){
				logAndExit("Fallo el select.");
			}
			printf("Cantidad de fd modificados: %d \n", cantModificados);
			puts("pude usar el select");

			for(fileDescriptor = 3; fileDescriptor <= fileDescriptorMax; fileDescriptor++){
				printf("Entre al for con el fd: %d\n", fileDescriptor);
				if(FD_ISSET(fileDescriptor, &readFD)){
					printf("Hay un file descriptor listo. El id es: %d\n", fileDescriptor);

					if(fileDescriptor == socketDeEscuchaDatanodes){
						nuevoFileDescriptor = conectarNuevoCliente(fileDescriptor, &masterFD);
						printf("Nuevo nodo conectado: %d\n", nuevoFileDescriptor);
						fileDescriptorMax = MAXIMO(nuevoFileDescriptor, fileDescriptorMax);
						printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);
						break;
					}
						puts("Recibiendo...");

						estado = recv(fileDescriptor, head, sizeof(Theader), 0);

						if(estado == -1){
							log_trace(logger, "Error al recibir información de un cliente.");
							break;
						}
						else if( estado == 0){
							puts("vamos a agregar el nodo desconectado a lista de desconectados");
							nodoEncontrado = buscarNodoPorFD(fileDescriptor);
							list_add(listaDeNodosDesconectados, nodoEncontrado);
							puts("vamos a borrar al negro por su file descriptor");
							borrarNodoPorFD(fileDescriptor);
							puts("eliminamos al nodo de la tabla do nodos");
							eliminarNodoDeTablaDeNodos(nodoEncontrado);
							sprintf(mensaje, "Se desconecto el cliente de fd: %d.", fileDescriptor);
							log_trace(logger, mensaje);
							clearAndClose(fileDescriptor, &masterFD);
							break;
						}
					if(head->tipo_de_proceso==DATANODE){
						switch(head->tipo_de_mensaje){
							case INFO_NODO:
								puts("Es datanode y quiere mandar la información del nodo");

								if((Tnodo*)buscarNodoPorFD(fileDescriptor) == NULL){
									infoBloque = recvInfoNodo(fileDescriptor);
									if((Tnodo*)buscarNodoDesconectadoPorFD(fileDescriptor) == NULL){
										//nodo nuevo;
										nuevoNodo = malloc(sizeof(Tnodo));
										nuevoNodo = inicializarNodo(infoBloque, fileDescriptor, nuevoNodo);
										printf("Para el nro de bloque recibi %d bytes\n", estado);
										list_add(listaDeNodos, nuevoNodo);
										puts("Nodo inicializado y guardado en la lista");
										mostrarBitmap(nuevoNodo->bitmap);
									}
									else {//se reconecta;
										//pensar si hay que volver a inicializarlo al nodo que
										//se reconecta
										list_add(listaDeNodos, buscarNodoDesconectadoPorFD(fileDescriptor));
										nuevoNodo = inicializarNodo(infoBloque, fileDescriptor, nuevoNodo);
										borrarNodoDesconectadoPorFD(fileDescriptor);
										log_trace(logger, "Nodo que se habia caído, se reconecto");
									}
									agregarNodoATablaDeNodos(nuevoNodo);
									liberarTPackInfoBloqueDN(infoBloque);
								}
								else {
									puts("viejo me estas mandando la info otra vez, media pila che");
								}
								cantNodosPorConectar--;
								break;

							default:
								puts("Tipo de Mensaje no encontrado en el protocolo");
								log_trace(logger, "LLego un tipo de mensaje, no especificado en el protocolo de filesystem.");
								break;
					}

					printf("Recibi %d bytes\n",estado);
					printf("el proceso es %d\n", head->tipo_de_proceso);
					printf("el mensaje es %d\n", head->tipo_de_mensaje);
					break;

				} else{
					printf("se quiso conectar el proceso: %d\n",head->tipo_de_proceso);
					puts("Hacker detected");
					log_trace(logger, "Se conecto a filesystem, un proceso que no es conocido/confiable. Expulsandolo...");
					clearAndClose(fileDescriptor, &masterFD);
					puts("Intruso combatido");
				}

				} //termine el if


				puts("Sali del if de ISSET");

			} //termine el for

			puts("sali del for");

		} // termina el while

	//POR ACA VA UN SIGNAL PARA INDICAR QUE FS YA TIENE TODOS LOS NODOS CONECTADOS.
}

int conectarNuevoCliente( int fileDescriptor, fd_set * bolsaDeFileDescriptors){
		int nuevoFileDescriptor = aceptarCliente(fileDescriptor);
		FD_SET(nuevoFileDescriptor, bolsaDeFileDescriptors);
		return nuevoFileDescriptor;
}

void clearAndClose(int fileDescriptor, fd_set* masterFD){
	FD_CLR(fileDescriptor, masterFD);
	close(fileDescriptor);
}
