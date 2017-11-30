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

void liberarBloques(void * bloque){
	TpackageUbicacionBloques * bloqueArchivo = (TpackageUbicacionBloques*) bloque;
	free(bloqueArchivo->nombreNodoC1);
	free(bloqueArchivo->nombreNodoC2);
	free(bloqueArchivo);
}

void liberarTInfoArchivoFSYama(TinfoArchivoFSYama * infoArchivo){
	list_destroy_and_destroy_elements(infoArchivo->listaBloques,liberarBloques);
	free(infoArchivo);
}

void liberarArchivosDeTablaDeArchivosGlobal(TarchivoDeTablaArchivoGlobal * archivo){
	free(archivo->nombreArchivo);
	list_destroy(archivo->bloques);
}

void liberarNodoDeTablaDeArchivosGlobal(TelementoDeTablaArchivoGlobal * nodo){
	free(nodo->nombreNodo);
	list_destroy_and_destroy_elements(nodo->archivos, free);
}

void liberarTablaDeArchivosGlobal(t_list * tablaDeArchivosGlobal){
	int i = 0;
	int j = 0;
	int cantidadNodos = list_size(tablaDeArchivosGlobal);
	int cantidadArchivos;
	TelementoDeTablaArchivoGlobal * nodo;
	TarchivoDeTablaArchivoGlobal * archivo;
	while(i < cantidadNodos){
		nodo = list_get(tablaDeArchivosGlobal, i);
		cantidadArchivos = list_size(nodo->archivos);
		j = 0;
		while(j < cantidadArchivos){
			archivo = list_get(nodo->archivos, j);
			liberarArchivosDeTablaDeArchivosGlobal(archivo);
			j++;
		}
		liberarNodoDeTablaDeArchivosGlobal(nodo);
		i++;
	}
	list_destroy_and_destroy_elements(tablaDeArchivosGlobal, free);
}


void liberarListaCircular(TlistaCircular * listaCircular){
	if(listaCircular == NULL){
		return;
	}
	TlistaCircular * ultimoElemento = ultimoElementoDeListaCircular(listaCircular);
	TlistaCircular * auxiliar = listaCircular;
	while(listaCircular != ultimoElemento){
		listaCircular = listaCircular->siguiente;
		free(auxiliar);
		auxiliar = listaCircular;
	}
	free(listaCircular);

	return;
}


