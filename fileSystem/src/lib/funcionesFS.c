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

void mostrarConfiguracion(TfileSystem *fileSystem){

	printf("Puerto Entrada: %s\n",  fileSystem->puerto_entrada);
	printf("Puerto Datanode: %s\n",       fileSystem->puerto_datanode);
	printf("Puerto Yama: %s\n", fileSystem->puerto_yama);
	printf("IP Yama: %s\n", fileSystem->ip_yama);
	printf("Tipo de proceso: %d\n", fileSystem->tipo_de_proceso);
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
	Theader * head = malloc(sizeof(Theader));
	Tbuffer * buffer;
	int estado;
	head->tipo_de_proceso=FILESYSTEM;
	head->tipo_de_mensaje=ALMACENAR_BLOQUE;

	list_sort(listaDeNodos, ordenarSegunBloquesDisponibles);
	Tnodo* nodo1 = (Tnodo*)list_get(listaDeNodos, 0);
	Tnodo* nodo2 = (Tnodo*)list_get(listaDeNodos, 1);

	buffer = empaquetarBloque(head,bloque->numeroDeBloque,bloque->tamanio,bloque->contenido);

	printf("Numero de bloque %d , Tamanio de bloque %llu\n", bloque->numeroDeBloque,bloque->tamanio);
	printf("Tamanio del buffer que se va a enviar %llu \n", buffer->tamanio);
	 if ((estado = send(nodo1->fd, buffer->buffer , buffer->tamanio, 0)) == -1){
		 logAndExit("Fallo al enviar a Nodo el bloque a almacenar");
	 }

	 printf("Se envio bloque a Nodo1 %d bytes\n", estado);
	 if ((estado = send(nodo2->fd, buffer->buffer , buffer->tamanio, 0)) == -1){
		 logAndExit("Fallo al enviar a Nodo el bloque a almacenar");
	 }
	 printf("Se envio bloque a Nodo2 %d bytes\n",estado);

	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaCero.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	 strcpy(estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaCero.nombreDeNodo, nodo1->nombre);
	 printf("El nombre de nodo es %s\n", estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaCero.nombreDeNodo);

	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaCero.numeroBloqueDeNodo = nodo1->primerBloqueLibreBitmap;
	 ocuparProximoBloqueBitmap(nodo1);
	 mostrarBitmap(nodo1->bitmap);

	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaUno.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	 strcpy(estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaUno.nombreDeNodo, nodo2->nombre);
	 printf("El nombre de nodo es %s\n", estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaUno.nombreDeNodo);

	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].copiaUno.numeroBloqueDeNodo = nodo2->primerBloqueLibreBitmap;
	 ocuparProximoBloqueBitmap(nodo2);
	 mostrarBitmap(nodo2->bitmap);

	 estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].bytes = bloque->tamanio;
	 printf("El tamaño del bloque en bytes es: %llu", estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque].bytes);

	 liberarEstructuraBuffer(buffer);

}

void liberarEstructuraBuffer(Tbuffer * buffer){
	free(buffer->buffer);
	free(buffer);
}

int existeDirectorio(char * directorio){

	char ** carpetas = string_split(directorio, "/");
	int i = 0;
	int indicePadre = 0;

	while(carpetas[i] != NULL){
		Tdirectorio * estructuraDirectorio = (Tdirectorio*)buscarPorNombreDeDirectorio(carpetas[i]);
		if(estructuraDirectorio != NULL){
			if(estructuraDirectorio->padre == indicePadre){
				indicePadre = estructuraDirectorio->index;
				i++;
			}else{
				return 0;
			}
		}
		else{
			return 0;
		}
	}

	return 1;

}

void* buscarPorNombreDeDirectorio(char * directorio){
	bool buscarPorNombreDeDirectorioParaLista(void* elementoDeLista){
		Tdirectorio* estructuraDirectorio = (Tdirectorio*) elementoDeLista;

		return !strcmp(directorio, estructuraDirectorio->nombre);
	}

	return list_find(listaTablaDirectorios, buscarPorNombreDeDirectorioParaLista);
}

int buscarIndexPorNombreDeDirectorio(char * directorio){
	Tdirectorio * estructuraDirectorio = (Tdirectorio *)buscarPorNombreDeDirectorio(directorio);
	puts("rompi todo");
	if(estructuraDirectorio != NULL){
		return estructuraDirectorio->index;
	}
	return -1;

}

int obtenerIndexDeUnaRuta(char * rutaDestino){
	puts("obteniendo index de una ruta");
	char ** palabras = string_split(rutaDestino, "/");
	char * directorio = obtenerUltimoElementoDeUnSplit(palabras);
	puts("index obtenido");
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

char * generarArrayParaArchivoConfig(char * dato1, char * dato2){
	char * concatenacionLoca = malloc(1 + TAMANIO_NOMBRE_NODO + 1 + 3);
	string_append(&concatenacionLoca, "[");
	string_append(&concatenacionLoca, dato1);
	string_append(&concatenacionLoca, ",");
	string_append(&concatenacionLoca, dato2);
	string_append(&concatenacionLoca, "]");
	return concatenacionLoca;
}

void almacenarEstructuraArchivoEnUnArchivo(Tarchivo * archivoAAlmacenar, char * rutaArchivo){
	t_config * archivoConfig = config_create(rutaArchivo);
	int cantidadDeBloques = cantidadDeBloquesDeUnArchivo(archivoAAlmacenar->tamanioTotal);
	char * bloque0CopiaN;
	char * bloque1CopiaN;
	char * bloqueNBytes;
	char * concatenacionLoca;
	char * concatenacionLoca2;
	char * numeroBloqueEnString;
	char * numeroBloqueEnString2;
	int i = 0;

	config_set_value(archivoConfig, "TAMANIO", string_itoa(archivoAAlmacenar->tamanioTotal));
	config_set_value(archivoConfig, "TIPO", archivoAAlmacenar->extensionArchivo);

	while(cantidadDeBloques != 0){

		numeroBloqueEnString = malloc(3);
		numeroBloqueEnString = malloc(3);

		bloque0CopiaN = generarStringDeBloqueNCopiaN(i,0);
		numeroBloqueEnString = string_itoa(archivoAAlmacenar->bloques[i].copiaCero.numeroBloqueDeNodo);
		concatenacionLoca = generarArrayParaArchivoConfig(archivoAAlmacenar->bloques[i].copiaCero.nombreDeNodo, numeroBloqueEnString);

		config_set_value(archivoConfig, bloque0CopiaN, concatenacionLoca);

		bloque1CopiaN = generarStringDeBloqueNCopiaN(i,1);
		numeroBloqueEnString2 = string_itoa(archivoAAlmacenar->bloques[i].copiaUno.numeroBloqueDeNodo);
		concatenacionLoca2 = generarArrayParaArchivoConfig(archivoAAlmacenar->bloques[i].copiaUno.nombreDeNodo, numeroBloqueEnString2);
		config_set_value(archivoConfig, bloque1CopiaN, concatenacionLoca2);

		//aca hace el tamanio
		bloqueNBytes = generarStringDeBloqueNBytes(i);
		//hay que pasar un long a string
		config_set_value(archivoConfig, bloqueNBytes, string_itoa(archivoAAlmacenar->bloques[i].bytes));
		cantidadDeBloques--;
		i++;

		free(numeroBloqueEnString);
		free(numeroBloqueEnString2);
		free(bloque0CopiaN);
		free(bloque1CopiaN);
		free(bloqueNBytes);
		free(concatenacionLoca);
		free(concatenacionLoca2);
	}

	puts("guardando TODO");
	config_save(archivoConfig);
	puts("guardado.");
	config_destroy(archivoConfig);

}

void guardarTablaDeArchivo(Tarchivo * archivoAAlmacenar, char * rutaDestino){
	int index = obtenerIndexDeUnaRuta(rutaDestino);
	char * rutaArchivo = malloc(200);
	sprintf(rutaArchivo, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/%d/%s.%s", index, archivoAAlmacenar->nombreArchivoSinExtension, archivoAAlmacenar->extensionArchivo);
	//hay que verificar si existe el directorio
	FILE * archivo = fopen(rutaArchivo, "w+");
	fclose(archivo);
	almacenarEstructuraArchivoEnUnArchivo(archivoAAlmacenar, rutaArchivo);

}

void liberarEstructuraBloquesAEnviar(TbloqueAEnviar * infoBloque){
	free(infoBloque->contenido);
	free(infoBloque);
}

void procesarArchivoBinario(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar * infoBloque){
	int cantidadDeBloquesDelArchivo = cantidadDeBloquesDeUnArchivo(archivoAAlmacenar->tamanioTotal);
	unsigned long long bytesFaltantesPorEnviar = archivoAAlmacenar->tamanioTotal;
	char * punteroAuxiliar = archivoMapeado;
	infoBloque->tamanio = BLOQUE_SIZE;

	while(cantidadDeBloquesDelArchivo != 1){
		memcpy(infoBloque->contenido, punteroAuxiliar, BLOQUE_SIZE);
		enviarBloque(infoBloque, archivoAAlmacenar);
		infoBloque->numeroDeBloque++;
		cantidadDeBloquesDelArchivo--;
		punteroAuxiliar += BLOQUE_SIZE;
		bytesFaltantesPorEnviar -= BLOQUE_SIZE;
	}

	//el ultimo bloque tiene tamaño <= que BLOQUE_SIZE (1024*1024)
	infoBloque->tamanio = bytesFaltantesPorEnviar;
	memcpy(infoBloque->contenido, punteroAuxiliar, bytesFaltantesPorEnviar);
	enviarBloque(infoBloque, archivoAAlmacenar);



}

void procesarArchivoCsv(Tarchivo * archivoAAlmacenar, char * archivoMapeado){
	//aca laburas vos torri
}

void procesarArchivoSegunExtension(Tarchivo * archivoAAlmacenar, char * archivoMapeado){
	TbloqueAEnviar * infoBloque = malloc(sizeof(TbloqueAEnviar));
	infoBloque->numeroDeBloque = 0;
	infoBloque->contenido = malloc(BLOQUE_SIZE);

	if(strcmp(archivoAAlmacenar->extensionArchivo, "bin") == 0){
		procesarArchivoBinario(archivoAAlmacenar, archivoMapeado, infoBloque);
	}
	else{
		procesarArchivoCsv(archivoAAlmacenar, archivoMapeado);
	}

	liberarEstructuraBloquesAEnviar(infoBloque);
}

void almacenarArchivo(char **palabras){
	//palabras[1] --> ruta archivo a almacenar
	//palabras[2] --> ruta de nuestro directorio
	char * archivoMapeado;
	char ** splitDeRuta = string_split(palabras[1], "/");
	char * nombreArchivoConExtension = obtenerUltimoElementoDeUnSplit(splitDeRuta);
	printf("El archivo a guardar es: %s\n", nombreArchivoConExtension);

	Tarchivo * archivoAAlmacenar = malloc(sizeof(Tarchivo));
	archivoAAlmacenar->nombreArchivoSinExtension = obtenerNombreDeArchivoSinExtension(nombreArchivoConExtension);
	puts("obtuvo el nombre del archivo sin extension");
	archivoAAlmacenar->extensionArchivo = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);
	printf("El nombre del archivo es: %s\n", archivoAAlmacenar->nombreArchivoSinExtension);
	printf("La extensión es es: %s\n", archivoAAlmacenar->extensionArchivo);

	//esto va adentro de la funcion que voy a llamar
	//unsigned long long bytesDisponiblesEnBloque = BLOQUE_SIZE;

	//esto tiene que ir adentro de la funcion que voy a llamar
	//para no tener que pasarselo a procesarSegunExtension
	/*TbloqueAEnviar * infoBloque = malloc(sizeof(TbloqueAEnviar));
	infoBloque->numeroDeBloque = 0;
	infoBloque->contenido = malloc(BLOQUE_SIZE);*/

	FILE * archivoOrigen = fopen(palabras[1], "r");
	int fd = fileno(archivoOrigen);

	unsigned long long tamanio = tamanioArchivo(archivoOrigen);
	archivoAAlmacenar->tamanioTotal = tamanio;

	printf("El tamaño del archivo es: %llu\n", tamanio);


	if ((archivoMapeado = mmap(NULL, tamanio, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		logAndExit("Error al hacer mmap");
	}

	fclose(archivoOrigen);

	archivoAAlmacenar->bloques = malloc(sizeof(Tbloques) * cantidadDeBloquesDeUnArchivo(tamanio));

	printf("La cantidad de bloquees es: %d \n", cantidadDeBloquesDeUnArchivo(tamanio));

	if(tamanio == 0){
		puts("Error al almacenar archivo, está vacío");
		return;
	}
	else {

		procesarArchivoSegunExtension(archivoAAlmacenar, archivoMapeado);

		/*puts("El archivo no es vacio");
		lineas = string_split(archivoMapeado,"\n");
		puts("Pude splitear el archivo mapeado.");
		int i = 0;
		printf("%c = caracter final\n",archivoMapeado[tamanio-1]);*/

		/*while(lineas[i]!=NULL){
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

		}*/
		guardarTablaDeArchivo(archivoAAlmacenar, palabras[2]);

		close(fd);

		liberarPunteroDePunterosAChar(splitDeRuta);
		free(splitDeRuta);
		free(nombreArchivoConExtension);
		liberarTablaDeArchivo(archivoAAlmacenar);
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
				// cuando se crea un directorio, hay que comprobar
				// que no supere los 100 elementos de la lista
				printf("ya pude crear el directorio\n");
			}
		}
		else{
			puts("Error en la cantidad de parametros");
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
			if (cantidad ==1){
				//getMD5(palabras);
				printf("ya pude solicitar el md5 de un archivo del file system\n");
			}
			else {
				puts("Error en la cantidad de parametros");
			}

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

void* buscarNodoPorFD(int fd){
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd == fd;
	}

	return list_find(listaDeNodos, buscarPorFDParaLista);
}

void borrarNodoDesconectadoPorFD(int fd){
	Tnodo* nodoABorrar = (Tnodo*)buscarNodoPorFD(fd);
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd==fd;
	}
	list_remove_by_condition(listaDeNodosDesconectados,buscarPorFDParaLista);
}

void borrarNodoPorFD(int fd){
	Tnodo* nodoABorrar = (Tnodo*)buscarNodoPorFD(fd);
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd==fd;
	}
	list_remove_by_condition(listaDeNodos,buscarPorFDParaLista);
}

Tnodo * inicializarNodo(TpackInfoBloqueDN * infoBloqueRecibido, int fileDescriptor){
	Tnodo * nuevoNodo = malloc(sizeof(Tnodo));
	nuevoNodo->fd = fileDescriptor;
	nuevoNodo->cantidadBloquesTotal = infoBloqueRecibido->databinEnMB;
	nuevoNodo->cantidadBloquesLibres = infoBloqueRecibido->databinEnMB;
	nuevoNodo->primerBloqueLibreBitmap = 0;
	nuevoNodo->nombre = strdup(infoBloqueRecibido->nombreNodo);
	nuevoNodo->bitmap = crearBitmap(infoBloqueRecibido->databinEnMB);
	return nuevoNodo;
}

TpackInfoBloqueDN * recvInfoNodo(int socketFS){
	int estado;
	TpackInfoBloqueDN * infoBloque = malloc(sizeof(TpackInfoBloqueDN));
	char * nombreNodo;
	char * ipNodo;
	char * puertoNodo;

	//Recibo el tamaño del nombre del nodo
	if ((estado = recv(socketFS, &infoBloque->tamanioNombre, sizeof(int), 0)) == -1) {
		logAndExit("Error al recibir el tamanio del nombre del nodo");
		}
	printf("Para el tamaño del nombre recibi %d bytes\n", estado);
	nombreNodo = malloc(infoBloque->tamanioNombre);

	//Recibo el nombre del nodo
	if ((estado = recv(socketFS, nombreNodo, infoBloque->tamanioNombre, 0)) == -1) {
		logAndExit("Error al recibir el nombre del nodo");
		}

	printf("Para el nombre del nodo recibi %d bytes\n", estado);

	//Recibo el tamanio de la ip del nodo
	if ((estado = recv(socketFS, &infoBloque->tamanioIp, sizeof(int), 0)) == -1) {
		logAndExit("Error al recibir el tamanio del ip del nodo");
		}
	printf("Para el tamaño de la ip recibi %d bytes\n", estado);

	ipNodo = malloc(infoBloque->tamanioIp);

	//Recibo la ip del nodo
	if ((estado = recv(socketFS, ipNodo, infoBloque->tamanioIp, 0)) == -1) {
		logAndExit("Error al recibir el ip del nodo");
		}

	printf("Para el la ip recibi %d bytes\n", estado);

	//Recibo el tamanio del puerto del nodo
	if ((estado = recv(socketFS, &infoBloque->tamanioPuerto, sizeof(int), 0)) == -1) {
		logAndExit("Error al recibir el tamanio del puerto del nodo");
		}
	printf("Para el tamaño del puerto recibi %d bytes\n", estado);

	puertoNodo = malloc(infoBloque->tamanioPuerto);

	//Recibo el puerto del nodo
	if ((estado = recv(socketFS, puertoNodo, infoBloque->tamanioPuerto, 0)) == -1) {
		logAndExit("Error al recibir el puerto del nodo");
		}

	//Recibo el databin en MB
	if ((estado = recv(socketFS, &infoBloque->databinEnMB, sizeof(int), 0)) == -1) {
		logAndExit("Error al recibir el tamanio del databin");
		}

	printf("Para el tamanio del databin recibi %d bytes\n", estado);

	 infoBloque = desempaquetarInfoNodo(infoBloque, nombreNodo, ipNodo, puertoNodo);
	 puts("desempaqueta la info del nodo");
	 return infoBloque;
}

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
							list_add(listaDeNodosDesconectados, buscarNodoPorFD(fileDescriptor));
							borrarNodoPorFD(fileDescriptor);
							sprintf(mensaje, "Se desconecto el cliente de fd: %d.", fileDescriptor);
							log_trace(logger, mensaje);
							clearAndClose(fileDescriptor, &masterFD);
						}
					if(head->tipo_de_proceso==DATANODE){
						switch(head->tipo_de_mensaje){
							case INFO_NODO:
								puts("Es datanode y quiere mandar la información del nodo");
								//caro, tenes que traer el tamanio del databin
								infoBloque = recvInfoNodo(fileDescriptor);
								if((Tnodo*)buscarNodoPorFD(fileDescriptor) == NULL){
									printf("Para el nro de bloque recibi %d bytes\n", estado);
									nuevoNodo = inicializarNodo(infoBloque, fileDescriptor);
									list_add(listaDeNodos, nuevoNodo);
									puts("Nodo inicializado y guardado en la lista");
									mostrarBitmap(nuevoNodo->bitmap);
								}
								else{
									list_add(listaDeNodos, buscarNodoPorFD(fileDescriptor));
									borrarNodoDesconectadoPorFD(fileDescriptor);
								}
								cantNodosPorConectar--;
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
					printf("se quiso conectar el proceso: %d\n",head->tipo_de_proceso);
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

	while(!feof(archivoDirectorios)){
		Tdirectorio * directorio = malloc(sizeof(Tdirectorio));
		fscanf(archivoDirectorios, "%d %s %d", &directorio->index, directorio->nombre, &directorio->padre);
		printf("%d \t %s \t %d \n", directorio->index, directorio->nombre, directorio->padre);
		list_add(listaTablaDirectorios, directorio);
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
		//tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo = malloc(sizeof(char)*4);
		tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
		//tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo = malloc(sizeof(char)*4);

		bloqueCopia0 = generarStringDeBloqueNCopiaN(nroBloque,0);
		bloqueCopia1 = generarStringDeBloqueNCopiaN(nroBloque,1);
		bloqueBytes = generarStringDeBloqueNBytes(nroBloque);

		temporal1 = config_get_array_value(archivo, bloqueCopia0);
		temporal2 = config_get_array_value(archivo, bloqueCopia1);
		tablaArchivos->bloques[nroBloque].bytes = config_get_int_value(archivo, bloqueBytes);

		strcpy(tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo,temporal1[0]);
		tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo = atoi(temporal1[1]);

		strcpy(tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo,temporal2[0]);
		tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo = atoi(temporal2[1]);

		printf("Nombre de nodo copia cero %s\n",tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo);
		printf("Numero de nodo copia cero %d\n",tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo);
		printf("Nombre de nodo copia uno %s\n",tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo);
		printf("Numero de nodo copia uno %d\n",tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo);
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

	free(nodoTotal);
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

	int cantBloques = cantidadDeBloquesDeUnArchivo(tablaDeArchivos->tamanioTotal)-1;
	free(tablaDeArchivos->extensionArchivo);
	free(tablaDeArchivos->nombreArchivoSinExtension);

	for(; cantBloques >= 0; cantBloques--){
		free(tablaDeArchivos->bloques[cantBloques].copiaCero.nombreDeNodo);
		free(tablaDeArchivos->bloques[cantBloques].copiaUno.nombreDeNodo);
	}
	free(tablaDeArchivos->bloques);
	free(tablaDeArchivos);
}

void crearDirectorio(char * ruta){
	mkdir(ruta, 0700);
}

int getMD5(char**palabras){
	/*Tdirectorio * directorio;
	char ** splitDeRuta = string_split(palabras[1], "/");
	char * nombreArchivoConExtension = obtenerUltimoElementoDeUnSplit(splitDeRuta);
	printf("El archivo del que queremos su MD5 es: %s\n", nombreArchivoConExtension);

	Tarchivo * archivo = malloc(sizeof(Tarchivo));
	archivo->nombreArchivoSinExtension = obtenerNombreDeArchivoSinExtension(nombreArchivoConExtension);
	archivo->extensionArchivo = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);

	char* ruta_temporal = string_duplicate("/tmp/");
		string_append(&ruta_temporal, archivo->nombreArchivoSinExtension);
		string_append(&ruta_temporal, ".");
		string_append(&ruta_temporal, archivo->extensionArchivo);
		copiar_archivo_mdfs_a_local(palabras[1], ruta_temporal); //FALTA ESTO

		char* comando = string_duplicate("md5sum ");
		string_append(&comando, ruta_temporal);
		system(comando);

		free(comando);
		free(ruta_temporal);*/
		return 0;
}
