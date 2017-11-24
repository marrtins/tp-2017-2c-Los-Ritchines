#include "../funcionesFS.h"

void conexionesDatanode(void * estructura){
	char * mensaje = malloc(100);
	char * rutaLocalArchivoFinal;
	char * extensionArchivoFinal;
	char * archivoFinalMapeado;
	char * nombreDeArchivoFinalConExtension;
	char * rutaATemporal;
	char ** rutasParaCpfrom;
	char * directorioACrear;
	int socketDeEscuchaDatanodes;
	int fileDescriptorMax = -1;
	int cantModificados = 0;
	int	nuevoFileDescriptor;
	int cantNodosPorConectar;
	int fileDescriptor;
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
	socketDeEscuchaDatanodes = crearSocketDeEscucha(fileSystem->puerto_datanode);
	fileDescriptorMax = MAXIMO(socketDeEscuchaDatanodes, fileDescriptorMax);

	while (listen(socketDeEscuchaDatanodes, BACKLOG) == -1){
				log_error(logError, "Fallo al escuchar el socket servidor de file system, reintentando.");
	}

	FD_SET(socketDeEscuchaDatanodes, &masterFD);
	while(1){

			readFD = masterFD;
			if((cantModificados = select(fileDescriptorMax + 1, &readFD, NULL, NULL, NULL)) == -1){
				logErrorAndExit("Fallo el select de los datanodes.");
			}

			for(fileDescriptor = 3; fileDescriptor <= fileDescriptorMax; fileDescriptor++){
				if(FD_ISSET(fileDescriptor, &readFD)){

					if(fileDescriptor == socketDeEscuchaDatanodes){
						nuevoFileDescriptor = conectarNuevoCliente(fileDescriptor, &masterFD);
						log_info(logInfo,"Nuevo cliente conectado (nodo o worker).");
						fileDescriptorMax = MAXIMO(nuevoFileDescriptor, fileDescriptorMax);
						break;
					}

					estado = recv(fileDescriptor, head, sizeof(Theader), 0);

					if(estado == -1){
						log_error(logError, "Error al recibir información de un datanode.");
						break;
					}
					else if( estado == 0){
						nodoEncontrado = buscarNodoPorFD(listaDeNodos, fileDescriptor);
						list_add(listaDeNodosDesconectados, nodoEncontrado);
						borrarNodoPorFD(fileDescriptor);
						sprintf(mensaje, "Se desconecto un datanode de fd: %d.", fileDescriptor);
						log_error(logError, mensaje);
						clearAndClose(fileDescriptor, &masterFD);
						break;
					}

					if(head->tipo_de_proceso==DATANODE){
						switch(head->tipo_de_mensaje){
							case INFO_NODO:
								log_info(logInfo,"Recibiendo la informacion de datanode (IP, TAMANIO, ETC).");
								infoNodo = recvInfoNodo(fileDescriptor);
								if((Tnodo*)buscarNodoPorNombre(listaDeNodos, infoNodo->nombreNodo) == NULL){
									if((Tnodo*)buscarNodoPorFD(listaDeNodosDesconectados, fileDescriptor) == NULL){
										//nodo nuevo;
										nuevoNodo = malloc(sizeof(Tnodo));
										infoNodoNuevo = inicializarInfoNodo(infoNodo);
										nuevoNodo = inicializarNodo(infoNodo, fileDescriptor, nuevoNodo);
										list_add(listaInfoNodo,infoNodoNuevo);
										list_add(listaDeNodos, nuevoNodo);
										almacenarBitmap(nuevoNodo);
										agregarNodoATablaDeNodos(nuevoNodo);
										verificarSiEsEstable(cantNodosPorConectar);

									}
									else {//se reconecta;
										//pensar si hay que volver a inicializarlo al nodo que
										//se reconecta
										log_info(logInfo,"Un datanode quiere reconectarse.");
										nuevoNodo = buscarNodoPorNombre(listaDeNodosDesconectados,infoNodo->nombreNodo);
										nuevoNodo->fd = fileDescriptor;
										list_add(listaDeNodos, nuevoNodo);
										//nuevoNodo = inicializarNodo(infoBloque, fileDescriptor, nuevoNodo);
										borrarNodoPorNombre(listaDeNodosDesconectados,nuevoNodo->nombre);
										log_info(logInfo, "Nodo que se habia caído, se reconecto.");
									}
								}
								else {
									clearAndClose(fileDescriptor, &masterFD);
									log_error(logError, "Un nodo ya conectado, se esta volviendo a conectar");
								}

								liberarTPackInfoBloqueDN(infoNodo);
								break;

							case OBTENER_BLOQUE:
								log_info(logInfo,"Un datanode nos mando un bloque.");
								bloqueACopiar = malloc(sizeof(Tbuffer));
								int nroBloqueRecibido;
								if(recv(fileDescriptor, &bloqueACopiar->tamanio, sizeof(unsigned long long), 0) == -1){
									logErrorAndExit("Error al recibir el tamanio del bloque");
								}
								bloqueACopiar->buffer = malloc(bloqueACopiar->tamanio);
								if(recv(fileDescriptor, bloqueACopiar->buffer, bloqueACopiar->tamanio, MSG_WAITALL) == -1){
									logErrorAndExit("Error al recibir el contenido del bloque de datanode.");
								}
								if(recv(fileDescriptor, &nroBloqueRecibido,sizeof(int),0) == -1){
									logErrorAndExit("Error al recibir el numero del bloque de datanode.");
								}

								pthread_mutex_unlock(&bloqueMutex);

								break;
							default:
								log_error(logError, "LLego un tipo de mensaje, no especificado en el protocolo de filesystem.");
								break;
					}

					break;

				}
				else if(head->tipo_de_proceso == WORKER){
						switch(head->tipo_de_mensaje){
							case ALMACENAR_ARCHIVO:
								log_info(logInfo,"Es worker y quiere almacenar el archivo final en yamafs.");
								rutaATemporal = malloc(250);
								rutasParaCpfrom = malloc(sizeof(char *) * 4);
								desempaquetarArchivoFinal(fileDescriptor, estructuraArchivoFinal);

								if(verificarDisponibilidadDeEspacioEnNodos(estructuraArchivoFinal->tamanioContenido ) == 0 ){
									free(rutasParaCpfrom);
									liberarEstructuraArchivoFinal(estructuraArchivoFinal);
									free(rutaATemporal);
									log_error(logError, "No hay suficiente espacio en los nodos, para almacenar el archivo final.");
									break;
								}

								//todo hay que probar esto
								directorioACrear = obtenerRutaSinArchivo(estructuraArchivoFinal->rutaArchivo);
								if(existeDirectorio(directorioACrear)){
									log_info(logInfo, "El directorio para el almacenamiento final, ya existe.");
								}
								else{
									log_info(logInfo, "El directorio para el almacenamiento final no existe, creando.");
									if(crearDirectorio(directorioACrear)>=0){
										persistirTablaDeDirectorios();
									}
									else{
										log_error(logError,"No se pudo crear el directorio del almacenamiento final.");
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
									logErrorAndExit("Error al hacer mmap para el archivo final.");
								}

								pasarInfoDeUnArchivoAOtro(estructuraArchivoFinal->contenidoArchivo, archivoFinalMapeado, estructuraArchivoFinal->tamanioContenido);

								close(fileDescriptorArchivoFinal);
								fclose(archivoFinal);
								if (msync(archivoFinalMapeado, estructuraArchivoFinal->tamanioContenido, MS_SYNC) < 0) {
									logErrorAndExit("Fallo el msync para actualizar el archivo final en disco.");
								}
								munmap(archivoFinalMapeado, estructuraArchivoFinal->tamanioContenido);

								rutasParaCpfrom[0] = strdup("cpfrom");
								rutasParaCpfrom[1] = strdup(rutaATemporal);
								rutasParaCpfrom[2] = strdup(directorioACrear);
								rutasParaCpfrom[3] = NULL;
								log_info(logInfo, "Se creo un archivo temporal del archivo final.");

								if(almacenarArchivo(rutasParaCpfrom) == -1){
									log_error(logError, "Error al guardar el archivo final en yamafs.");
								}
								log_info(logInfo, "Se guardo el archivo final correctamente.");

								remove(rutaATemporal);

								liberarPunteroDePunterosAChar(rutasParaCpfrom);
								free(rutasParaCpfrom);
								liberarEstructuraArchivoFinal(estructuraArchivoFinal);
								free(rutaLocalArchivoFinal);
								free(extensionArchivoFinal);
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

	//todo POR ACA VA UN SIGNAL PARA INDICAR QUE FS YA TIENE TODOS LOS NODOS CONECTADOS.
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

void verificarSiEsEstable(int cantNodosPorConectar) {

	if (list_size(listaDeNodos) == cantNodosPorConectar) {
		if (esEstadoRecuperado) {
			if(todosLosArchivosTienenCopias() && losNodosConectadosSonLosQueEstabanAntes()) {
				sem_post(&yama);
				puts("FILE SYSTEM ESTABLE");
			}
		} else {
			sem_post(&yama);
			puts("FILE SYSTEM ESTABLE");
		}
	}
}

