#include "../funcionesFS.h"

void conexionesDatanode(void * estructura){
	char * mensaje = malloc(100);
	char * rutaLocalArchivoFinal;
	char * extensionArchivoFinal;
	char * archivoFinalMapeado;
	char * nombreDeArchivoFinalConExtension;
	char * rutaATemporal;
	char ** rutasParaCpfrom = malloc(sizeof(char *) * 4);
	int socketDeEscuchaDatanodes;
	int fileDescriptorMax = -1;
	int cantModificados = 0;
	int	nuevoFileDescriptor;
	int fileDescriptor;
	int	cantNodosPorConectar;
	int	estado;
	int fileDescriptorArchivoFinal;
	fd_set readFD, masterFD;
	FILE * archivoFinal;
	Tnodo * nuevoNodo;
	Tnodo * nodoEncontrado;
	TinfoNodo * infoNodoNuevo;
	TpackInfoBloqueDN * infoNodo;
	TfileSystem * fileSystem = (TfileSystem *) estructura;
	Theader * head = malloc(sizeof(Theader));
	TarchivoFinal * estructuraArchivoFinal = malloc(sizeof(TarchivoFinal));
	cantNodosPorConectar = fileSystem->cant_nodos;

	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);
	mostrarConfiguracion(fileSystem);
	socketDeEscuchaDatanodes = crearSocketDeEscucha(fileSystem->puerto_datanode);
	fileDescriptorMax = MAXIMO(socketDeEscuchaDatanodes, fileDescriptorMax);
	puts("antes de entrar al while");

	while (listen(socketDeEscuchaDatanodes, BACKLOG) == -1){
				log_error(logError, "Fallo al escuchar el socket servidor de file system.");
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
				logErrorAndExit("Fallo el select.");
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
						log_info(logInfo,"Nuevo nodo conectado.");
						fileDescriptorMax = MAXIMO(nuevoFileDescriptor, fileDescriptorMax);
						printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);
						break;
					}
						puts("Recibiendo...");

						estado = recv(fileDescriptor, head, sizeof(Theader), 0);

						if(estado == -1){
							log_error(logError, "Error al recibir información de un cliente.");
							break;
						}
						else if( estado == 0){
							nodoEncontrado = buscarNodoPorFD(listaDeNodos, fileDescriptor);
							list_add(listaDeNodosDesconectados, nodoEncontrado);
							borrarNodoPorFD(fileDescriptor);
							sprintf(mensaje, "Se desconecto el cliente de fd: %d.", fileDescriptor);
							log_error(logError, mensaje);
							clearAndClose(fileDescriptor, &masterFD);
							break;
						}
					if(head->tipo_de_proceso==DATANODE){
						switch(head->tipo_de_mensaje){
							case INFO_NODO:
								log_info(logInfo,"Recibiendo la informacion de datanode.");
								puts("Es datanode y quiere mandar la información del nodo");
								infoNodo = recvInfoNodo(fileDescriptor);
								if((Tnodo*)buscarNodoPorNombre(listaDeNodos, infoNodo->nombreNodo) == NULL){
									if((Tnodo*)buscarNodoPorFD(listaDeNodosDesconectados, fileDescriptor) == NULL){
										//nodo nuevo;
										nuevoNodo = malloc(sizeof(Tnodo));
										infoNodoNuevo = inicializarInfoNodo(infoNodo);
										nuevoNodo = inicializarNodo(infoNodo, fileDescriptor, nuevoNodo);
										list_add(listaInfoNodo,infoNodoNuevo);
										list_add(listaDeNodos, nuevoNodo);
										cantNodosPorConectar--;
										almacenarBitmap(nuevoNodo);
										agregarNodoATablaDeNodos(nuevoNodo);
									}
									else {//se reconecta;
										//pensar si hay que volver a inicializarlo al nodo que
										//se reconecta
										log_info(logInfo,"Es datanode y quiere reconectarse.");
										nuevoNodo = buscarNodoPorNombre(listaDeNodosDesconectados,infoNodo->nombreNodo);
										nuevoNodo->fd = fileDescriptor;
										list_add(listaDeNodos, nuevoNodo);
										//nuevoNodo = inicializarNodo(infoBloque, fileDescriptor, nuevoNodo);
										borrarNodoPorNombre(listaDeNodosDesconectados,nuevoNodo->nombre);
										log_error(logError, "Nodo que se habia caído, se reconecto.");
									}
								}
								else {
									puts("Un nodo ya conectado, se esta volviendo a conectar");
									clearAndClose(fileDescriptor, &masterFD);
									log_error(logError, "Un nodo ya conectado, se esta volviendo a conectar");
								}
								liberarTPackInfoBloqueDN(infoNodo);
								break;

							case OBTENER_BLOQUE_Y_NRO:
								puts("Es datanode y nos manda un bloque con su numero");
								int tamanio,nroBloque;
								char * bloque;

								if ((estado = recv(fileDescriptor, &nroBloque, sizeof(int), 0)) == -1) {
										logErrorAndExit("Error al recibir el nroBloque");
										}

								if ((estado = recv(fileDescriptor, &tamanio, sizeof(int), 0)) == -1) {
										logErrorAndExit("Error al recibir el tamanio do bloque");
										}
								bloque = malloc(tamanio);
								if ((estado = recv(fileDescriptor, bloque, sizeof(int), 0)) == -1) {
										logErrorAndExit("Error al recibir el contenido do bloque");
										}

								break;
							case OBTENER_BLOQUE:
								puts("Es datanode y nos mando un bloque");
								log_info(logInfo,"Es datanode y mando un bloque.");
								bloqueACopiar = malloc(sizeof(Tbuffer));
								int nroBloqueRecibido;
								if(recv(fileDescriptor, &bloqueACopiar->tamanio, sizeof(unsigned long long), 0) == -1){
									logErrorAndExit("Error al recibir el tamanio del bloque");
								}
								puts("voy a copiar el bloque");
								bloqueACopiar->buffer = malloc(bloqueACopiar->tamanio);
								if(recv(fileDescriptor, bloqueACopiar->buffer, bloqueACopiar->tamanio, MSG_WAITALL) == -1){
									logErrorAndExit("Error al recibir el contenido del bloque");
								}
								if(recv(fileDescriptor, &nroBloqueRecibido,sizeof(int),0) == -1){
									logErrorAndExit("Error al recibir el numero del bloque");
								}

								pthread_mutex_unlock(&bloqueMutex);

								break;
							default:
								puts("Tipo de Mensaje no encontrado en el protocolo");
								log_error(logError, "LLego un tipo de mensaje, no especificado en el protocolo de filesystem.");
								break;
					}

					printf("Recibi %d bytes\n",estado);
					printf("el proceso es %d\n", head->tipo_de_proceso);
					printf("el mensaje es %d\n", head->tipo_de_mensaje);
					break;

				}
				else if(head->tipo_de_proceso == WORKER){
						switch(head->tipo_de_mensaje){
							case ALMACENAR_ARCHIVO:
								log_info(logInfo,"Es worker y quiere almacenar el archivo final en yamafs.");
								rutaATemporal = malloc(250);
								desempaquetarArchivoFinal(fileDescriptor, estructuraArchivoFinal);

								//todo hay que probar esto
								char * directorioACrear = obtenerRutaSinArchivo(estructuraArchivoFinal->rutaArchivo);
								if(existeDirectorio(directorioACrear)){
									log_info("El directorio para el almacenamiento final, ya existe.");
								}
								else{
									log_info("El directorio para el almacenamiento final no existe, creando.");
									if(crearDirectorio(directorioACrear)>=0){
										persistirTablaDeDirectorios();
									}
									else{
										log_error("No se pudo crear el directorio del almacenamiento final.");
									}
								}
								//todo hay que probar esto de arriba


								rutaLocalArchivoFinal = obtenerRutaLocalDeArchivo(estructuraArchivoFinal->rutaArchivo);
								nombreDeArchivoFinalConExtension = obtenerNombreDeArchivoDeUnaRuta(rutaLocalArchivoFinal);
								extensionArchivoFinal = obtenerExtensionDeArchivoDeUnaRuta(rutaLocalArchivoFinal);
								sprintf(rutaATemporal, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/tmp/%s", nombreDeArchivoFinalConExtension);
								mkdir("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/tmp/", 0777);
								if(!strcmp(extensionArchivoFinal, "csv")){
									archivoFinal = fopen(rutaATemporal, "w+");
								}else{
									archivoFinal = fopen(rutaATemporal, "wb+");
								}
								truncate(rutaATemporal, estructuraArchivoFinal->tamanioContenido);
								fileDescriptorArchivoFinal = fileno(archivoFinal);

								if ((archivoFinalMapeado = mmap(NULL, estructuraArchivoFinal->tamanioContenido, PROT_WRITE, MAP_SHARED,	fileDescriptorArchivoFinal, 0)) == MAP_FAILED) {
									logErrorAndExit("Error al hacer mmap");
								}

								pasarInfoDeUnArchivoAOtro(estructuraArchivoFinal->contenidoArchivo, archivoFinalMapeado, estructuraArchivoFinal->tamanioContenido);

								close(fileDescriptorArchivoFinal);
								fclose(archivoFinal);
								if (msync(archivoFinalMapeado, estructuraArchivoFinal->tamanioContenido, MS_SYNC) < 0) {
									logErrorAndExit("Sucedio lo imposible, fallo el msync");
								}
								munmap(archivoFinalMapeado, estructuraArchivoFinal->tamanioContenido);

								rutasParaCpfrom[0] = strdup("cpfrom");
								rutasParaCpfrom[1] = strdup(rutaATemporal);
								rutasParaCpfrom[2] = strdup(estructuraArchivoFinal->rutaArchivo);
								rutasParaCpfrom[3] = NULL;
								log_info(logInfo, "Se creo un archivo temporal del archivo final.");

								if(almacenarArchivo(rutasParaCpfrom) == -1){
									log_error(logError, "Error al guardar el archivo final en yamafs.");
								}
								log_info(logInfo, "Se guardo el archivo final correctamente.");

								liberarPunteroDePunterosAChar(rutasParaCpfrom);
								free(rutasParaCpfrom);
								liberarEstructuraArchivoFinal(estructuraArchivoFinal);
								free(rutaLocalArchivoFinal);
								free(extensionArchivoFinal);
								free(estructuraArchivoFinal);
								free(rutaATemporal);
								free(directorioACrear);
								head->tipo_de_proceso=FILESYSTEM;
								head->tipo_de_mensaje=FIN_ALMACENAMIENTOFINALOK;
								enviarHeader(fileDescriptor,head);
								//todo: revisar

								break;
							default:
								log_error(logError, "Tipo de mensaje no encontrado en el protocolo.");
								puts("Tipo de mensaje no encontrado en el protocolo.");
								break;
						}
				}
				else{
					log_error(logError, "Se conecto a filesystem, un proceso que no es conocido/confiable. Expulsandolo...");
					clearAndClose(fileDescriptor, &masterFD);
				}

				} //termine el if

			} //termine el for

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

void formatearNodos(t_list * lista){
	int cantidadElementos = list_size(lista);
	Tnodo * nodo;
	int i = 0;
	while(i < cantidadElementos){
		nodo = (Tnodo *)list_get(lista, i);
		inicializarBitmap(nodo);
		almacenarBitmap(nodo);
		nodo->cantidadBloquesLibres = nodo->cantidadBloquesTotal;
		i++;
	}
}
