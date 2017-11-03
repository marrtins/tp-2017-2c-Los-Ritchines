#include "../funcionesFS.h"

void liberarEstructuraBuffer(Tbuffer * buffer){
	free(buffer->buffer);
	free(buffer);
}

void liberarEstructuraBloquesAEnviar(TbloqueAEnviar * infoBloque){
	free(infoBloque->contenido);
	free(infoBloque);
}

void liberarCopia(void * copia){

	TcopiaNodo * copiaNodo = (TcopiaNodo*) copia;
	free(copiaNodo->nombreDeNodo);
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
