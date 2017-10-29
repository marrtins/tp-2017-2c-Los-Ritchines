#include "../funcionesFS.h"

void inicializarTablaDeNodos(){
	remove("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");
	FILE * archivo = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin", "wb");
	fclose(archivo);
	t_config * archivoNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");
	config_set_value(archivoNodos, "TAMANIO", "0");
	config_set_value(archivoNodos, "LIBRE","0");
	config_set_value(archivoNodos, "NODOS", "[]");
	config_save(archivoNodos);
	config_destroy(archivoNodos);
}

void agregarNodoATablaDeNodos(Tnodo * nuevoNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//TAMANIO
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, "TAMANIO", nuevoNodo->cantidadBloquesTotal, sumaDeDosNumerosInt);

	//LIBRE
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, "LIBRE", nuevoNodo->cantidadBloquesLibres, sumaDeDosNumerosInt);

	//NODOS
	agregarElementoAArrayArchivoConfig(tablaDeNodos, "NODOS", nuevoNodo->nombre);

	//agregar Nodos Dinamicamente
	char * nodoTotalAString = string_new();
	printf("nombreDeNodo %s", nuevoNodo->nombre);
	string_append_with_format(&nodoTotalAString,"%sTotal", nuevoNodo->nombre);
	char * bloquesTotalString = string_itoa(nuevoNodo->cantidadBloquesTotal);
	config_set_value(tablaDeNodos, nodoTotalAString, bloquesTotalString);

	char * nodoLibreAString = string_new();
	string_append_with_format(&nodoLibreAString,"%sLibre", nuevoNodo->nombre);
	char * bloquesLibresString = string_itoa(nuevoNodo->cantidadBloquesLibres);
	config_set_value(tablaDeNodos, nodoLibreAString, bloquesLibresString);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	free(nodoTotalAString);
	free(nodoLibreAString);
	free(bloquesLibresString);
	free(bloquesTotalString);
}

void eliminarNodoDeTablaDeNodos(Tnodo * nuevoNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//NODONTOTAL
	char * nodoTotalAString = string_new();
	puts("holi voy a appendear con format a nodototalastring");
	string_append_with_format(&nodoTotalAString,"%sTotal", nuevoNodo->nombre);
	puts("holi voy a obtener el get int value de la tabla de nodos con nodoTotalAString");
	puts(nodoTotalAString);
	int nodoTotal = config_get_int_value(tablaDeNodos, nodoTotalAString);

	//TAMANIO
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, "TAMANIO", nodoTotal, restaDeDosNumerosInt);

	//NODONLIBRE
	char * nodoLibreAString = string_new();
	string_append_with_format(&nodoLibreAString,"%sLibre", nuevoNodo->nombre);
	int nodoLibre = config_get_int_value(tablaDeNodos, nodoLibreAString);

	//LIBRE
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, "LIBRE", nodoLibre, restaDeDosNumerosInt);

	//SETEAR NODONTOTAL
	config_set_value(tablaDeNodos, nodoTotalAString, "");

	//SETEAR NODONLIBRE
	config_set_value(tablaDeNodos, nodoLibreAString, "");

	//NODOS
	eliminarElementoDeArrayArchivosConfig(tablaDeNodos, "NODOS", nuevoNodo->nombre);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	free(nodoTotalAString);
	free(nodoLibreAString);
}

void levantarTablaNodos(Tnodos * tablaNodos){

	int i = 0;
	int j = 0;
	char* nodoTotal = malloc(sizeof(char)*12);
	char* nodoLibre = malloc(sizeof(char)*12);


	t_config * archivoNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	tablaNodos->cantBloquesTotal = config_get_int_value(archivoNodos,"TAMANIO");
	tablaNodos->cantLibresTotal = config_get_int_value(archivoNodos,"LIBRE");

	printf("Cantidad bloques totales %d\n", tablaNodos->cantBloquesTotal);
	printf("Cantidad bloques libres %d\n", tablaNodos->cantLibresTotal);

	tablaNodos->nodos = config_get_array_value(archivoNodos,"NODOS");
	printf("nombre nodo 1: %s\n", tablaNodos->nodos[0]);
	printf("nombre nodo 2: %s\n", tablaNodos->nodos[1]);
	while(tablaNodos->nodos[j] != NULL){
		j++;
	}

	tablaNodos->nodoBloques = malloc(sizeof(TnodoBloque)*j);

	while(tablaNodos->nodos[i] != NULL){

		sprintf(nodoTotal,"Nodo%dTotal",i+1);
		sprintf(nodoLibre,"Nodo%dLibre",i+1);

		tablaNodos->nodoBloques[i].cantBloques = config_get_int_value(archivoNodos,nodoTotal);
		tablaNodos->nodoBloques[i].cantLibres = config_get_int_value(archivoNodos,nodoLibre);

		i++;
		}

	free(nodoTotal);
	free(nodoLibre);
	config_destroy(archivoNodos);
}

void agregarElementoAArrayArchivoConfig(t_config * tablaDeNodos, char * key, char * nombreElemento){
	char ** elementos = config_get_array_value(tablaDeNodos, key);
	char * nuevoString = string_new();
	int i = 0;
	string_append(&nuevoString, "[");
	while(elementos[i] != NULL){
		string_append(&nuevoString, elementos[i]);
		string_append(&nuevoString, ",");
		i++;
	}
	string_append(&nuevoString, nombreElemento);
	string_append(&nuevoString, "]");
	config_set_value(tablaDeNodos, key, nuevoString);
	free(nuevoString);
	liberarPunteroDePunterosAChar(elementos);
	free(elementos);
}
