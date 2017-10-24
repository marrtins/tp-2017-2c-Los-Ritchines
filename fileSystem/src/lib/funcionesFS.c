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

void almacenarBloquesEnEstructuraArchivo(Tarchivo * estructuraArchivoAAlmacenar, Tnodo * nodo1, Tnodo * nodo2, TbloqueAEnviar * bloque){
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
		double bloquesOcupados = nodo->cantidadBloquesTotal - nodo->cantidadBloquesLibres;
		return bloquesOcupados / nodo->cantidadBloquesTotal;
	}
	return obtenerProporcionDeDisponibilidad(nodoA) < obtenerProporcionDeDisponibilidad(nodoB);
}

void ocuparProximoBloqueBitmap(Tnodo * nodo){
	bitarray_set_bit(nodo->bitmap, nodo->primerBloqueLibreBitmap);
	nodo->primerBloqueLibreBitmap++;
	nodo->cantidadBloquesLibres--;
}

void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar){
	Theader * head = malloc(sizeof(Theader));
	Tbuffer * buffer1;
	Tbuffer * buffer2;
	int estado;
	head->tipo_de_proceso=FILESYSTEM;
	head->tipo_de_mensaje=ALMACENAR_BLOQUE;

	list_sort(listaDeNodos, ordenarSegunBloquesDisponibles);
	Tnodo* nodo1 = (Tnodo*)list_get(listaDeNodos, 0);
	Tnodo* nodo2 = (Tnodo*)list_get(listaDeNodos, 1);
	//Tnodo* nodo3 = (Tnodo*)list_get(listaDeNodos, 2);
	//Tnodo* nodo4 = (Tnodo*)list_get(listaDeNodos, 3);

	buffer1 = empaquetarBloque(head,bloque,nodo1);

	printf("Numero de bloque %d , Tamanio de bloque %llu\n", bloque->numeroDeBloque,bloque->tamanio);
	printf("Tamanio del buffer que se va a enviar %llu \n", buffer1->tamanio);
	 if ((estado = send(nodo1->fd, buffer1->buffer , buffer1->tamanio, 0)) == -1){
		 logAndExit("Fallo al enviar a Nodo el bloque a almacenar");
	 }

	buffer2 = empaquetarBloque(head,bloque,nodo2);
	printf("Se envio bloque a Nodo1 %d bytes\n", estado);
	 if ((estado = send(nodo2->fd, buffer2->buffer , buffer2->tamanio, 0)) == -1){
		 logAndExit("Fallo al enviar a Nodo el bloque a almacenar");
	 }
		double obtenerProporcionDeDisponibilidad(Tnodo* nodo){
			if(nodo->cantidadBloquesLibres == 0) return 1;
			double bloquesOcupados = nodo->cantidadBloquesTotal - nodo->cantidadBloquesLibres;
			return bloquesOcupados / nodo->cantidadBloquesTotal;
		}
		/*
	double p1 = obtenerProporcionDeDisponibilidad(nodo1);
	double p2 = obtenerProporcionDeDisponibilidad(nodo2);
	double p3 = obtenerProporcionDeDisponibilidad(nodo3);
	double p4 = obtenerProporcionDeDisponibilidad(nodo4);
	 FILE * archivoDeSeguimiento = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/envioBloques.txt","a");
	 fseek(archivoDeSeguimiento,0,SEEK_END);
	 fprintf(archivoDeSeguimiento, "%s %d \n","Bloque nro: ", bloque->numeroDeBloque);
	 fprintf(archivoDeSeguimiento, "%s\n%f\n%f\n%f\n%f\n", "proporciones: ",p1,p2,p3,p4);
	 fprintf(archivoDeSeguimiento, "%s (%d - %d) / %d\n","operacion:",nodo1->cantidadBloquesTotal,nodo1->cantidadBloquesLibres,nodo1->cantidadBloquesTotal);
	 fprintf(archivoDeSeguimiento, "%s (%d - %d) / %d\n","operacion:",nodo2->cantidadBloquesTotal,nodo2->cantidadBloquesLibres,nodo2->cantidadBloquesTotal);
	 fprintf(archivoDeSeguimiento, "%s (%d - %d) / %d\n","operacion:",nodo3->cantidadBloquesTotal,nodo3->cantidadBloquesLibres,nodo3->cantidadBloquesTotal);
	 fprintf(archivoDeSeguimiento, "%s (%d - %d) / %d\n","operacion:",nodo4->cantidadBloquesTotal,nodo4->cantidadBloquesLibres,nodo4->cantidadBloquesTotal);
	 fwrite(nodo1->nombre, strlen(nodo1->nombre), 1, archivoDeSeguimiento);
	 fputs("\n",archivoDeSeguimiento);
	 fwrite(nodo2->nombre, strlen(nodo2->nombre), 1, archivoDeSeguimiento);
	 fputs("\n",archivoDeSeguimiento);
	*/ printf("Se envio bloque a Nodo2 %d bytes\n",estado);

	 almacenarBloquesEnEstructuraArchivo(estructuraArchivoAAlmacenar, nodo1, nodo2, bloque);

	 liberarEstructuraBuffer(buffer1);
	 liberarEstructuraBuffer(buffer2);
}


void liberarEstructuraBuffer(Tbuffer * buffer){
	free(buffer->buffer);
	free(buffer);
}

int existeDirectorio(char * directorio){

	char ** carpetas = string_split(directorio, "/");
	char * yamafs = malloc(10);
	int i = 0;
	int indicePadre = 0;
	strcpy(yamafs,"yamafs:");
	if (string_equals_ignore_case(carpetas[i], yamafs)) {
		i++;
		while (carpetas[i] != NULL) {
			Tdirectorio * estructuraDirectorio = (Tdirectorio*) buscarPorNombreDeDirectorio(carpetas[i]);
			if (estructuraDirectorio != NULL) {
				if (estructuraDirectorio->padre == indicePadre) {
					indicePadre = estructuraDirectorio->index;
					i++;
				} else {
					return 0;
				}
			} else {
				return 0;
			}
		}

		return 1;
	} else {
		puts("Para referirse al filesystem colocar 'yamafs:' al principio de la ruta");
		return 0;
	}

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

		numeroBloqueEnString = malloc(4);
		numeroBloqueEnString = malloc(4);

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
	free(rutaArchivo);

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

void procesarArchivoCsv(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar* infoBloque){
	char * punteroAuxiliar = archivoMapeado;
	unsigned long long bytesFaltantesPorEnviar = archivoAAlmacenar->tamanioTotal;
	unsigned long long posicionUltimoBarraN = 0;
	unsigned long long bytesCopiados = 0;


	while(bytesFaltantesPorEnviar > 0){
		if(bytesFaltantesPorEnviar < BLOQUE_SIZE){
			memcpy(infoBloque->contenido,punteroAuxiliar,bytesFaltantesPorEnviar);
			infoBloque->tamanio = bytesFaltantesPorEnviar;
			bytesFaltantesPorEnviar = 0;
		}
		else {
			posicionUltimoBarraN = posicionUltimoBarraN + BLOQUE_SIZE;
			while(archivoMapeado[posicionUltimoBarraN] != '\n'){
				posicionUltimoBarraN--;
			}
			memcpy(infoBloque->contenido,punteroAuxiliar,posicionUltimoBarraN);
			infoBloque->tamanio = posicionUltimoBarraN - bytesCopiados;
			bytesCopiados = posicionUltimoBarraN;
		}
		bytesFaltantesPorEnviar-=bytesCopiados;
		//freir infoBloque->contenido en enviarBloque;
		//los tamaños varian según la posición del \n;
		enviarBloque(infoBloque, archivoAAlmacenar);
		punteroAuxiliar = archivoMapeado + posicionUltimoBarraN;
		infoBloque->numeroDeBloque++;
	}
}

int sumarListasPorTamanioDatabin(){
	int cantidadDeElementos = listaDeNodos->elements_count;
	int tamanioTotalDisponible = 0;
	int i = 0;
	Tnodo * nodo;
	while(cantidadDeElementos != 0){
		nodo = list_get(listaDeNodos, i);
		tamanioTotalDisponible += nodo->cantidadBloquesLibres;
		i++;
		cantidadDeElementos--;
	}
	printf("La cantidad de MB disponibles en los datanodes es: %d", tamanioTotalDisponible);
	return tamanioTotalDisponible;

}

int verificarDisponibilidadDeEspacioEnNodos(unsigned long long tamanioDelArchivoAGuardar){
	int tamanioEnMBDisponiblesEnNodos = sumarListasPorTamanioDatabin();
	//se multiplica por 2 por que se guarda 1 copia en otro nodo
	if(tamanioEnMBDisponiblesEnNodos * BLOQUE_SIZE < tamanioDelArchivoAGuardar * 2){
		return -1;
	}
	return 0;
}

int procesarArchivoSegunExtension(Tarchivo * archivoAAlmacenar, char * nombreArchivo){
	FILE * archivoOrigen = fopen(nombreArchivo, "r");
	int fd = fileno(archivoOrigen);
	char * archivoMapeado;
	unsigned long long tamanio = tamanioArchivo(archivoOrigen);
	archivoAAlmacenar->tamanioTotal = tamanio;
	TbloqueAEnviar * infoBloque = malloc(sizeof(TbloqueAEnviar));
	infoBloque->numeroDeBloque = 0;
	infoBloque->contenido = malloc(BLOQUE_SIZE);

	printf("El tamaño del archivo es: %llu\n", tamanio);

	if ((archivoMapeado = mmap(NULL, tamanio, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		logAndExit("Error al hacer mmap");
	}

	fclose(archivoOrigen);
	close(fd);

	archivoAAlmacenar->bloques = malloc(sizeof(Tbloques) * cantidadDeBloquesDeUnArchivo(tamanio));

	printf("La cantidad de bloquees es: %d \n", cantidadDeBloquesDeUnArchivo(tamanio));

	if(tamanio == 0){
		puts("Error al almacenar archivo, está vacío");
		return -1;
	}

	if(verificarDisponibilidadDeEspacioEnNodos(tamanio) == -1){
		puts("No hay suficiente espacio en los datanodes, intente con un archivo más chico");
		return -1;
	}

	if(strcmp(archivoAAlmacenar->extensionArchivo, "csv") == 0){
		procesarArchivoCsv(archivoAAlmacenar, archivoMapeado, infoBloque);
	}
	else{
		procesarArchivoBinario(archivoAAlmacenar, archivoMapeado, infoBloque);
	}

	liberarEstructuraBloquesAEnviar(infoBloque);
	return 0;
}

void almacenarArchivo(char **palabras){
	//palabras[1] --> ruta archivo a almacenar
	//palabras[2] --> ruta de nuestro directorio
	char ** splitDeRuta = string_split(palabras[1], "/");
	char * nombreArchivoConExtension = obtenerUltimoElementoDeUnSplit(splitDeRuta);
	printf("El archivo a guardar es: %s\n", nombreArchivoConExtension);

	Tarchivo * archivoAAlmacenar = malloc(sizeof(Tarchivo));
	archivoAAlmacenar->nombreArchivoSinExtension = obtenerNombreDeArchivoSinExtension(nombreArchivoConExtension);
	puts("obtuvo el nombre del archivo sin extension");
	archivoAAlmacenar->extensionArchivo = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);
	printf("El nombre del archivo es: %s\n", archivoAAlmacenar->nombreArchivoSinExtension);
	printf("La extensión es es: %s\n", archivoAAlmacenar->extensionArchivo);

	if(procesarArchivoSegunExtension(archivoAAlmacenar, palabras[1]) == -1){
		return;
	}

	guardarTablaDeArchivo(archivoAAlmacenar, palabras[2]);
	liberarPunteroDePunterosAChar(splitDeRuta);
	free(splitDeRuta);
	liberarTablaDeArchivo(archivoAAlmacenar);
}
void persistirTablaDeDirectorios(){


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
				if(crearDirectorio(palabras[1])<0){
					puts("El directorio no se pudo crear");
				}else{
				// cuando se crea un directorio, hay que comprobar
				// que no supere los 100 elementos de la lista
				printf("ya pude crear el directorio\n");
				}
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
	persistirTablaDeDirectorios();
}

void consolaFS(){
	puts("Bienvenido a la consola. Ingrese un comando:");
	while(1){
		char *linea = readline(">");
		add_history(linea);
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
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd==fd;
	}
	list_remove_by_condition(listaDeNodosDesconectados,buscarPorFDParaLista);
}

void borrarNodoPorFD(int fd){
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
	puts("\n\n\nACA SALE EL NOMBRE DEL DATANODE QUE SE CONECTO\n\n\n");
	puts(infoBloqueRecibido->nombreNodo);
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

char * agregarNodoAArrayDeNodos(char ** nodos, char * nombreNodo){
	puts("Estoy adentro de la funcion que appendea");
	char * nuevoString = string_new();
	int i = 0;
	string_append(&nuevoString, "[");
	puts("Voy a hacer el while");
	while(nodos[i] != NULL){
		string_append(&nuevoString, nodos[i]);
		string_append(&nuevoString, ",");
		i++;
	}
	string_append(&nuevoString, nombreNodo);
	string_append(&nuevoString, "]");
	puts("Por salir");
	return nuevoString;
}

void inicializarTablaDeNodos(){
	puts("antes de borrar");
	remove("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");
	puts("despues de borrar");
	FILE * archivo = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin", "wb");
	fclose(archivo);
	t_config * archivoNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");
	config_set_value(archivoNodos, "TAMANIO", "0");
	config_set_value(archivoNodos, "LIBRE","0");
	config_set_value(archivoNodos, "NODOS", "[]");
	config_save(archivoNodos);
	config_destroy(archivoNodos);
}

void agregarNodoATablaDeNodos(Tnodo * nuevoNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	puts("TAMANIO");
	//TAMANIO
	int tamanio = config_get_int_value(tablaDeNodos, "TAMANIO");
	tamanio += nuevoNodo->cantidadBloquesTotal;
	char * tamanioString = string_itoa(tamanio);
	config_set_value(tablaDeNodos, "TAMANIO", tamanioString);

	puts("LIBRE");
	//LIBRE
	int libre = config_get_int_value(tablaDeNodos, "LIBRE");
	libre += nuevoNodo->cantidadBloquesLibres;
	char * libreString = string_itoa(libre);
	config_set_value(tablaDeNodos, "LIBRE", libreString);

	puts("NODOS");
	//NODOS
	char ** nodos = config_get_array_value(tablaDeNodos, "NODOS");
	char * nodosConNodoAgregado = agregarNodoAArrayDeNodos(nodos, nuevoNodo->nombre);
	puts(nodosConNodoAgregado);
	puts("cargando Nodos");
	config_set_value(tablaDeNodos, "NODOS", nodosConNodoAgregado);

	puts("NODONTOTAL");
	//agregar Nodos Dinamicamente
	char * nodoTotalAString = string_new();
	string_append_with_format(&nodoTotalAString,"%sTotal", nuevoNodo->nombre);
	char * bloquesTotalString = string_itoa(nuevoNodo->cantidadBloquesTotal);
	config_set_value(tablaDeNodos, nodoTotalAString, bloquesTotalString);

	puts("NODONLIBRE");
	char * nodoLibreAString = string_new();
	string_append_with_format(&nodoLibreAString,"%sLibre", nuevoNodo->nombre);
	char * bloquesLibresString = string_itoa(nuevoNodo->cantidadBloquesLibres);
	config_set_value(tablaDeNodos, nodoLibreAString, bloquesLibresString);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	free(nodoTotalAString);
	free(nodoLibreAString);
	free(libreString);
	free(tamanioString);
	liberarPunteroDePunterosAChar(nodos);
	free(nodos);
	free(bloquesLibresString);
	free(bloquesTotalString);
	free(nodosConNodoAgregado);
}

char * eliminarNodoDelArrayDeNodos(char ** nodos, char * nombre){
	char * nuevoString = string_new();
	int i = 0;
	string_append(&nuevoString, "[");
	while(nodos[i] != NULL){
		if(strcmp(nodos[i], nombre)){
			string_append(&nuevoString, nodos[i]);
			string_append(&nuevoString, ",");
		}
		i++;
	}
	string_append(&nuevoString, "]");
	return nuevoString;
}

void eliminarNodoDeTablaDeNodos(Tnodo * nuevoNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//NODONTOTAL
	char * nodoTotalAString = string_new();
	string_append_with_format(&nodoTotalAString,"%sTotal", nuevoNodo->nombre);
	int nodoTotal = config_get_int_value(tablaDeNodos, nodoTotalAString);

	//TAMANIO
	int tamanio = config_get_int_value(tablaDeNodos, "TAMANIO");
	tamanio -= nodoTotal;
	char * tamanioString = string_itoa(tamanio);
	config_set_value(tablaDeNodos, "TAMANIO", tamanioString);

	//NODONLIBRE
	char * nodoLibreAString = string_new();
	string_append_with_format(&nodoLibreAString,"%sLibre", nuevoNodo->nombre);
	int nodoLibre = config_get_int_value(tablaDeNodos, nodoLibreAString);

	//LIBRE
	int libre = config_get_int_value(tablaDeNodos, "LIBRE");
	libre -= nodoLibre;
	char * libreString = string_itoa(libre);
	config_set_value(tablaDeNodos, "LIBRE", libreString);

	//SETEAR NODONTOTAL
	config_set_value(tablaDeNodos, nodoTotalAString, "");

	//SETEAR NODONLIBRE
	config_set_value(tablaDeNodos, nodoLibreAString, "");

	//NODOS
	char ** nodos = config_get_array_value(tablaDeNodos, "NODOS");
	char * nodosConNodoEliminado = eliminarNodoDelArrayDeNodos(nodos, nuevoNodo->nombre);
	config_set_value(tablaDeNodos, "NODOS", nodosConNodoEliminado);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	free(nodoTotalAString);
	free(nodoLibreAString);
	free(libreString);
	free(tamanioString);
	liberarPunteroDePunterosAChar(nodos);
	free(nodos);
	free(nodosConNodoEliminado);
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
							eliminarNodoDeTablaDeNodos(nuevoNodo);
							sprintf(mensaje, "Se desconecto el cliente de fd: %d.", fileDescriptor);
							log_trace(logger, mensaje);
							clearAndClose(fileDescriptor, &masterFD);
							break;
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
									agregarNodoATablaDeNodos(nuevoNodo);
									puts("Nodo inicializado y guardado en la lista");
									mostrarBitmap(nuevoNodo->bitmap);
								}
								else{
									//puede que esto no este bien
									//habria que probarlo
									infoBloque = recvInfoNodo(fileDescriptor);
									nuevoNodo = inicializarNodo(infoBloque, fileDescriptor);
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
int directorioNoExistente(char ** carpetas) {
	int cant, indicePadre = 0, i = 0;
	char * yamafs = malloc(10);
	cant = contarPunteroDePunteros(carpetas);
	strcpy(yamafs,"yamafs:");
	if (string_equals_ignore_case(carpetas[i], yamafs)) {
			i++;
	for (i = 1; i < cant; i++) {
		Tdirectorio * estructuraDirectorio = (Tdirectorio*) buscarPorNombreDeDirectorio(carpetas[i]);
		if (estructuraDirectorio != NULL) {
			if (estructuraDirectorio->padre == indicePadre) {
				indicePadre = estructuraDirectorio->index;
			} else {
				return i;
			}
		} else {
			return i;
		}
	}
	return -1;
	}else{
		puts("Falta la referencia al filesystem local : 'yamafs:'");
		return -1;
	}
}

bool ordenarListaPorMayor(void * directorio1, void * directorio2){
	Tdirectorio * directorioA = (Tdirectorio*)directorio1;
	Tdirectorio * directorioB = (Tdirectorio*)directorio2;

	int obtenerDirectorio(Tdirectorio *directorio) {
		return directorio->index;
	}
	return obtenerDirectorio(directorioA) >= obtenerDirectorio(directorioB);
}

int buscarIndexMayor(){
	list_sort(listaTablaDirectorios,ordenarListaPorMayor);
	Tdirectorio * directorio = (Tdirectorio*)list_get(listaTablaDirectorios,0);
	return directorio->index;
}

int crearDirectorio(char * ruta) {
	int nroDirectorio, cant, index, indicePadre;
	char ** carpetas = string_split(ruta, "/");
	cant = contarPunteroDePunteros(carpetas);
	char* directorio = malloc(40);
	strcpy(directorio, "src/metadata/archivos/");
	char * indice;
	Tdirectorio * tDirectorio = malloc(sizeof(Tdirectorio));

	if ((nroDirectorio = directorioNoExistente(carpetas)) < 0) {
		puts("El directorio no se puede crear");
		return -1;
	} else {
		if (nroDirectorio == cant - 1) {
			index = buscarIndexMayor() + 1;
			indicePadre = buscarIndexPorNombreDeDirectorio(
					carpetas[nroDirectorio - 1]);
			printf("Indice padre del nuevo directorio %d\n", indicePadre);
			printf("Index asignado al nuevo directorio %d\n", index);
			indice = string_itoa(index);
			string_append(&directorio, indice);
			syscall(SYS_mkdir, directorio);
			printf("Cree directorio %s\n", carpetas[nroDirectorio]);

			tDirectorio->index = index;
			strcpy(tDirectorio->nombre,carpetas[nroDirectorio]);
			tDirectorio->padre = indicePadre;

			list_add(listaTablaDirectorios,tDirectorio);
			return 0;
		}
			puts("No se puede crear directorio dentro de un directorio que no existe");
			return -1;

			free(indice);
	}
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
