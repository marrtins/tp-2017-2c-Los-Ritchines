#include "../funcionesFS.h"

char * recvRutaArchivo(int socket){


	char * buffer;
	TpackBytes *pathArchivo;

	puts("Nos llega el path del archivo");

	if ((buffer = recvGeneric(socket)) == NULL){
		puts("Fallo recepcion de PATH_FILE_TOREDUCE");
		return NULL;
	}

	if ((pathArchivo = (TpackBytes *) deserializeBytes(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del path arch a reducir");
		return NULL;
	}

	printf("Path archivo: %s\n",pathArchivo->bytes);

	return pathArchivo->bytes;



	/*int tamanio;
	char * ruta;
	char * rutaArchivo;

	if (recv(socket, &tamanio, sizeof(int), 0) == -1) {
		logAndExit("Error al recibir el tamanio de la ruta");
	}
	ruta = malloc(tamanio);
	rutaArchivo = malloc(tamanio);

	if (recv(socket, rutaArchivo, tamanio, 0) == -1) {
		logAndExit("Error al recibir la ruta");
	}
	puts(rutaArchivo);
	memcpy(ruta, rutaArchivo, tamanio);
	free(rutaArchivo);

	return ruta;*/
}


TinfoArchivoFSYama * crearListaTablaArchivoParaYama(Tarchivo * archivo){

	TinfoArchivoFSYama *infoSend = malloc(sizeof(TinfoArchivoFSYama));
	t_list * listaBloques = list_create();

	int i;
	int cantBloques = cantidadDeBloquesDeUnArchivo(archivo->tamanioTotal);
	for (i = 0; i < cantBloques; i++) {
		TpackageUbicacionBloques *bloque = malloc(sizeof(TpackageUbicacionBloques));
		TcopiaNodo *copia1 = list_get(archivo->bloques[i].copia, 0);
		bloque->bloque = i;
		bloque->nombreNodoC1 = malloc(strlen(copia1->nombreDeNodo) + 1);
		bloque->nombreNodoC1 = copia1->nombreDeNodo;
		bloque->nombreNodoC1Len = (strlen(copia1->nombreDeNodo) + 1);
		bloque->bloqueC1 = copia1->numeroBloqueDeNodo;

		TcopiaNodo *copia2 = list_get(archivo->bloques[i].copia, 1);
		bloque->nombreNodoC2 = malloc(strlen(copia2->nombreDeNodo) + 1);
		bloque->nombreNodoC2 = copia2->nombreDeNodo;
		bloque->nombreNodoC2Len = (strlen(copia2->nombreDeNodo) + 1);
		bloque->bloqueC2 = copia2->numeroBloqueDeNodo;

		bloque->finBloque = archivo->bloques[i].bytes;
		list_add(listaBloques, bloque);
	}

	infoSend->listaSize = list_size(listaBloques);
	infoSend->listaBloques = list_create();
	infoSend->listaBloques = listaBloques;

	return infoSend;

}

void enviarInfoNodoAYama(int socketYama, Tarchivo * archivo){

	char * buffer;
	int packSize;
	char ** nodos;
	Theader head;
	head.tipo_de_proceso=FILESYSTEM;
	head.tipo_de_mensaje=INFO_NODO;
	t_list * listaNodos;

	nodos = obtenerNodosDeUnArchivo(archivo);

	listaNodos = generarListaInfoNodos(nodos);

	TinfoNodosFSYama *infoNodos = malloc(sizeof(TinfoNodosFSYama));
	infoNodos->listaSize=list_size(listaNodos);
	infoNodos->listaNodos=list_create();
	infoNodos->listaNodos=listaNodos;

	buffer = serializarInfoNodosYamaFS(head,infoNodos,&packSize);

	if ((send(socketYama, buffer ,packSize, 0)) == -1){
		logErrorAndExit("Fallo al enviar la informacion de un archivo");
	}
	printf("pack size info nodos %d\n",packSize);
	puts("envie lista de nodos");

	//liberarPunteroDePunterosAChar(nodos);
	//free(nodos);
	//list_destroy_and_destroy_elements(listaNodos,liberarTPackageInfoNodo);
	//list_destroy_and_destroy_elements(infoNodos->listaNodos, liberarTPackageInfoNodo);
	//free(infoNodos);
	//free(buffer);

}




t_list* generarListaInfoNodos(char **nodos){

	t_list * listaNodos = list_create();
	TinfoNodo * infoNodo = malloc(sizeof(TinfoNodo));
	int i = 0;

	int ipTamanio=10;
	int puertoTamanio=5;

	while(nodos[i] != NULL){

		TpackageInfoNodo *nodo = malloc(sizeof(TpackageInfoNodo));
		infoNodo = buscarInfoNodoPorNombre(listaInfoNodo, nodos[i]);


		nodo->nombreNodo = malloc(TAMANIO_NOMBRE_NODO);
		strcpy(nodo->nombreNodo,infoNodo->nombre);
		nodo->tamanioNombre = strlen(nodo->nombreNodo)+1;

		nodo->ipNodo = malloc(ipTamanio);
		strcpy(nodo->ipNodo,infoNodo->ip);
		nodo->tamanioIp = strlen(nodo->ipNodo)+1;

		nodo->puertoWorker =malloc(puertoTamanio);
		strcpy(nodo->puertoWorker,infoNodo->puerto);
		nodo->tamanioPuerto=strlen(nodo->puertoWorker)+1;

		list_add(listaNodos,nodo);
		i++;
	}

	return listaNodos;
}

