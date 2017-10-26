#include "../funcionesFS.h"

bool ordenarSegunBloquesDisponibles(void* nodo1, void* nodo2){
	Tnodo* nodoA = (Tnodo*)nodo1;
	Tnodo* nodoB = (Tnodo*)nodo2;

	double obtenerProporcionDeDisponibilidad(Tnodo* nodo){
		if(nodo->cantidadBloquesLibres == 0) return 1;
		double bloquesOcupados = nodo->cantidadBloquesTotal - nodo->cantidadBloquesLibres;
		return bloquesOcupados / nodo->cantidadBloquesTotal;
	}
	return obtenerProporcionDeDisponibilidad(nodoA) < obtenerProporcionDeDisponibilidad(nodoB);
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
	printf("La cantidad de MB disponibles en los datanodes es: %d", tamanioTotalDisponible);
	return tamanioTotalDisponible;

}

void liberarNodoDeLaListaGlobal(Tnodo* nodo){
	free(nodo->nombre);
	bitarray_destroy(nodo->bitmap);
	free(nodo);
}

void* buscarNodoPorFD(int fd){
	bool buscarPorFDParaLista(void* elementoDeLista){
		Tnodo* nodo = (Tnodo*) elementoDeLista;
		return nodo->fd == fd;
	}

	return list_find(listaDeNodos, buscarPorFDParaLista);
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

void* buscarPorNombreDeDirectorio(char * directorio){
	bool buscarPorNombreDeDirectorioParaLista(void* elementoDeLista){
		Tdirectorio* estructuraDirectorio = (Tdirectorio*) elementoDeLista;

		return !strcmp(directorio, estructuraDirectorio->nombre);
	}

	return list_find(listaTablaDirectorios, buscarPorNombreDeDirectorioParaLista);
}

