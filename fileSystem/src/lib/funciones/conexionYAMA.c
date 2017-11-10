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

void enviarInfoNodoAYama(int socketYama){



	char * buffer;
	int packSize;
	Theader head;
	head.tipo_de_proceso=FILESYSTEM;
	head.tipo_de_mensaje=INFO_NODO;
	t_list * listaNodos = list_create();
	//lista hardcode para probar:
	generarListaInfoNodos(listaNodos);

	TinfoNodosFSYama *infoNodos = malloc(sizeof(TinfoNodosFSYama));
	infoNodos->listaSize=list_size(listaNodos);
	infoNodos->listaNodos=list_create();
	infoNodos->listaNodos=listaNodos;


	buffer = serializarInfoNodosYamaFS(head,infoNodos,&packSize);

	if ((send(socketYama, buffer ,packSize, 0)) == -1){
		logAndExit("Fallo al enviar la informacion de un archivo");
	}
	printf("pack size info nodos %d\n",packSize);
	puts("envie lista de nodos");

}




void generarListaInfoNodos(t_list *nodos){

	//lsita hardcode para probar

	int nombreLen=6;
	int ipLen=10;
	int puertoLen=5;


	TpackageInfoNodo *nodo1 = malloc(sizeof(TpackageInfoNodo));
	nodo1->nombreNodo=malloc(nombreLen);
	nodo1->nombreNodo="Nodo1";
	nodo1->tamanioNombre=strlen(nodo1->nombreNodo)+1;
	nodo1->ipNodo=malloc(ipLen);
	nodo1->ipNodo="127.0.0.1";
	nodo1->tamanioIp=strlen(nodo1->ipNodo)+1;
	nodo1->puertoWorker=malloc(puertoLen);
	nodo1->puertoWorker = "5013";
	nodo1->tamanioPuerto=strlen(nodo1->puertoWorker)+1;
	list_add(nodos,nodo1);

	TpackageInfoNodo *nodo2 = malloc(sizeof(TpackageInfoNodo));
	nodo2->nombreNodo=malloc(nombreLen);
	nodo2->nombreNodo="Nodo2";
	nodo2->tamanioNombre=strlen(nodo2->nombreNodo)+1;
	nodo2->ipNodo=malloc(ipLen);
	nodo2->ipNodo="127.0.0.1";
	nodo2->tamanioIp=strlen(nodo2->ipNodo)+1;
	nodo2->puertoWorker=malloc(puertoLen);
	nodo2->puertoWorker = "5014";
	nodo2->tamanioPuerto=strlen(nodo2->puertoWorker)+1;

	list_add(nodos,nodo2);

	TpackageInfoNodo *nodo3 = malloc(sizeof(TpackageInfoNodo));
	nodo3->nombreNodo=malloc(nombreLen);
	nodo3->nombreNodo="Nodo3";
	nodo3->tamanioNombre=strlen(nodo3->nombreNodo)+1;
	nodo3->ipNodo=malloc(ipLen);
	nodo3->ipNodo="127.0.0.1";
	nodo3->tamanioIp=strlen(nodo3->ipNodo)+1;
	nodo3->puertoWorker=malloc(puertoLen);
	nodo3->puertoWorker = "5034";
	nodo3->tamanioPuerto=strlen(nodo3->puertoWorker)+1;

	list_add(nodos,nodo3);


}

