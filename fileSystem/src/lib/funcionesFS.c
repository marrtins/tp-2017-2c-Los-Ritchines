#include "funcionesFS.h"

void almacenarBloquesEnEstructuraArchivo(Tarchivo * estructuraArchivoAAlmacenar, Tnodo * nodo1, Tnodo * nodo2, TbloqueAEnviar * bloque){
	Tbloques * tBloque = &estructuraArchivoAAlmacenar->bloques[bloque->numeroDeBloque];
	TcopiaNodo * copia0 = malloc(sizeof(TcopiaNodo));
	TcopiaNodo * copia1 = malloc(sizeof(TcopiaNodo));

	copia0->nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	strcpy(copia0->nombreDeNodo, nodo1->nombre);
	copia0->nroDeCopia = 0;
	printf("El nombre de nodo es %s\n", copia0->nombreDeNodo);
	

	tBloque->copia = list_create();

	int bloqueAOcupar = obtenerBloqueDisponible(nodo1);
	copia0->numeroBloqueDeNodo = bloqueAOcupar;
	ocuparBloque(nodo1, bloqueAOcupar);
	mostrarBitmap(nodo1->bitmap);

	list_add(tBloque->copia, copia0);

	copia1->nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
	strcpy(copia1->nombreDeNodo, nodo2->nombre);
	copia1->nroDeCopia = 1;
	printf("El nombre de nodo es %s\n", copia1->nombreDeNodo);

	bloqueAOcupar = obtenerBloqueDisponible(nodo2);
	copia1->numeroBloqueDeNodo = bloqueAOcupar;
	ocuparBloque(nodo2, bloqueAOcupar);
	mostrarBitmap(nodo2->bitmap);

	tBloque->bytes = bloque->tamanio;
	printf("El tamaño del bloque en bytes es: %llu", tBloque->bytes);

	list_add(tBloque->copia, copia1);
	tBloque->cantidadCopias = 2;
}

void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar){
	Theader * head = malloc(sizeof(Theader));
	Tbuffer * buffer1;
	Tbuffer * buffer2;
	//Tnodo * nodo1 = NULL;
	//Tnodo * nodo2 = NULL;
	int estado;
	head->tipo_de_proceso=FILESYSTEM;
	head->tipo_de_mensaje=ALMACENAR_BLOQUE;
	puts("llegue y rompi");
	//buscarLosDosNodosConMasDisponibilidad(listaDeNodos, nodo1, nodo2);
	list_sort(listaDeNodos, ordenarSegunBloquesDisponibles);
	Tnodo* nodo1 = (Tnodo*)list_get(listaDeNodos, 0);
	Tnodo* nodo2 = (Tnodo*)list_get(listaDeNodos, 1);
	//Tnodo* nodo3 = (Tnodo*)list_get(listaDeNodos, 2);
	//Tnodo* nodo4 = (Tnodo*)list_get(listaDeNodos, 3);
	puts("che, pude pasar");
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
	/*double obtenerProporcionDeDisponibilidad(Tnodo* nodo){
		if(nodo->cantidadBloquesLibres == 0) return 1;
		double bloquesOcupados = nodo->cantidadBloquesTotal - nodo->cantidadBloquesLibres;
		return bloquesOcupados / nodo->cantidadBloquesTotal;
	}*/
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
	*/

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
		//freir infoBloque->contenido en enviarBloque;
		//los tamaños varian según la posición del \n;
		printf("bloque a enviar %d\n",infoBloque->numeroDeBloque);
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
		log_trace(logger, "Error al almacenar archivo, está vacío");
		liberarEstructuraBloquesAEnviar(infoBloque);
		return -1;
	}

	if(list_size(listaDeNodos) <= 1){
		puts("No hay nodos conectados, hermano");
		return -1;
	}

	if(verificarDisponibilidadDeEspacioEnNodos(tamanio) == -1){
		puts("No hay suficiente espacio en los datanodes, intente con un archivo más chico");
		log_trace(logger, "No hay suficiente espacio en los datanodes, intente con un archivo más chico");
		puts("voy a violar el segmento");
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

int almacenarArchivo(char **palabras){
	//palabras[1] --> ruta archivo a almacenar
	//palabras[2] --> ruta de nuestro directorio
	char ** splitDeRuta = string_split(palabras[1], "/");
	char * nombreArchivoConExtension = obtenerUltimoElementoDeUnSplit(splitDeRuta);
	printf("El archivo a guardar es: %s\n", nombreArchivoConExtension);

	Tarchivo * archivoAAlmacenar = malloc(sizeof(Tarchivo));
	archivoAAlmacenar->nombreArchivoSinExtension = obtenerNombreDeArchivoSinExtension(nombreArchivoConExtension);
	archivoAAlmacenar->extensionArchivo = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);

	if(procesarArchivoSegunExtension(archivoAAlmacenar, palabras[1]) == -1){
		liberarPunteroDePunterosAChar(splitDeRuta);
		free(splitDeRuta);
		free(nombreArchivoConExtension);
		free(archivoAAlmacenar->nombreArchivoSinExtension);
		free(archivoAAlmacenar->extensionArchivo);
		free(archivoAAlmacenar->bloques);
		free(archivoAAlmacenar);
		return -1;
	}
	guardarTablaDeArchivo(archivoAAlmacenar, palabras[2]);
	almacenarTodosLosBitmaps(listaDeNodos);
	liberarPunteroDePunterosAChar(splitDeRuta);
	free(splitDeRuta);
	free(nombreArchivoConExtension);
	liberarTablaDeArchivo(archivoAAlmacenar);
	return 1;
}

Tnodo * inicializarNodo(TpackInfoBloqueDN * infoBloqueRecibido, int fileDescriptor, Tnodo * nuevoNodo){
	nuevoNodo->fd = fileDescriptor;
	nuevoNodo->cantidadBloquesTotal = infoBloqueRecibido->databinEnMB;
	nuevoNodo->cantidadBloquesLibres = infoBloqueRecibido->databinEnMB;
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
	free(nombreNodo);
	free(ipNodo);
	free(puertoNodo);

	return infoBloque;
}



int levantarArchivo(Tarchivo * tablaArchivo, char * ruta){
	int cantBloques, nroBloque=0;
	Tbuffer* bloque = malloc(sizeof(Tbuffer));
	int fd;
	char * archivoMapeado;

	cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal);

	FILE * archivo = fopen(ruta, "w+");
	fd = fileno(archivo);
	ftruncate(fd, tablaArchivo->tamanioTotal);

	if ((archivoMapeado = mmap(NULL, tablaArchivo->tamanioTotal, PROT_WRITE, MAP_SHARED,fd, 0)) == MAP_FAILED) {
		log_trace(logger,"Error al hacer mmap");
		puts("Error al hacer mmap");
		liberarEstructuraBuffer(bloque);
		return -1;
	}
	fclose(archivo);
	close(fd);

	char * p = archivoMapeado;
	while(nroBloque != cantBloques){

		if(nodosDisponiblesParaBloqueDeArchivo(tablaArchivo, nroBloque) == 0){
			puts("No se encontraron los nodos con las copias del bloque");
			return -1;
			}

		//pthread_cond_init(&bloqueCond, NULL);
		pthread_mutex_init(&bloqueMutex,NULL);
		pthread_mutex_lock(&bloqueMutex);
		printf("Voy a pedir el bloque %d\n",nroBloque);
		pedirBloque(tablaArchivo, nroBloque);

		pthread_mutex_lock(&bloqueMutex);
		//pthread_cond_wait(&bloqueCond, &bloqueMutex);
		//pthread_mutex_unlock(&bloqueMutex);

		puts("pase el mutex, voy a copiar un bloque");
		if(copiarBloque(bloqueACopiar, bloque) == -1){
			puts("Error al copiar bloque recibido. Intentelo de nuevo");
			log_trace(logger,"Error al copiar bloque recibido");
			liberarEstructuraBuffer(bloque);
		//borrar archivo
			return -1;
		}
		puts("voy a hacer un memcpy");
		memcpy(p,bloque->buffer,bloque->tamanio);
		p += bloque->tamanio;
		puts("hice el memcpy");
		nroBloque++;
	}

	if (msync((void *)archivoMapeado, tablaArchivo->tamanioTotal, MS_SYNC) < 0) {
		log_trace(logger,"Error al hacer msync");
		puts("Error al hacer msync");
		return -1;
	}
	puts("Achivo copiado con éxito");
	liberarEstructuraBuffer(bloque);
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
	puts("levante la tabla");
	nombreArchivo = obtenerNombreDeArchivoDeUnaRuta(palabras[1]);
	strcpy(rutaDirectorio,palabras[2]);

	if(!string_ends_with(rutaDirectorio,"/")){
		string_append(&rutaDirectorio,"/");
	}
	string_append(&rutaDirectorio,nombreArchivo);
	puts("vot a levantar archivo");
	if(levantarArchivo(archivo,rutaDirectorio) == -1){
		puts("Error al levantar archivo");
		return -1;
	}
	puts("ya levante archivo");

	liberarTablaDeArchivo(archivo);
	free(rutaTablaArchivo);
	free(nombreArchivo);
	free(rutaDirectorio);
	return 1;
}

int pedirBloque(Tarchivo* tablaArchivo, int nroBloque){
	Tbloques* tBloque = &tablaArchivo->bloques[nroBloque];
	Tnodo * nodo;
	int i = 0;
	TcopiaNodo * copiaNodo;
	Theader* header = malloc(sizeof(Theader));
	Tbuffer * buffer;
	header->tipo_de_proceso = FILESYSTEM;
	header->tipo_de_mensaje = OBTENER_BLOQUE;
	while(i < tBloque->cantidadCopias){
		copiaNodo = (TcopiaNodo *)list_get(tBloque->copia,i);
		nodo = buscarNodoPorNombre(listaDeNodos, copiaNodo->nombreDeNodo);
		if(nodo != NULL){
			buffer = empaquetarPeticionBloque(header, nroBloque, tBloque->bytes);
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
	puts("No se encontraron nodos donde realizar la petición");
	free(header);
	liberarEstructuraBuffer(buffer);
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
	while(i < tBloque->cantidadCopias){
		nodo = (Tnodo*)list_get(listaDeNodos,i);
		if(nodo != NULL){
			return 1;
		}
		i++;
	}

	return 0;
}
