#include "../funcionesFS.h"

void buscarLosDosNodosConMasDisponibilidad(t_list * lista, Tnodo * nodo1, Tnodo * nodo2){
	int cantidadElementos = list_size(lista);
	int i = 0;
	int maximo = 0;
	int maximo2 = 0;
	Tnodo * auxiliar = NULL;
	nodo1 = NULL;
	nodo2 = NULL;
	while(i != cantidadElementos){
		puts("voy a romper");
		auxiliar = (Tnodo * ) list_get(lista, i);
		puts("antes");
		printf("%p\n", auxiliar);
		printf("%p\n", nodo1);
		printf("%p\n", nodo2);
		if(auxiliar->cantidadBloquesLibres >= maximo){
			nodo2 = nodo1;
			nodo1 = auxiliar;
			maximo2 = maximo;
			maximo = auxiliar->cantidadBloquesLibres;
		}
		else if(auxiliar->cantidadBloquesLibres >= maximo2){
			nodo2 = auxiliar;
			maximo2 = auxiliar->cantidadBloquesLibres;
		}
		i++;
		printf("%p\n", auxiliar);
		printf("%p\n", nodo1);
		printf("%p\n", nodo2);
	}
	printf("nombre: %s\n", nodo1->nombre);
	printf("libres: %d\n", nodo1->cantidadBloquesLibres);
	printf("total: %d\n", nodo1->cantidadBloquesTotal);
	printf("fd: %d\n", nodo1->fd);
	printf("nombre: %s\n", nodo2->nombre);
	printf("libres: %d\n", nodo2->cantidadBloquesLibres);
	printf("total: %d\n", nodo2->cantidadBloquesTotal);
	printf("fd: %d\n", nodo2->fd);
}

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

void vaciarLista(){
	if(!list_is_empty(listaTablaDirectorios)){
	list_clean_and_destroy_elements(listaTablaDirectorios,liberarTablaDirectorios);
	}
}
