#include "../funcionesFS.h"

char * recvRutaArchivo(int socket){
	int tamanio;
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

	return ruta;
}


TinfoArchivoFSYama * crearListaTablaArchivoParaYama(Tarchivo * archivo){

	TinfoArchivoFSYama *infoSend = malloc(sizeof(TinfoArchivoFSYama));
	t_list * listaBloques = list_create();

	int i;
	int cantBloques = cantidadDeBloquesDeUnArchivo(archivo->tamanioTotal);
	for (i = 0; i < cantBloques; i++) {
		TpackageUbicacionBloques *bloque = malloc(
				sizeof(TpackageUbicacionBloques));
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

	Tbuffer * buffer;
	infoNodo->head.tipo_de_proceso = FILESYSTEM;
	infoNodo->head.tipo_de_mensaje = INFO_NODO;

	buffer = empaquetarInfoNodo(infoNodo);

	if ((send(socketYama, buffer->buffer , buffer->tamanio, 0)) == -1){
		logAndExit("Fallo al enviar la informacion de un archivo");
	}

}
