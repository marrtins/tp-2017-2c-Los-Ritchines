#include "../funcionesFS.h"

void inicializarTablaDeNodos(char * ruta){
	FILE * archivo = fopen(ruta, "wb");
	fclose(archivo);
	t_config * archivoNodos = config_create(ruta);
	config_set_value(archivoNodos, "TAMANIO", "0");
	config_set_value(archivoNodos, "LIBRE","0");
	config_set_value(archivoNodos, "NODOS", "[]");
	config_save(archivoNodos);
	config_destroy(archivoNodos);
}

void bajaDeNodoEnTablaDeNodos(Tnodo * nodo){
	eliminarNodoDeTablaDeNodos("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin", nodo);
	agregarNodoATablaDeNodos("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodosDesconectados.bin", nodo);
}

void altaDeNodoEnTablaDeNodos(Tnodo * nodo){
	agregarNodoATablaDeNodos("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin", nodo);
	eliminarNodoDeTablaDeNodos("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodosDesconectados.bin", nodo);

}

char * generarStringNodoNLibre(char * nombre){
	return string_from_format("%sLibre", nombre);
}

char * generarStringNodoNTotal(char * nombre){
	return string_from_format("%sTotal", nombre);
}

void inicializarListaDeNodosPorConectar(char * ruta, t_list * desconectados){
	FILE * archivoTablaDeNodos = fopen(ruta, "rb");
	char * mensaje;
	if(archivoTablaDeNodos == NULL){
		mensaje = malloc(250);
		sprintf(mensaje, "No se pudo levantar un estado anterior ya que no existe el archivo de la ruta: %s", ruta);
		log_trace(logger, mensaje);
		return;
	}
	fclose(archivoTablaDeNodos);
	t_config * archivo = config_create(ruta);
	char ** nodos = config_get_array_value(archivo, "NODOS");
	int i = 0;
	char * nodoNTotal;
	char * nodoNLibres;
	Tnodo * nodo;
	while (nodos[i] != NULL) {
		puts(nodos[i]);
		nodo = malloc(sizeof(Tnodo));
		nodo->nombre = strdup(nodos[i]);
		nodoNLibres = generarStringNodoNLibre(nodos[i]);
		nodo->cantidadBloquesLibres = config_get_int_value(archivo,
				nodoNLibres);
		nodoNTotal = generarStringNodoNTotal(nodos[i]);
		nodo->cantidadBloquesTotal = config_get_int_value(archivo, nodoNTotal);
		puts("creando bitmap");
		printf("%d", nodo->cantidadBloquesTotal);
		nodo->bitmap = crearBitmap(nodo->cantidadBloquesTotal);
		puts("levantando bitmap");
		levantarBitmapDeUnNodo(nodo);
		puts("bitmap");
		nodo->fd = -1;
		list_add(desconectados, nodo);
		free(nodoNLibres);
		free(nodoNTotal);
		i++;
	}
	liberarPunteroDePunterosAChar(nodos);
	free(nodos);
	config_destroy(archivo);
	puts("voy a mostrar");
	mostrarListaDeNodos(desconectados);
}

void levantarEstadoAnteriorDeLaTablaDeNodos(t_list * desconectados){
	inicializarListaDeNodosPorConectar("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin", desconectados);
	inicializarListaDeNodosPorConectar("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodosDesconectados.bin", desconectados);

}

void agregarNodoATablaDeNodos(char * ruta, Tnodo * nuevoNodo){
	t_config * tablaDeNodos = config_create(ruta);

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

void eliminarNodoDeTablaDeNodos(char * ruta, Tnodo * nuevoNodo){
	t_config * tablaDeNodos = config_create(ruta);

	//NODONTOTAL
	char * nodoTotalAString = string_new();
	string_append_with_format(&nodoTotalAString,"%sTotal", nuevoNodo->nombre);
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

	//NODOS
	eliminarElementoDeArrayArchivosConfig(tablaDeNodos, "NODOS", nuevoNodo->nombre);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	eliminarKeyDeArchivo("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin", nodoTotalAString);
	eliminarKeyDeArchivo("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin", nodoLibreAString);

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
