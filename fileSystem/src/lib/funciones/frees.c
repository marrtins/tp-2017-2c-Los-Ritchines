#include "../funcionesFS.h"

void liberarEstructuraBuffer(Tbuffer * buffer){
	free(buffer->buffer);
	free(buffer);
}

void liberarEstructuraArchivoFinal(TarchivoFinal * archivoFinal){
	free(archivoFinal->rutaArchivo);
	free(archivoFinal->contenidoArchivo);
	free(archivoFinal);
}

void liberarEstructuraBloquesAEnviar(TbloqueAEnviar * infoBloque){
	free(infoBloque->contenido);
	free(infoBloque);
}

void liberarCopia(void * copia){

	TcopiaNodo * copiaNodo = (TcopiaNodo*) copia;
	free(copiaNodo->nombreDeNodo);
	free(copiaNodo);
}

void liberarTablaDeArchivo(Tarchivo * tablaDeArchivos){

	int i = 0;
	int cantBloques = cantidadDeBloquesDeUnArchivo(tablaDeArchivos->tamanioTotal);
	free(tablaDeArchivos->extensionArchivo);
	free(tablaDeArchivos->nombreArchivoSinExtension);

	for(; i < cantBloques; i++){
		list_destroy_and_destroy_elements(tablaDeArchivos->bloques[i].copia,liberarCopia);
	}

	free(tablaDeArchivos->bloques);
	free(tablaDeArchivos);
}

void liberarTPackInfoBloqueDN(TpackInfoBloqueDN * bloque){
	free(bloque->ipNodo);
	free(bloque->nombreNodo);
	free(bloque->puertoNodo);
	free(bloque);
}

void liberarTablaDirectorios(void * directorio){
	free(directorio);
}

void liberarTPackageInfoNodo (void * infoNodo){
	TpackageInfoNodo * nodo = (TpackageInfoNodo *) infoNodo;
	free(nodo->nombreNodo);
	free(nodo->ipNodo);
	free(nodo->puertoWorker);
	free(nodo);

}

void liberarTpackageUbicacionBloques(void * bloque){
	TpackageUbicacionBloques * bloqueA = (TpackageUbicacionBloques *) bloque;
	free(bloqueA->nombreNodoC1);
	free(bloqueA->nombreNodoC2);
	free(bloqueA);
}
