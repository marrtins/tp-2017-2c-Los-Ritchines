#include "../funcionesFS.h"

void empaquetarBloqueAEliminar(Tbuffer * buffer, Theader * head, int numeroDeBloque){
	buffer->tamanio = sizeof(head) + sizeof(numeroDeBloque);
	buffer->buffer = malloc(buffer->tamanio);
	char * p = buffer->buffer;
	memcpy(p, head, sizeof(*head));
	p += sizeof(*head);
	memcpy(p, &numeroDeBloque, sizeof(numeroDeBloque));
}

void desempaquetarArchivoFinal(int fileDescriptor, TarchivoFinal * archivoFinal){

	if (recv(fileDescriptor, &archivoFinal->tamanio, sizeof(archivoFinal->tamanio), 0) == -1) {
		logErrorAndExit("Error al recibir el tamanio del bloque del archivo final.");
	}

	archivoFinal->rutaArchivo = malloc(archivoFinal->tamanio);

	if (recv(fileDescriptor, archivoFinal->rutaArchivo, archivoFinal->tamanio, 0) == -1) {
		logErrorAndExit("Error al recibir la ruta del archivo final.");
	}

	if (recv(fileDescriptor, &archivoFinal->tamanioContenido, sizeof(archivoFinal->tamanioContenido), 0)
			== -1) {
		logErrorAndExit("Error al recibir el tamanio del contenido del archivo final");
	}

	archivoFinal->contenidoArchivo = malloc(archivoFinal->tamanioContenido);

	if (recv(fileDescriptor, archivoFinal->contenidoArchivo, archivoFinal->tamanioContenido, MSG_WAITALL) == -1) {
		logErrorAndExit("Error al recibir el contenido del archivo final");
	}
}
