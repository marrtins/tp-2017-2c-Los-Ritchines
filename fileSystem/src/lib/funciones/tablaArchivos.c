#include "../funcionesFS.h"

void levantarTablaArchivo(Tarchivo * tablaArchivo, char * ruta){

	t_config *archivo = config_create(ruta);

	tablaArchivo->extensionArchivo = malloc(TAMANIO_EXTENSION_ARCHIVO);

	int cantBloques, nroBloque = 0;
	char **temporal1;
	char **temporal2;
	char* bloqueCopia0;
	char* bloqueCopia1;
	char* bloqueBytes;

	tablaArchivo->tamanioTotal = config_get_int_value(archivo, "TAMANIO");
	strcpy(tablaArchivo->extensionArchivo, config_get_string_value(archivo, "TIPO"));

	cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal);
	tablaArchivo->bloques = malloc(sizeof(Tbloques)*cantBloques);

	while(nroBloque != cantBloques){

		tablaArchivo->bloques[nroBloque].copiaCero.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
		//tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo = malloc(sizeof(char)*4);
		tablaArchivo->bloques[nroBloque].copiaUno.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
		//tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo = malloc(sizeof(char)*4);

		bloqueCopia0 = generarStringDeBloqueNCopiaN(nroBloque,0);
		bloqueCopia1 = generarStringDeBloqueNCopiaN(nroBloque,1);
		bloqueBytes = generarStringDeBloqueNBytes(nroBloque);

		temporal1 = config_get_array_value(archivo, bloqueCopia0);
		temporal2 = config_get_array_value(archivo, bloqueCopia1);
		tablaArchivo->bloques[nroBloque].bytes = config_get_int_value(archivo, bloqueBytes);

		strcpy(tablaArchivo->bloques[nroBloque].copiaCero.nombreDeNodo,temporal1[0]);
		tablaArchivo->bloques[nroBloque].copiaCero.numeroBloqueDeNodo = atoi(temporal1[1]);

		strcpy(tablaArchivo->bloques[nroBloque].copiaUno.nombreDeNodo,temporal2[0]);
		tablaArchivo->bloques[nroBloque].copiaUno.numeroBloqueDeNodo = atoi(temporal2[1]);

		printf("Nombre de nodo copia cero %s\n",tablaArchivo->bloques[nroBloque].copiaCero.nombreDeNodo);
		printf("Numero de nodo copia cero %d\n",tablaArchivo->bloques[nroBloque].copiaCero.numeroBloqueDeNodo);
		printf("Nombre de nodo copia uno %s\n",tablaArchivo->bloques[nroBloque].copiaUno.nombreDeNodo);
		printf("Numero de nodo copia uno %d\n",tablaArchivo->bloques[nroBloque].copiaUno.numeroBloqueDeNodo);
		printf("Bytes %llu\n",tablaArchivo->bloques[nroBloque].bytes);
		nroBloque++;

		liberarPunteroDePunterosAChar(temporal1);
		free(temporal1);
		liberarPunteroDePunterosAChar(temporal2);
		free(temporal2);

		free(bloqueCopia0);
		free(bloqueCopia1);
		free(bloqueBytes);
	}

	config_destroy(archivo);
}

void ocuparBloqueEnTablaArchivos(char * nombreNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//LIBRE
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, "LIBRE", 1, restaDeDosNumerosInt);

	char * nodoLibreAString = string_new();
	string_append_with_format(&nodoLibreAString,"%sLibre", nombreNodo);
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, nodoLibreAString, 1, restaDeDosNumerosInt);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	free(nodoLibreAString);
}

char * generarStringDeBloqueNCopiaN(int numeroDeBloque, int numeroDeCopia){
	char * stringGenerado = malloc(TAMANIO_BLOQUE_N_COPIA_N);
	sprintf(stringGenerado, "BLOQUE%dCOPIA%d", numeroDeBloque, numeroDeCopia);
	return stringGenerado;
}

char * generarStringDeBloqueNBytes(int numeroDeBloque){
	char * stringGenerado = malloc(TAMANIO_BLOQUE_N_COPIA_N);
	sprintf(stringGenerado, "BLOQUE%dBYTES", numeroDeBloque);
	return stringGenerado;
}

void almacenarEstructuraArchivoEnUnArchivo(Tarchivo * archivoAAlmacenar, char * rutaArchivo){
	t_config * archivoConfig = config_create(rutaArchivo);
	int cantidadDeBloques = cantidadDeBloquesDeUnArchivo(archivoAAlmacenar->tamanioTotal);
	char * bloque0CopiaN;
	char * bloque1CopiaN;
	char * bloqueNBytes;
	char * numeroBloqueEnString;
	char * numeroBloqueEnString2;
	char * tamanioTotal;
	char * bytes;
	int i = 0;

	tamanioTotal = string_itoa(archivoAAlmacenar->tamanioTotal);
	config_set_value(archivoConfig, "TAMANIO", tamanioTotal);
	config_set_value(archivoConfig, "TIPO", archivoAAlmacenar->extensionArchivo);

	free(tamanioTotal);

	while(cantidadDeBloques != 0){

		bloque0CopiaN = generarStringDeBloqueNCopiaN(i,0);
		numeroBloqueEnString = string_itoa(archivoAAlmacenar->bloques[i].copiaCero.numeroBloqueDeNodo);
		generarArrayParaArchivoConfig(archivoConfig, bloque0CopiaN, archivoAAlmacenar->bloques[i].copiaCero.nombreDeNodo, numeroBloqueEnString);

		bloque1CopiaN = generarStringDeBloqueNCopiaN(i,1);
		numeroBloqueEnString2 = string_itoa(archivoAAlmacenar->bloques[i].copiaUno.numeroBloqueDeNodo);
		generarArrayParaArchivoConfig(archivoConfig, bloque1CopiaN, archivoAAlmacenar->bloques[i].copiaUno.nombreDeNodo, numeroBloqueEnString2);

		//aca hace el tamanio
		bloqueNBytes = generarStringDeBloqueNBytes(i);
		//hay que pasar un long a string
		bytes = string_itoa(archivoAAlmacenar->bloques[i].bytes);
		config_set_value(archivoConfig, bloqueNBytes, bytes);

		cantidadDeBloques--;
		i++;

		free(bytes);
		free(numeroBloqueEnString);
		free(numeroBloqueEnString2);
		free(bloque0CopiaN);
		free(bloque1CopiaN);
		free(bloqueNBytes);
	}

	puts("guardando TODO");
	config_save(archivoConfig);
	puts("guardado.");
	config_destroy(archivoConfig);

}


void mostrarTablaArchivo(Tarchivo* tablaArchivo){
	/*
	int cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal);
	int i = 0;
	printf("Nombre: %s.%s",tablaArchivo->nombreArchivoSinExtension,tablaArchivo->extensionArchivo);
	printf("Tamanio: %d", tablaArchivo->tamanioTotal);

	while(i != cantBloques){
		printf("Tamanio de bloque %d = %d\n",i, tablaArchivo->bloques[i].bytes);
		printf("Nombre del nodo donde esta la copia cero = %s",tablaArchivo->bloques[i].copiaCero.nombreDeNodo);
		printf("Número de bloque del nodo donde esta la copia cero = %d", tablaArchivo->bloques[i].copiaCero.numeroBloqueDeNodo);
		printf("Nombre del nodo donde esta la copia uno = %s",tablaArchivo->bloques[i].copiaUno.nombreDeNodo);
		printf("Número de bloque del nodo donde esta la copia uno = %d", tablaArchivo->bloques[i].copiaUno.numeroBloqueDeNodo);
		i++;
	}
	*/
}

