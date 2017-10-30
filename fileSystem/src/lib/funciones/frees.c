#include "../funcionesFS.h"

void liberarEstructuraBuffer(Tbuffer * buffer){
	free(buffer->buffer);
	free(buffer);
}

void liberarEstructuraBloquesAEnviar(TbloqueAEnviar * infoBloque){
	free(infoBloque->contenido);
	free(infoBloque);
}

void liberarTablaDeArchivo(Tarchivo * tablaDeArchivos){

	int i = 0;
	int cantBloques = cantidadDeBloquesDeUnArchivo(tablaDeArchivos->tamanioTotal);
	free(tablaDeArchivos->extensionArchivo);
	free(tablaDeArchivos->nombreArchivoSinExtension);

	for(; i != cantBloques; i++){
		free(tablaDeArchivos->bloques[i].copiaCero.nombreDeNodo);
		free(tablaDeArchivos->bloques[i].copiaUno.nombreDeNodo);
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
