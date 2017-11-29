#include "funcionesFS.h"

void almacenarBloquesEnEstructuraArchivo(Tarchivo * estructuraArchivoAAlmacenar, Tnodo * nodo1, Tnodo * nodo2, TbloqueAEnviar * bloque){
	Tbloques * tBloque = &estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque];
	TcopiaNodo * copia0 = malloc(sizeof(TcopiaNodo));
	TcopiaNodo * copia1 = malloc(sizeof(TcopiaNodo));

	copia0->nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	strcpy(copia0->nombreDeNodo, nodo1->nombre);
	copia0->nroDeCopia = 0;

	tBloque->copia = list_create();

	int bloqueAOcupar = obtenerBloqueDisponible(nodo1);
	copia0->numeroBloqueDeNodo = bloqueAOcupar;
	ocuparBloque(nodo1, bloqueAOcupar);

	list_add(tBloque->copia, copia0);

	copia1->nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	strcpy(copia1->nombreDeNodo, nodo2->nombre);
	copia1->nroDeCopia = 1;

	bloqueAOcupar = obtenerBloqueDisponible(nodo2);
	copia1->numeroBloqueDeNodo = bloqueAOcupar;
	ocuparBloque(nodo2, bloqueAOcupar);

	tBloque->bytes = bloque->tamanio;

	list_add(tBloque->copia, copia1);
	tBloque->cantidadCopias = 2;
}

void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar){
	Theader * head = malloc(sizeof(Theader));
	Tbuffer * buffer1;
	Tbuffer * buffer2;
	int estado;
	head->tipo_de_proceso=FILESYSTEM;
	head->tipo_de_mensaje=ALMACENAR_BLOQUE;
	list_sort(listaDeNodos, ordenarSegunBloquesDisponibles);
	Tnodo * nodo1 = (Tnodo*)buscarNodoDiponibleParaEnviar(listaDeNodos);
	Tnodo * nodo2 = (Tnodo*)buscarNodoDiponibleParaEnviar(listaDeNodos);
	if(nodo1 == NULL || nodo2 == NULL){
		if(nodo1 == NULL){
			//significa que, todos los nodos estan en 1
			setearDisponibilidadDeEnvioDeNodos(listaDeNodos, 0);
			nodo1 = (Tnodo*)buscarNodoDiponibleParaEnviar(listaDeNodos);
			nodo2 = (Tnodo*)buscarNodoDiponibleParaEnviar(listaDeNodos);
		}
		else{
			//hay uno solo en cero
			setearDisponibilidadDeEnvioDeNodos(listaDeNodos, 0);
			nodo1->estadoParaEnviarBloque = 1;
			nodo2 = (Tnodo*)buscarNodoDiponibleParaEnviar(listaDeNodos);
		}
	}
	//Tnodo* nodo1 = (Tnodo*)list_get(listaDeNodos, 0);
	//Tnodo* nodo2 = (Tnodo*)list_get(listaDeNodos, 1);
	buffer1 = empaquetarBloque(head,bloque,nodo1);

	 if ((estado = send(nodo1->fd, buffer1->buffer , buffer1->tamanio, 0)) == -1){
		 logErrorAndExit("Fallo al enviar a Nodo el bloque a almacenar.");
	 }

	buffer2 = empaquetarBloque(head,bloque,nodo2);
	 if ((estado = send(nodo2->fd, buffer2->buffer , buffer2->tamanio, 0)) == -1){
		 logErrorAndExit("Fallo al enviar a Nodo el bloque a almacenar.");
	 }

	almacenarBloquesEnEstructuraArchivo(estructuraArchivoAAlmacenar, nodo1, nodo2, bloque);
	free(head);
	liberarEstructuraBuffer(buffer1);
	liberarEstructuraBuffer(buffer2);
}

int cantidadDeBloquesDeUnArchivo(unsigned long long tamanio){
	float tamanioBloque = BLOQUE_SIZE;
	return ceil((float)tamanio / tamanioBloque);
}

void guardarTablaDeArchivo(Tarchivo * archivoAAlmacenar, char * rutaDestino){
	int index = obtenerIndexDeUnaRuta(rutaDestino);
	char * rutaArchivo = malloc(200);
	sprintf(rutaArchivo, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/%d/%s.%s", index, archivoAAlmacenar->nombreArchivoSinExtension, archivoAAlmacenar->extensionArchivo);
	FILE * archivo = fopen(rutaArchivo, "w+");
	fclose(archivo);
	almacenarEstructuraArchivoEnUnArchivo(archivoAAlmacenar, rutaArchivo);
	free(rutaArchivo);

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
	char * punteroInicioBloque = archivoMapeado;
	unsigned long long bytesFaltantesPorEnviar = archivoAAlmacenar->tamanioTotal;
	char * punteroFinalBloque = archivoMapeado;
	unsigned long long bytesACopiar = 0;


	while(bytesFaltantesPorEnviar > 0){
		if(bytesFaltantesPorEnviar < BLOQUE_SIZE){
			memcpy(infoBloque->contenido,punteroInicioBloque,bytesFaltantesPorEnviar);
			infoBloque->tamanio = bytesFaltantesPorEnviar;
			bytesFaltantesPorEnviar = 0;
			bytesACopiar = 0;
		}
		else {
			punteroFinalBloque += BLOQUE_SIZE;
			bytesACopiar = BLOQUE_SIZE;
			while(*punteroFinalBloque != '\n'){

				punteroFinalBloque--;
				bytesACopiar--;
			}
			memcpy(infoBloque->contenido,punteroInicioBloque,bytesACopiar);
			infoBloque->tamanio = bytesACopiar;
		}
		bytesFaltantesPorEnviar-=bytesACopiar;
		enviarBloque(infoBloque, archivoAAlmacenar);
		punteroInicioBloque+=bytesACopiar;
		infoBloque->numeroDeBloque++;
	}
}

int esPar(int numero){
	return !(numero % 2);
}

int capacidadDeAlmacenamientoDeFileSystem(Tnodo * nodoMaximo, int sumaSinMaximo){
	int total = nodoMaximo->cantidadBloquesLibres + sumaSinMaximo;
	if(nodoMaximo->cantidadBloquesLibres > sumaSinMaximo){
		return sumaSinMaximo;
	}else if(nodoMaximo->cantidadBloquesLibres == sumaSinMaximo){
		return nodoMaximo->cantidadBloquesLibres;
	}else if(esPar(total)){
		return total/2;
	}
	else{
		return (total-1)/2;
	}

}

int verificarDisponibilidadDeEspacioEnNodos(unsigned long long tamanioDelArchivoAGuardar){
	int tamanioEnMBArchivo = cantidadDeBloquesDeUnArchivo(tamanioDelArchivoAGuardar);
	Tnodo * nodoMaximo = obtenerNodoPorTamanioMaximo();
	int sumaSinMaximo = sumarBloquesLibresDeNodoSinElMaximo(nodoMaximo);
	int capacidadEnMB = capacidadDeAlmacenamientoDeFileSystem(nodoMaximo, sumaSinMaximo);
	if(tamanioEnMBArchivo > capacidadEnMB){
		return 0;
	}
	return 1;
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

	if(tamanio == 0){
		puts("Error al almacenar archivo, está vacío.");
		log_error(logError, "Error al almacenar archivo, está vacío.");
		liberarEstructuraBloquesAEnviar(infoBloque);
		return -1;
	}

	if ((archivoMapeado = mmap(NULL, tamanio, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		logErrorAndExit("Error al hacer mmap del archivo a copiar.");
	}

	fclose(archivoOrigen);
	close(fd);

	archivoAAlmacenar->bloques = malloc(sizeof(Tbloques) * cantidadDeBloquesDeUnArchivo(tamanio));

	if(list_size(listaDeNodos) <= 1){
		puts("No hay nodos conectados, imposible realizar la operacion.");
		log_error(logError, "Los nodos no están conectados, error en realizar la operación de almacenar en yamafs.");
		liberarEstructuraBloquesAEnviar(infoBloque);
		return -1;
	}

	if(verificarDisponibilidadDeEspacioEnNodos(tamanio) == 0){
		puts("No hay suficiente espacio en los datanodes, intente con un archivo más chico");
		log_error(logError, "No hay suficiente espacio en los datanodes, intente con un archivo más chico");
		liberarEstructuraBloquesAEnviar(infoBloque);
		return -1;
	}

	if(strcmp(archivoAAlmacenar->extensionArchivo, "csv") == 0){
		procesarArchivoCsv(archivoAAlmacenar, archivoMapeado, infoBloque);
	}
	else{
		procesarArchivoBinario(archivoAAlmacenar, archivoMapeado, infoBloque);
	}

	liberarEstructuraBloquesAEnviar(infoBloque);
	return 1;
}

int archivoRepetidoEnDirectorio(char* rutaLocalArchivo, char* rutaDestinoYamafs){
	char* nombreArchivo = obtenerNombreDeArchivoDeUnaRuta(rutaLocalArchivo);
	char* rutaArchivoYamafs = string_new();
	char input;
	string_append(&rutaArchivoYamafs,rutaDestinoYamafs);
	string_append(&rutaArchivoYamafs,"/");
	string_append(&rutaArchivoYamafs,nombreArchivo);
	int index = obtenerIndexDeUnaRuta(rutaDestinoYamafs);
	if(existeArchivo(index,rutaArchivoYamafs)){
		puts("El archivo ya existe en esta ubicacion.");
		puts("Desea sobreescribirlo? s / n");
		scanf(" %c",&input);
		while (input != 's' && input != 'n' && input != 'S' && input != 'N'){
			puts("Caracter incorrecto.");
			puts("Desea sobreescribirlo? s / n");
			scanf("%c", &input);
		}
		if (input == 'n' || input == 'N'){
			free(rutaArchivoYamafs);
			free(nombreArchivo);
			return 0;
		}
		if(removerArchivo(rutaArchivoYamafs)){
			//puts("El archivo se elimino correctamente.");
		}
		else{
			puts("No se pudo remover el archivo, el nodo con la copia no esta en ningun lado.");
		}
	}
	free(rutaArchivoYamafs);
	free(nombreArchivo);
	return 1;
}

int almacenarArchivo(char **palabras){
	//palabras[1] --> ruta archivo a almacenar
	//palabras[2] --> ruta de nuestro directorio
	char ** splitDeRuta = string_split(palabras[1], "/");
	char * nombreArchivoConExtension = obtenerUltimoElementoDeUnSplit(splitDeRuta);

	Tarchivo * archivoAAlmacenar = malloc(sizeof(Tarchivo));
	archivoAAlmacenar->nombreArchivoSinExtension = obtenerNombreDeArchivoSinExtension(nombreArchivoConExtension);
	archivoAAlmacenar->extensionArchivo = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);

	if(procesarArchivoSegunExtension(archivoAAlmacenar, palabras[1]) == -1){
		liberarPunteroDePunterosAChar(splitDeRuta);
		free(splitDeRuta);
		free(nombreArchivoConExtension);
		//no se utiliza la funcion que libera la estructura porque
		//si falla, no se malloquean los bloques
		free(archivoAAlmacenar->nombreArchivoSinExtension);
		free(archivoAAlmacenar->extensionArchivo);
		free(archivoAAlmacenar);
		return 0;
	}
	guardarTablaDeArchivo(archivoAAlmacenar, palabras[2]);
	almacenarTodosLosBitmaps(listaDeNodos);
	liberarPunteroDePunterosAChar(splitDeRuta);
	free(splitDeRuta);
	free(nombreArchivoConExtension);
	liberarTablaDeArchivo(archivoAAlmacenar);
	return 1;
}

TinfoNodo * inicializarInfoNodo(TpackInfoBloqueDN * infoBloqueRecibido){
	TinfoNodo * infoNuevoNodo = malloc(sizeof(TinfoNodo));

	infoNuevoNodo->nombre = strdup(infoBloqueRecibido->nombreNodo);
	infoNuevoNodo->ip = strdup(infoBloqueRecibido->ipNodo);
	infoNuevoNodo->puerto = strdup(infoBloqueRecibido->puertoNodo);

	return infoNuevoNodo;
}

Tnodo * inicializarNodo(TpackInfoBloqueDN * infoBloqueRecibido, int fileDescriptor, Tnodo * nuevoNodo){
	nuevoNodo->fd = fileDescriptor;
	nuevoNodo->cantidadBloquesTotal = infoBloqueRecibido->databinEnMB;
	nuevoNodo->cantidadBloquesLibres = infoBloqueRecibido->databinEnMB;
	nuevoNodo->nombre = strdup(infoBloqueRecibido->nombreNodo);
	nuevoNodo->bitmap = crearBitmap(infoBloqueRecibido->databinEnMB);
	nuevoNodo->estadoParaEnviarBloque = 0;
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
		logErrorAndExit("Error al recibir el tamanio del nombre del nodo.");
	}

	nombreNodo = malloc(infoBloque->tamanioNombre);

	//Recibo el nombre del nodo
	if ((estado = recv(socketFS, nombreNodo, infoBloque->tamanioNombre, 0)) == -1) {
		logErrorAndExit("Error al recibir el nombre del nodo.");
	}

	//Recibo el tamanio de la ip del nodo
	if ((estado = recv(socketFS, &infoBloque->tamanioIp, sizeof(int), 0)) == -1) {
		logErrorAndExit("Error al recibir el tamanio del ip del nodo.");
	}

	ipNodo = malloc(infoBloque->tamanioIp);

	//Recibo la ip del nodo
	if ((estado = recv(socketFS, ipNodo, infoBloque->tamanioIp, 0)) == -1) {
		logErrorAndExit("Error al recibir el ip del nodo.");
	}

	//Recibo el tamanio del puerto del nodo
	if ((estado = recv(socketFS, &infoBloque->tamanioPuerto, sizeof(int), 0)) == -1) {
		logErrorAndExit("Error al recibir el tamanio del puerto del nodo.");
	}

	puertoNodo = malloc(infoBloque->tamanioPuerto);

	//Recibo el puerto del nodo
	if ((estado = recv(socketFS, puertoNodo, infoBloque->tamanioPuerto, 0)) == -1) {
		logErrorAndExit("Error al recibir el puerto del nodo.");
	}

	//Recibo el databin en MB
	if ((estado = recv(socketFS, &infoBloque->databinEnMB, sizeof(int), 0)) == -1) {
		logErrorAndExit("Error al recibir el tamanio del databin.");
	}

	infoBloque = desempaquetarInfoNodo(infoBloque, nombreNodo, ipNodo, puertoNodo);
	free(nombreNodo);
	free(ipNodo);
	free(puertoNodo);

	return infoBloque;
}

int levantarArchivo(Tarchivo * tablaArchivo, char * ruta){
	int cantBloques, nroBloque=0;
	int fd;
	char * archivoMapeado;

	cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal);

	FILE * archivo = fopen(ruta, "w+");
	fd = fileno(archivo);
	ftruncate(fd, tablaArchivo->tamanioTotal);

	if ((archivoMapeado = mmap(NULL, tablaArchivo->tamanioTotal, PROT_WRITE, MAP_SHARED,fd, 0)) == MAP_FAILED) {
		log_error(logError, "Error al hacer mmap al levantar el archivo.");
		return -1;
	}

	close(fd);
	fclose(archivo);
	char* mensajePeticionBloque;
	char * p = archivoMapeado;
	while(nroBloque < cantBloques){

		if(nodosDisponiblesParaBloqueDeArchivo(tablaArchivo, nroBloque) == 0){
			puts("No se encuentran los nodos con las copias del bloque");
			log_error(logError, "No se encontraron los nodos con las copias del bloque.");
			return -1;
		}

		//pthread_cond_init(&bloqueCond, NULL);
		pthread_mutex_init(&bloqueMutex,NULL);
		pthread_mutex_lock(&bloqueMutex);
		mensajePeticionBloque = malloc(25);
		sprintf(mensajePeticionBloque,"Voy a pedir el bloque %d.",nroBloque);
		log_info(logInfo,mensajePeticionBloque);
		free(mensajePeticionBloque);
		if(pedirBloque(tablaArchivo, nroBloque) == -1){
			log_error(logError,"Error al levantar archivo.");
			return -1;
		}

		pthread_mutex_lock(&bloqueMutex);
		//pthread_cond_wait(&bloqueCond, &bloqueMutex);
		//pthread_mutex_unlock(&bloqueMutex);

		Tbuffer* bloque = malloc(sizeof(Tbuffer));

		copiarBloque(bloqueACopiar, bloque);
		memcpy(p,bloque->buffer,bloque->tamanio);
		p += bloque->tamanio;
		liberarEstructuraBuffer(bloque);
		liberarEstructuraBuffer(bloqueACopiar);
		nroBloque++;
	}

	if (msync((void *)archivoMapeado, tablaArchivo->tamanioTotal, MS_SYNC) < 0) {
		log_error(logError, "Error al hacer msync, al levantar el archivo.");
		return -1;
	}

	munmap(archivoMapeado,tablaArchivo->tamanioTotal);
	return 1;
}


int copiarArchivo(char ** palabras){
	//palabras[1] --> ruta archivo yamafs
	//palabras[2] --> directorio
	char * rutaTablaArchivo;
	char * nombreArchivo;
	char * rutaDirectorio = malloc(100);
	Tarchivo * archivo = malloc(sizeof(Tarchivo));
	rutaTablaArchivo = obtenerRutaLocalDeArchivo(palabras[1]);

	levantarTablaArchivo(archivo,rutaTablaArchivo);
	nombreArchivo = obtenerNombreDeArchivoDeUnaRuta(palabras[1]);
	strcpy(rutaDirectorio,palabras[2]);

	if(!string_ends_with(rutaDirectorio,"/")){
		string_append(&rutaDirectorio,"/");
	}
	string_append(&rutaDirectorio,nombreArchivo);
	if(levantarArchivo(archivo,rutaDirectorio) == -1){
		puts("Error al levantar archivo");
		return -1;
	}

	liberarTablaDeArchivo(archivo);
	free(rutaTablaArchivo);
	free(nombreArchivo);
	free(rutaDirectorio);
	return 1;
}

int pedirBloque(Tarchivo* tablaArchivo, int nroBloque){
	Tbloques* tBloque = &tablaArchivo->bloques[nroBloque];
	//TcopiaNodo * copiaNodo=list_get(tBloque.copia,0);

	Tnodo * nodo;
	int i = 0;
	TcopiaNodo * copiaNodo;
	Theader* header = malloc(sizeof(Theader));
	Tbuffer * buffer = NULL;
	header->tipo_de_proceso = FILESYSTEM;
	header->tipo_de_mensaje = OBTENER_BLOQUE;
	while(i < tBloque->cantidadCopias){
		copiaNodo = (TcopiaNodo *)list_get(tBloque->copia,i);
		nodo = buscarNodoPorNombre(listaDeNodos, copiaNodo->nombreDeNodo);
		if(nodo != NULL){
			buffer = empaquetarPeticionBloque(header, copiaNodo->numeroBloqueDeNodo, tBloque->bytes);
			if ((send(nodo->fd, buffer->buffer , buffer->tamanio, 0)) == -1){
				free(header);
				liberarEstructuraBuffer(buffer);
				return -1;
			}
			free(header);
			liberarEstructuraBuffer(buffer);
			return 1;
		}
		else {
			i++;
		}
	}
	log_error(logError, "No se encontraron nodos donde realizar la petición.");
	free(header);
	return -1;
}

int copiarBloque(Tbuffer* buffer, Tbuffer* bloque){
	if(buffer->buffer == NULL){
		return -1;
	}
	bloque->buffer = malloc(buffer->tamanio);
	memcpy(&bloque->tamanio,&buffer->tamanio,sizeof(unsigned long long));
	memcpy(bloque->buffer,buffer->buffer,buffer->tamanio);
	return 1;
}

int enviarBloqueA(TbloqueAEnviar* bloque, char* nombreNodo){
	Theader* head = malloc(sizeof(Theader));

	Tnodo* nodo = buscarNodoPorNombre(listaDeNodos,nombreNodo);

	Tbuffer* buffer;
	head->tipo_de_proceso = FILESYSTEM;
	head->tipo_de_mensaje = ALMACENAR_BLOQUE;
	buffer = empaquetarBloque(head, bloque, nodo);
	if(send(nodo->fd,buffer->buffer,buffer->tamanio,0) == -1){
		free(head);
		liberarEstructuraBuffer(buffer);
		return -1;
	}
	free(head);
	liberarEstructuraBuffer(buffer);
	return 1;
}

int nodosDisponiblesParaBloqueDeArchivo(Tarchivo* tablaArchivo,int nroBloque){
	int i = 0;
	Tbloques * tBloque = &tablaArchivo->bloques[nroBloque];
	Tnodo* nodo;
	TcopiaNodo * copiaNodo;
	while(i < tBloque->cantidadCopias){
		copiaNodo = (TcopiaNodo*)list_get(tBloque->copia,i);
		nodo = buscarNodoPorNombre(listaDeNodos, copiaNodo->nombreDeNodo);
		if(nodo != NULL){
			return 1;
		}
		i++;
	}

	return 0;
}
