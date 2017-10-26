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
	char ** nodos = config_get_array_value(tablaDeNodos, "NODOS");
	char * nodosConNodoAgregado = agregarNodoAArrayDeNodos(nodos, nuevoNodo->nombre);
	puts(nodosConNodoAgregado);
	puts("cargando Nodos");
	config_set_value(tablaDeNodos, "NODOS", nodosConNodoAgregado);

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
	liberarPunteroDePunterosAChar(nodos);
	free(nodos);
	free(bloquesLibresString);
	free(bloquesTotalString);
	free(nodosConNodoAgregado);
}

void eliminarNodoDeTablaDeNodos(Tnodo * nuevoNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//NODONTOTAL
	char * nodoTotalAString = string_new();
	string_append_with_format(&nodoTotalAString,"%sTotal", nuevoNodo->nombre);
	int nodoTotal = config_get_int_value(tablaDeNodos, nodoTotalAString);

	//TAMANIO
	int tamanio = config_get_int_value(tablaDeNodos, "TAMANIO");
	tamanio -= nodoTotal;
	char * tamanioString = string_itoa(tamanio);
	config_set_value(tablaDeNodos, "TAMANIO", tamanioString);

	//NODONLIBRE
	char * nodoLibreAString = string_new();
	string_append_with_format(&nodoLibreAString,"%sLibre", nuevoNodo->nombre);
	int nodoLibre = config_get_int_value(tablaDeNodos, nodoLibreAString);

	//LIBRE
	int libre = config_get_int_value(tablaDeNodos, "LIBRE");
	libre -= nodoLibre;
	char * libreString = string_itoa(libre);
	config_set_value(tablaDeNodos, "LIBRE", libreString);

	//SETEAR NODONTOTAL
	config_set_value(tablaDeNodos, nodoTotalAString, "");

	//SETEAR NODONLIBRE
	config_set_value(tablaDeNodos, nodoLibreAString, "");

	//NODOS
	char ** nodos = config_get_array_value(tablaDeNodos, "NODOS");
	char * nodosConNodoEliminado = eliminarNodoDelArrayDeNodos(nodos, nuevoNodo->nombre);
	config_set_value(tablaDeNodos, "NODOS", nodosConNodoEliminado);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	free(nodoTotalAString);
	free(nodoLibreAString);
	free(libreString);
	free(tamanioString);
	liberarPunteroDePunterosAChar(nodos);
	free(nodos);
	free(nodosConNodoEliminado);
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

char * eliminarNodoDelArrayDeNodos(char ** nodos, char * nombre){
	char * nuevoString = string_new();
	char * definitivo = string_new();
	int i = 0;
	string_append(&nuevoString, "[");
	while(nodos[i] != NULL){
		if(strcmp(nodos[i], nombre)){
			string_append(&nuevoString, nodos[i]);
			string_append(&nuevoString, ",");
		}
		i++;
	}
	definitivo = string_substring(nuevoString, 0, strlen(nuevoString)-1);
	string_append(&definitivo, "]");
	free(nuevoString);
	return definitivo;
}

char * agregarNodoAArrayDeNodos(char ** nodos, char * nombreNodo){
	puts("Estoy adentro de la funcion que appendea");
	char * nuevoString = string_new();
	int i = 0;
	string_append(&nuevoString, "[");
	puts("Voy a hacer el while");
	while(nodos[i] != NULL){
		string_append(&nuevoString, nodos[i]);
		string_append(&nuevoString, ",");
		i++;
	}
	string_append(&nuevoString, nombreNodo);
	string_append(&nuevoString, "]");
	puts("Por salir");
	return nuevoString;
}
