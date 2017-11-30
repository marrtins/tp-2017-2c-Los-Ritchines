#include "../funcionesFS.h"

int bloquesOcupadosDeNodo(Tnodo * nodo){
	return nodo->cantidadBloquesTotal - nodo->cantidadBloquesLibres;
}

bool ordenarSegunBloquesDisponibles(void * nodo1, void * nodo2){
	Tnodo * nodoA = (Tnodo *) nodo1;
	Tnodo * nodoB = (Tnodo *) nodo2;

	return nodoA->cantidadBloquesLibres >= nodoB->cantidadBloquesLibres;
}

void mostrarListaDeNodos(t_list * lista){
	int i = 0;
	int cantidadNodos = lista->elements_count;
	Tnodo * nodo;
	printf("La lista contiene la siguiente informacion:\n");
	while(i != cantidadNodos){
		nodo = (Tnodo *) list_get(lista, i);
		printf("Nombre: %s\n", nodo->nombre);
		printf("Cantidad De Bloques Total: %d\n", nodo->cantidadBloquesTotal);
		printf("Cantidad De Bloques Libres: %d\n", nodo->cantidadBloquesLibres);
		printf("File Descriptor: %d\n", nodo->fd);
		printf("Bitmap: ");
		mostrarBitmap(nodo->bitmap);
		i++;
	}
}

int sumarBloquesLibresDeNodoSinElMaximo(Tnodo * maximo){
	int cantidad = listaDeNodos->elements_count;
	int i = 0;
	Tnodo * nodo;
	int sumador = 0;
	while(i != cantidad){
		nodo = (Tnodo*)list_get(listaDeNodos, i);
		if(nodo != maximo){
			sumador += nodo->cantidadBloquesLibres;
		}
		i++;
	}
	return sumador;
}

Tnodo * obtenerNodoPorTamanioMaximo(){
	int cantidad = listaDeNodos->elements_count;
	Tnodo * nodo = NULL;
	Tnodo * nodoMaximo = NULL;
	int maximo = 0;
	int i = 0;
	while(i != cantidad){
		nodo = (Tnodo * )list_get(listaDeNodos,i);
		if(nodo->cantidadBloquesLibres > maximo){
			nodoMaximo = nodo;
			maximo = nodo->cantidadBloquesLibres;
		}
		i++;
	}
	return nodoMaximo;
}

Tnodo * buscarNodoPorNombre(t_list * lista, char * nombre){

	bool buscarPorNombre(void * nodo){
		Tnodo * nodoA = (Tnodo *) nodo;
		return !strcmp(nombre, nodoA->nombre);
	}

	return (Tnodo *)list_find(lista, buscarPorNombre);
}

TinfoNodo * buscarInfoNodoPorNombre(t_list * lista, char * nombre){

	bool buscarPorNombre(void * nodo){
		TinfoNodo * nodoA = (TinfoNodo *) nodo;
		return !strcmp(nombre, nodoA->nombre);
	}

	return (TinfoNodo *)list_find(lista, buscarPorNombre);
}

int sumarListasPorTamanioDatabin(){
	int cantidadDeElementos = listaDeNodos->elements_count;
	int tamanioTotalDisponible = 0;
	int i = 0;
	Tnodo * nodo;
	while(cantidadDeElementos != 0){
		nodo = list_get(listaDeNodos, i);
		tamanioTotalDisponible += nodo->cantidadBloquesLibres;
		i++;
		cantidadDeElementos--;
	}
	return tamanioTotalDisponible;

}

void liberarNodo(Tnodo* nodo){
	free(nodo->nombre);
	bitarray_destroy(nodo->bitmap);
	free(nodo);
}

void liberarNodosDeLista(void * nodo){
	Tnodo * nodoA = (Tnodo *) nodo;
	liberarNodo(nodoA);
}

void* buscarNodoPorFD(t_list * lista, int fd){
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd == fd;
	}

	return list_find(lista, buscarPorFDParaLista);
}

void borrarNodoDesconectadoPorFD(int fd){
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd==fd;
	}
	list_remove_by_condition(listaDeNodosDesconectados,buscarPorFDParaLista);
}

void borrarNodoPorFD(int fd){
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd==fd;
	}
	list_remove_by_condition(listaDeNodos,buscarPorFDParaLista);
}

void borrarNodoPorNombre(t_list * lista, char * nombre){
	bool buscarPorNombreParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return !strcmp(nodo->nombre, nombre);
	}
	list_remove_by_condition(lista,buscarPorNombreParaLista);
}

bool ordenarListaPorMayor(void * directorio1, void * directorio2){
	Tdirectorio * directorioA = (Tdirectorio*)directorio1;
	Tdirectorio * directorioB = (Tdirectorio*)directorio2;

	int obtenerDirectorio(Tdirectorio *directorio) {
		return directorio->index;
	}
	return obtenerDirectorio(directorioA) >= obtenerDirectorio(directorioB);
}

int buscarIndexMayor(){
	list_sort(listaTablaDirectorios,ordenarListaPorMayor);
	Tdirectorio * directorio = (Tdirectorio*)list_get(listaTablaDirectorios,0);
	return directorio->index;
}

Tdirectorio* buscarPorNombreDeDirectorio(char * directorio){
	bool buscarPorNombreDeDirectorioParaLista(void* elementoDeLista){
		Tdirectorio* estructuraDirectorio = (Tdirectorio*) elementoDeLista;

		return !strcmp(directorio, estructuraDirectorio->nombre);
	}

	return (Tdirectorio*)list_find(listaTablaDirectorios, buscarPorNombreDeDirectorioParaLista);
}

Tdirectorio * buscarDirectorioPorIndice(int indice){
	bool buscarPorIndice(void * dir){
		Tdirectorio * directorio = (Tdirectorio*) dir;
		return directorio->index == indice;
	}

	return (Tdirectorio*)list_find(listaTablaDirectorios, buscarPorIndice);
}

void vaciarLista(){
	if(!list_is_empty(listaTablaDirectorios)){
		list_clean_and_destroy_elements(listaTablaDirectorios,liberarTablaDirectorios);
	}
}

Tnodo * buscarNodoDiponibleParaEnviar(t_list * listaDeNodos, int *indice){
	Tnodo* nodo;
	int indiceAnterior = *indice;
	if(*indice >= list_size(listaDeNodos)){
		*indice = 0;
		while(*indice < list_size(listaDeNodos)){
			nodo = list_get(listaDeNodos,*indice);
			if(nodo->cantidadBloquesLibres > 0){
				(*indice)++;
				return nodo;
			}
		}
		return NULL;
	}
	while(*indice <= list_size(listaDeNodos)){
		if(*indice >= list_size(listaDeNodos)){
			*indice = 0;
		}
		nodo = list_get(listaDeNodos,*indice);
		if(nodo->cantidadBloquesLibres > 0){
			(*indice)++;
			return nodo;
		}
		(*indice)++;
		if(*indice == indiceAnterior){
			break;
		}
	}
	return NULL;
	/*	bool esNodoDisponibleParaEnviar(void * nodo){
		Tnodo * nodoAEnviar = (Tnodo*) nodo;
		return nodoAEnviar->estadoParaEnviarBloque == 0 && nodoAEnviar->cantidadBloquesLibres > 0;
	}
	Tnodo * nodo = (Tnodo*)list_find(listaDeNodos, esNodoDisponibleParaEnviar);
	if(nodo != NULL){
		nodo->estadoParaEnviarBloque = 1;
	}
	return nodo;*/
}

void setearDisponibilidadDeEnvioDeNodos(t_list * listaDeNodos, int valor){
	int i = 0;
	Tnodo * nodo;
	while(i < list_size(listaDeNodos)){
		nodo = list_get(listaDeNodos, i);
		nodo->estadoParaEnviarBloque = valor;
		i++;
	}
}

t_list * buscarHijosDeUnDirectorio(Tdirectorio * padre){
	bool esHijo(void * posibleHijoVoid){
		Tdirectorio * posibleHijo = (Tdirectorio *) posibleHijoVoid;
		return posibleHijo->padre == padre->index;
	}
	return list_filter(listaTablaDirectorios, esHijo);

}

TelementoDeTablaArchivoGlobal * siNoExisteElNodoAgregar(char * nombreNodo, t_list * tablaDeArchivosGlobal){
	bool buscarPorNombre(void * nodo){
		TelementoDeTablaArchivoGlobal * nodoEncontrado = (TelementoDeTablaArchivoGlobal*) nodo;
		return !strcmp(nombreNodo, nodoEncontrado->nombreNodo);
	}
	TelementoDeTablaArchivoGlobal * nodoEncontrado = (TelementoDeTablaArchivoGlobal*)list_find(tablaDeArchivosGlobal, buscarPorNombre);
	if(nodoEncontrado == NULL){
		nodoEncontrado = malloc(sizeof(TelementoDeTablaArchivoGlobal));
		nodoEncontrado->nombreNodo = strdup(nombreNodo);
		nodoEncontrado->archivos = list_create();
		list_add(tablaDeArchivosGlobal, nodoEncontrado);
	}

	return nodoEncontrado;
}

TarchivoDeTablaArchivoGlobal * siNoExisteElArchivoAgregar(char * nombreArchivo, t_list * listaDeArchivos){
	bool buscarPorNombre(void * archivo) {
		TarchivoDeTablaArchivoGlobal * archivoEncontrado = (TarchivoDeTablaArchivoGlobal*) archivo;
		return !strcmp(nombreArchivo, archivoEncontrado->nombreArchivo);
	}
	TarchivoDeTablaArchivoGlobal * archivoEncontrado = (TarchivoDeTablaArchivoGlobal *) list_find(listaDeArchivos, buscarPorNombre);
	if (archivoEncontrado == NULL) {
		archivoEncontrado = malloc(sizeof(TarchivoDeTablaArchivoGlobal));
		archivoEncontrado->nombreArchivo = strdup(nombreArchivo);
		archivoEncontrado->bloques = list_create();
		list_add(listaDeArchivos, archivoEncontrado);
	}

	return archivoEncontrado;
}

void ordenarTablaDeArchivosGlobalPorNombre(t_list * tablaDeArchivosGlobal){
	bool ordenarPorNombre(void * nodo1, void * nodo2){
		TelementoDeTablaArchivoGlobal * nodoEncontrado1 = (TelementoDeTablaArchivoGlobal*) nodo1;
		TelementoDeTablaArchivoGlobal * nodoEncontrado2 = (TelementoDeTablaArchivoGlobal*) nodo2;
		return !strcmp(nodoEncontrado1->nombreNodo, nodoEncontrado2->nombreNodo);
	}
	list_sort(tablaDeArchivosGlobal, ordenarPorNombre);
}

void ordenarArchivosDeTablaDeArchivosGlobalPorNombre(t_list * archivos){
	bool ordenarPorNombre(void * archivo1, void * archivo2){
		TarchivoDeTablaArchivoGlobal * archivoEncontrado1 = (TarchivoDeTablaArchivoGlobal*) archivo1;
		TarchivoDeTablaArchivoGlobal * archivoEncontrado2 = (TarchivoDeTablaArchivoGlobal*) archivo2;
		return !strcmp(archivoEncontrado1->nombreArchivo, archivoEncontrado2->nombreArchivo);
	}
	list_sort(archivos, ordenarPorNombre);
}
