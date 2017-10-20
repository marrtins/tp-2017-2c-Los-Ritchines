#include "funcionesFS.h"

TfileSystem * obtenerConfiguracionFS(char* ruta){
	TfileSystem *fileSystem = malloc(sizeof(TfileSystem));

	fileSystem->puerto_entrada = malloc(MAXIMA_LONGITUD_PUERTO);
	fileSystem->puerto_datanode = malloc(MAXIMA_LONGITUD_PUERTO);
	fileSystem->puerto_yama = malloc(MAXIMA_LONGITUD_PUERTO);
	fileSystem->ip_yama = malloc(MAXIMA_LONGITUD_IP);

	t_config *fsConfig = config_create(ruta);

	strcpy(fileSystem->puerto_entrada, config_get_string_value(fsConfig, "PUERTO_FILESYSTEM"));
	strcpy(fileSystem->puerto_datanode, config_get_string_value(fsConfig, "PUERTO_DATANODE"));
	strcpy(fileSystem->puerto_yama, config_get_string_value(fsConfig, "PUERTO_YAMA"));
	strcpy(fileSystem->ip_yama, config_get_string_value(fsConfig, "IP_YAMA"));
	fileSystem->cant_nodos = config_get_int_value(fsConfig, "CANT_NODOS");

	printf("Cant de nodos %d\n", fileSystem->cant_nodos);

	fileSystem->tipo_de_proceso = FILESYSTEM;

	config_destroy(fsConfig);

	return fileSystem;
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

void mostrarConfiguracion(TfileSystem *fileSystem){

	printf("Puerto Entrada: %s\n",  fileSystem->puerto_entrada);
	printf("Puerto Datanode: %s\n",       fileSystem->puerto_datanode);
	printf("Puerto Yama: %s\n", fileSystem->puerto_yama);
	printf("IP Yama: %s\n", fileSystem->ip_yama);
	printf("Tipo de proceso: %d\n", fileSystem->tipo_de_proceso);
}

void agregarArchivosATablaDeArchivos(){

}

void liberarPunteroDePunterosAChar(char** palabras){
	int i = 0;
	while(palabras[i] != NULL){
		free(palabras[i]);
		i++;
	}

}

int cantidadParametros(char ** palabras){
	int i = 1;
	while(palabras[i] != NULL){
		i++;
	}
	return i-1;
}


bool ordenarSegunBloquesDisponibles(void* nodo1, void* nodo2){
	Tnodo* nodoA = (Tnodo*)nodo1;
	Tnodo* nodoB = (Tnodo*)nodo2;

	double obtenerProporcionDeDisponibilidad(Tnodo* nodo){
		if(nodo->cantidadBloquesLibres == 0) return 1;
		return (double)nodo->cantidadBloquesLibres/ nodo->cantidadBloquesTotal;
	}
	return obtenerProporcionDeDisponibilidad(nodoA) < obtenerProporcionDeDisponibilidad(nodoB);
}

void ocuparProximoBloqueBitmap(Tnodo * nodo){
	bitarray_set_bit(nodo->bitmap, nodo->primerBloqueLibreBitmap);
	nodo->primerBloqueLibreBitmap++;
}

void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar){
	Theader *head = malloc(sizeof(Theader));
	 char * buffer;
	 head->tipo_de_proceso=FILESYSTEM;
	 head->tipo_de_mensaje=ALMACENAR_BLOQUE;

	list_sort(listaDeNodos, ordenarSegunBloquesDisponibles);
	Tnodo* nodo1 = (Tnodo*)list_get(listaDeNodos, 0);
	Tnodo* nodo2 = (Tnodo*)list_get(listaDeNodos, 1);
	//hacer el send a cada nodo
	buffer = empaquetarBloque(head,bloque->numeroDeBloque,bloque->tamanio,bloque->contenido);
	printf("Numero de bloque %d , Tamanio de bloque %llu, Cntenido de bloque %s \n", bloque->numeroDeBloque,bloque->tamanio,bloque->contenido);
	 if ((send(nodo1->fd, buffer , sizeof(Theader), 0)) == -1){
	 		logAndExit("Fallo al enviar a Nodo el bloque a almacenar");
	 	}
	 puts("Se envio bloque a Nodo1");
	 if ((send(nodo2->fd, buffer , sizeof(Theader), 0)) == -1){
	 			logAndExit("Fallo al enviar a Nodo el bloque a almacenar");
	 		}
	 puts("Se envio bloque a Nodo2");

	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaCero.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	 strcpy(estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaCero.nombreDeNodo, nodo1->nombre);
	 puts("copia cero nombre de nodo listo");

	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaCero.numeroBloqueDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	 strcpy(estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaCero.numeroBloqueDeNodo,string_itoa(nodo1->primerBloqueLibreBitmap));
	 puts("numero de bloque de la copia cero");

	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaUno.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	 strcpy(estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaUno.nombreDeNodo, nodo2->nombre);


	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaUno.numeroBloqueDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	 strcpy(estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaUno.numeroBloqueDeNodo, string_itoa(nodo2->primerBloqueLibreBitmap));
	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].bytes = bloque->tamanio;
	 puts("Metio todo en la estructura");
}

void inicializarTablaDeDirectorio(){

}

int existeDirectorio(char * directorio){
	char ** carpeta = string_split(directorio, "/");

	int i=0, j,booleano=0;
	bool encontrado;
	int indicePadre = 0;

	while(carpeta[i] != NULL && booleano !=-1){

		for(j=0;j<=99;j++){
		encontrado = string_equals_ignore_case(tablaDirectorios[j].nombre,carpeta[i]);
		if(encontrado){
			if(tablaDirectorios[j].padre == indicePadre){
				indicePadre = tablaDirectorios[j].index;
			}else{
				encontrado = 0;
				booleano = -1;
			}
			break;
		}
		}
		i++;
	}
	return encontrado;

}

int buscarIndexPorNombreDeDirectorio(char * directorio){
	int i, comparacion;
	for(i = 0; i < 100; i++){
		comparacion = strcmp(tablaDirectorios[i].nombre, directorio);
		if(comparacion == 0){
			return tablaDirectorios[i].index;
		}
	}
	return -1;
}

char * obtenerUltimoElementoDeUnSplit(char ** palabras){
	char * ultimaPalabra;
	int i = 0;
	while(palabras[i] != NULL){
		ultimaPalabra = palabras[i];
		i++;
	}
	return ultimaPalabra;
}

int obtenerIndexDeUnaRuta(char * rutaDestino){
	char ** palabras = string_split(rutaDestino, "/");
	char * directorio = obtenerUltimoElementoDeUnSplit(palabras);
	return buscarIndexPorNombreDeDirectorio(directorio);

}

int cantidadDeBloquesDeUnArchivo(unsigned long long tamanio){
	float tamanioBloque = BLOQUE_SIZE;
	return ceil((float)tamanio / tamanioBloque);
}

char * generarStringDeBloqueNCopiaN(int numeroDeBloque, int numeroDeCopia){
	char * stringGenerado = malloc(TAMANIO_BLOQUE_N_COPIA_N);
	sprintf(stringGenerado, "BLOQUE%dCOPIA%d", numeroDeBloque, numeroDeCopia);
	return stringGenerado;
}

char * generarStringDeBloqueNBytes(int numeroDeBloque){
	char * stringGenerado = malloc(TAMANIO_BLOQUE_N_COPIA_N);
	sprintf(stringGenerado, "BLOQUE%dBYTES", numeroDeBloque);
	return stringGenerado;
}

char* deUnsignedLongLongAString(int number) {
    return string_from_format("%ull", number);
}

void almacenarEstructuraArchivoEnUnArchivo(Tarchivo * archivoAAlmacenar, char * rutaArchivo){
	t_config * archivoConfig = config_create(rutaArchivo);
	int cantidadDeBloques = cantidadDeBloquesDeUnArchivo(archivoAAlmacenar->tamanioTotal);
	char * bloqueNCopiaN;
	char * bloqueNCopiaN2;
	char * bloqueNBytes;
	int i = 0;

	config_set_value(archivoConfig, "TAMANIO", string_itoa(archivoAAlmacenar->tamanioTotal));
	config_set_value(archivoConfig, "TIPO", archivoAAlmacenar->extensionArchivo);

	while(cantidadDeBloques != 0){

		bloqueNCopiaN = generarStringDeBloqueNCopiaN(i,0);
		//aca Meteria el valor de bloqueNcopia0
		bloqueNCopiaN2 = generarStringDeBloqueNCopiaN(i,1);
		//aca Meteria el valor de bloqueNcopia1

		//aca hace el tamanio
		bloqueNBytes = generarStringDeBloqueNBytes(i);
		//hay que pasar un long a string
		config_set_value(archivoConfig, bloqueNBytes, string_itoa(archivoAAlmacenar->bloques[i].bytes));
		cantidadDeBloques--;
		i++;
		free(bloqueNCopiaN);
		free(bloqueNCopiaN2);
		free(bloqueNBytes);
	}

	config_save(archivoConfig);
	config_destroy(archivoConfig);

}

void guardarTablaDeArchivo(Tarchivo * archivoAAlmacenar, char * rutaDestino){
	int index = obtenerIndexDeUnaRuta(rutaDestino);
	char * rutaArchivo = malloc(200);
	sprintf(rutaArchivo, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/%d/%s.%s", index, archivoAAlmacenar->nombreArchivoSinExtension, archivoAAlmacenar->extensionArchivo);
	FILE * archivo = fopen(rutaArchivo, "w+");
	fclose(archivo);
	almacenarEstructuraArchivoEnUnArchivo(archivoAAlmacenar, rutaArchivo);

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

void liberarEstructuraBloquesAEnviar(TbloqueAEnviar * infoBloque){
	free(infoBloque->contenido);
	free(infoBloque);
}

void almacenarArchivo(char **palabras){

	//palabras[1] --> ruta archivo a almacenar
	//palabras[2] --> ruta de nuestro directorio
	puts("Entre a almacenarArchivo");
	char * archivoMapeado;
	char ** lineas;
	char ** splitDeRuta = string_split(palabras[1], "/");
	char * nombreArchivoConExtension = obtenerUltimoElementoDeUnSplit(splitDeRuta);
	printf("El archivo a guardar es: %s\n", nombreArchivoConExtension);

	Tarchivo * archivoAAlmacenar = malloc(sizeof(Tarchivo));
	archivoAAlmacenar->nombreArchivoSinExtension = obtenerNombreDeArchivoSinExtension(nombreArchivoConExtension);
	puts("obtuvo el nombre del archivo sin extension");
	archivoAAlmacenar->extensionArchivo = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);
	puts("obtuvo extension");
	printf("El nombre del archivo es: %s\n", obtenerNombreDeArchivoSinExtension(nombreArchivoConExtension));
	printf("La extensión es es: %s\n", obtenerExtensionDeUnArchivo(nombreArchivoConExtension));

	unsigned long long bytesDisponiblesEnBloque = BLOQUE_SIZE;
	TbloqueAEnviar * infoBloque = malloc(sizeof(TbloqueAEnviar));
	infoBloque->numeroDeBloque = 0;
	infoBloque->contenido = malloc(BLOQUE_SIZE);

	FILE * archivoOrigen = fopen(palabras[1], "r");
	int fd = fileno(archivoOrigen);

	unsigned long long tamanio = tamanioArchivo(archivoOrigen);
	archivoAAlmacenar->tamanioTotal = tamanio;

	printf("El tamaño del archivo es: %llu\n", tamanio);


	if ((archivoMapeado = mmap(NULL, tamanio, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		logAndExit("Error al hacer mmap");
	}

	fclose(archivoOrigen);

	int cantidadBloques = cantidadDeBloquesDeUnArchivo(tamanio);
	archivoAAlmacenar->bloques = malloc(sizeof(Tbloques) * cantidadBloques);

	printf("La cantidad de bloquees es: %d \n", cantidadBloques);

	if(tamanio == 0){
		puts("Error al almacenar archivo, está vacío");
		return;
	}
	else {
		puts("El archivo no es vacio");
		lineas = string_split(archivoMapeado,"\n");
		puts("Pude splitear el archivo mapeado.");
		int i = 0;
		printf("%c = caracter final\n",archivoMapeado[tamanio-1]);

		while(lineas[i]!=NULL){
			//+1 por el \n faltante
			if(lineas[i+1] == NULL){
				puts("Bloque no lleno, cargandolo...");
				bytesDisponiblesEnBloque -= strlen(lineas[i])+1;
				string_append(&infoBloque->contenido,lineas[i]);
				if(lineas[i+2] != NULL){
					string_append(&infoBloque->contenido,"\n");
				}
				if(lineas[i+1] == NULL && archivoMapeado[tamanio-1] == '\n'){
					string_append(&infoBloque->contenido,"\n");

				}
			}
			if((bytesDisponiblesEnBloque - strlen(lineas[i]) +1 < 0) || lineas[i+1] == NULL){
				puts("Preparando todo para enviar el bloque ya lleno.");
				infoBloque->tamanio = BLOQUE_SIZE - bytesDisponiblesEnBloque;
				archivoAAlmacenar->bloques->bytes = infoBloque->tamanio;
				enviarBloque(infoBloque, archivoAAlmacenar);
				bytesDisponiblesEnBloque = BLOQUE_SIZE;
				infoBloque->numeroDeBloque++;
			}
			else{
				puts("Bloque no lleno, cargandolo...");
				bytesDisponiblesEnBloque -= strlen(lineas[i])+1;
				string_append(&infoBloque->contenido,lineas[i]);
				if(lineas[i+1] != NULL){
					string_append(&infoBloque->contenido,"\n");
				}

			}
			i++;

		}
		puts("ya almacene el archivo");
		guardarTablaDeArchivo(archivoAAlmacenar, palabras[2]);

		close(fd);

		liberarPunteroDePunterosAChar(splitDeRuta);
		free(splitDeRuta);
		free(nombreArchivoConExtension);
		liberarPunteroDePunterosAChar(lineas);
		//hay que ver esto de liberar tabla de archivos
		liberarTablaDeArchivo(archivoAAlmacenar);
		liberarEstructuraBloquesAEnviar(infoBloque);
		free(lineas);
	}
}

void procesarInput(char* linea) {
	int cantidad = 0;
	char **palabras = string_split(linea, " ");
	cantidad = cantidadParametros(palabras);
	if (string_equals_ignore_case(*palabras, "format")) {
		printf("ya pude formatear el fs\n");
	} else if (string_equals_ignore_case(*palabras, "rm")) {
		printf("ya pude remover el archivo\n");
	} else if (string_equals_ignore_case(*palabras, "rename")) {
		printf("ya pude renombrar el archivo\n");
	} else if (string_equals_ignore_case(*palabras, "mv")) {
		printf("ya pude mover el archivo\n");
	} else if (string_equals_ignore_case(*palabras, "cat")) {
		printf("ya pude leer el archivo\n");
	} else if (string_equals_ignore_case(*palabras, "mkdir")) {
		if(cantidad == 1){
			if(existeDirectorio(palabras[1])){
				puts("Existe el directorio");
			}else {
				puts("No existe el directorio"); //HAY QUE CREARLO
				printf("ya pude crear el directorio\n");
			}
		}
	} else if (string_equals_ignore_case(*palabras, "cpfrom")) {
		if(cantidad == 2){
			if(existeDirectorio(palabras[2])){
			puts("Existe el directorio");
			almacenarArchivo(palabras);
			}else {
				puts("No existe el directorio");
			}
		}
		else {
			puts("Error en la cantidad de parametros");
		}

	} else if (string_equals_ignore_case(*palabras, "cpto")) {
		printf("ya pude copiar un archivo local al file system\n");
	} else if (string_equals_ignore_case(*palabras, "cpblock")) {
		printf("ya pude crear una copia de un bloque del archivo en un nodo\n");
	} else if (string_equals_ignore_case(*palabras, "md5")) {
		printf("ya pude solicitar el md5 de un archivo del file system\n");
	} else if (string_equals_ignore_case(*palabras, "ls")) {
		printf("ya pude listar los archivos del directorio\n");
	} else if (string_equals_ignore_case(*palabras, "info")) {
		printf("ya pude mostrar la informacion del archivo\n");
	} else if(string_equals_ignore_case(*palabras, "exit")){
		printf("Finalizando consola\n");
		liberarPunteroDePunterosAChar(palabras);
		free(palabras);
		free(linea);
		pthread_kill(pthread_self(),SIGKILL);
	} else {
		printf("No existe el comando\n");
	}
	liberarPunteroDePunterosAChar(palabras);
	free(palabras);
	free(linea);
}

void consolaFS(){
	puts("Bienvenido a la consola. Ingrese un comando:");
	while(1){
		char *linea = readline(">");
		procesarInput(linea);
	}
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

void liberarNodoDeLaListaGlobal(Tnodo* nodo){
	free(nodo->nombre);
	bitarray_destroy(nodo->bitmap);
	free(nodo);
}

void* buscarPorFD(int fd){
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd == fd;
	}

	return list_find(listaDeNodos, buscarPorFDParaLista);
}

void borrarPorFD(int fd){
	Tnodo* nodoABorrar = (Tnodo*)buscarPorFD(fd);
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd==fd;
	}
	list_remove_by_condition(listaDeNodos,buscarPorFDParaLista);
	liberarNodoDeLaListaGlobal(nodoABorrar);
}

void inicializarNodo(int fileDescriptor, char* buffer){
	//hay que inicializar el bitarray;
	Tnodo* nodo = (Tnodo*)buscarPorFD(fileDescriptor);
	//aca se deserializa el buffer que contiene la info del nodo;
	//y se almacena en el puntero nodo que apunta al nodo de la lista global de nodos;
}

void conexionesDatanode(void * estructura){
	TfileSystem * fileSystem = (TfileSystem *) estructura;
	fd_set readFD, masterFD;
	int socketDeEscuchaDatanodes,
		fileDescriptorMax = -1,
		cantModificados = 0,
		nuevoFileDescriptor,
		fileDescriptor,
		estable,
		cantNodosPorConectar = fileSystem->cant_nodos,
		estado;
	Theader * head = malloc(sizeof(Theader));
	char * mensaje = malloc(100);
	char * streamInfoNodo;
	Tnodo * nuevoNodo;

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
							borrarPorFD(fileDescriptor);
							sprintf(mensaje, "Se desconecto el cliente de fd: %d.", fileDescriptor);
							log_trace(logger, mensaje);
							clearAndClose(fileDescriptor, &masterFD);
						}
					if(head->tipo_de_proceso==DATANODE){
						switch(head->tipo_de_mensaje){
							case INFO_NODO:
								nuevoNodo = malloc(sizeof(Tnodo));
								nuevoNodo->fd = nuevoFileDescriptor;
								cantNodosPorConectar--;
								//hay que volver a recv lo que sigue después del head;
								//recv el nombre nodo, bloques totales, bloques libres;
								//y los va a meter en la estructura Tnodo;
								inicializarNodo(fileDescriptor,streamInfoNodo);
								list_add(listaDeNodos,nuevoNodo);
								break;

							default:
								puts("Tipo de Mensaje no encontrado en el protocolo");
								break;
					}

					printf("Recibi %d bytes\n",estado);
					printf("el proceso es %d\n", head->tipo_de_proceso);
					printf("el mensaje es %d\n", head->tipo_de_mensaje);
					break;

				} else{
					puts("Hacker detected");
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

void levantarTablasDirectorios(){
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "r");
	int i = 0;

	while(!feof(archivoDirectorios)){
		fscanf(archivoDirectorios, "%d %s %d", &tablaDirectorios[i].index, tablaDirectorios[i].nombre, &tablaDirectorios[i].padre);
		printf("%d \t %s \t %d \n", tablaDirectorios[i].index, tablaDirectorios[i].nombre, tablaDirectorios[i].padre);
		i++;
	}

	fclose(archivoDirectorios);

}

void mostrarDirectorios(){

}


void levantarTablaArchivo(Tarchivo * tablaArchivos){

	t_config *archivo = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/2/archivo1.txt");

	tablaArchivos->extensionArchivo = malloc(TAMANIO_EXTENSION_ARCHIVO);


	int cantBloques,
			nroBloque = 0;
	char **temporal1;
	char **temporal2;
	char* bloqueCopia0;
	char* bloqueCopia1;
	char* bloqueBytes;

	tablaArchivos->tamanioTotal = config_get_int_value(archivo, "TAMANIO");
	strcpy(tablaArchivos->extensionArchivo, config_get_string_value(archivo, "TIPO"));

	cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivos->tamanioTotal);
	tablaArchivos->bloques = malloc(sizeof(Tbloques)*cantBloques);
	printf("cant bloques %d\n",cantBloques);

	printf("Tamanio %llu\n", tablaArchivos->tamanioTotal);
	printf("Extension %s\n", tablaArchivos->extensionArchivo);

	while(nroBloque != cantBloques){

		tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
		tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo = malloc(sizeof(char)*4);
		tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
		tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo = malloc(sizeof(char)*4);

		bloqueCopia0 = generarStringDeBloqueNCopiaN(nroBloque,0);
		bloqueCopia1 = generarStringDeBloqueNCopiaN(nroBloque,1);
		bloqueBytes = generarStringDeBloqueNBytes(nroBloque);

		temporal1 = config_get_array_value(archivo, bloqueCopia0);
		temporal2 = config_get_array_value(archivo, bloqueCopia1);
		tablaArchivos->bloques[nroBloque].bytes = config_get_int_value(archivo, bloqueBytes);

		strcpy(tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo,temporal1[0]);
		strcpy(tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo,temporal1[1]);

		strcpy(tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo,temporal2[0]);
		strcpy(tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo,temporal2[1]);

		printf("Nombre de nodo copia cero %s\n",tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo);
		printf("Numero de nodo copia cero %s\n",tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo);
		printf("Nombre de nodo copia uno %s\n",tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo);
		printf("Numero de nodo copia uno %s\n",tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo);
		printf("Bytes %llu\n",tablaArchivos->bloques[nroBloque].bytes);
		nroBloque++;

		liberarPunteroDePunterosAChar(temporal1);
		free(temporal1);
		liberarPunteroDePunterosAChar(temporal2);
		free(temporal2);

		free(bloqueCopia0);
		free(bloqueCopia1);
		free(bloqueBytes);
	}

	config_destroy(archivo);
}

t_bitarray* crearBitmap(int tamanioDatabin){
	int tamanioEnBits = ceil(tamanioDatabin/8.0);
	char * bitarray = calloc(tamanioEnBits,1);
	t_bitarray* bitmap = bitarray_create_with_mode(bitarray,tamanioEnBits,LSB_FIRST);
	return bitmap;
}

void levantarTablaNodos(Tnodos * tablaNodos){

	int i = 0;
	int j = 0;
	char* nodoTotal = malloc(sizeof(char)*12);
	char* nodoLibre = malloc(sizeof(char)*12);


	t_config * archivoNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	tablaNodos->cantBloquesTotal = config_get_int_value(archivoNodos,"TAMANIO");
	tablaNodos->cantLibresTotal = config_get_int_value(archivoNodos,"LIBRE");

	printf("Cantidad bloques totales %d\n", tablaNodos->cantBloquesTotal);
	printf("Cantidad bloques libres %d\n", tablaNodos->cantLibresTotal);

	tablaNodos->nodos = config_get_array_value(archivoNodos,"NODOS");
	printf("nombre nodo 1: %s\n", tablaNodos->nodos[0]);
	printf("nombre nodo 2: %s\n", tablaNodos->nodos[1]);
	while(tablaNodos->nodos[j] != NULL){
		j++;
	}

	tablaNodos->nodoBloques = malloc(sizeof(TnodoBloque)*j);

	while(tablaNodos->nodos[i] != NULL){

		sprintf(nodoTotal,"Nodo%dTotal",i+1);
		sprintf(nodoLibre,"Nodo%dLibre",i+1);

		tablaNodos->nodoBloques[i].cantBloques = config_get_int_value(archivoNodos,nodoTotal);
		tablaNodos->nodoBloques[i].cantLibres = config_get_int_value(archivoNodos,nodoLibre);

		i++;
		}



	/*
	config_set_value(tablaNodosConfig, "Nodo1Libre", "5");
	config_save(tablaNodosConfig); // escribe algo en el configs
*/	free(nodoTotal);
	free(nodoLibre);
	config_destroy(archivoNodos);
}

void levantarTablas(Tnodos * tablaNodos){
	levantarTablasDirectorios();
	levantarTablaNodos(tablaNodos);
	mostrarDirectorios(); //no hace nada, pero deberia
}

void mostrarBitmap(t_bitarray* bitmap){
	int i;
	puts("El bitmap es:");
	for(i=0;i < bitarray_get_max_bit(bitmap);i++){
		printf("%d",bitarray_test_bit(bitmap,i));
	}
	puts("");
}

void liberarTablaDeArchivo(Tarchivo * tablaDeArchivos){
	//deberia liberar todo pero hace malloc de algunas cosas... porque ???
	int i;
	int cantBloques = ceil(tablaDeArchivos->tamanioTotal/1048576.0);
	tablaDeArchivos->extensionArchivo = malloc(sizeof(Tarchivo));
	tablaDeArchivos->bloques = malloc(sizeof(Tbloques));

	for(i = 0; i != cantBloques; i++){
		free(tablaDeArchivos->bloques[i].copiaCero.nombreDeNodo);
		free(tablaDeArchivos->bloques[i].copiaCero.numeroBloqueDeNodo);
		free(tablaDeArchivos->bloques[i].copiaUno.nombreDeNodo);
		free(tablaDeArchivos->bloques[i].copiaUno.numeroBloqueDeNodo);
	}
}