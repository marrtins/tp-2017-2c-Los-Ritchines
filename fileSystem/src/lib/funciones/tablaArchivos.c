#include "../funcionesFS.h"

void levantarTablaArchivo(Tarchivo * tablaArchivo, char * ruta){

	t_config *archivo = config_create(ruta);

	tablaArchivo->extensionArchivo = malloc(TAMANIO_EXTENSION_ARCHIVO);
	tablaArchivo->nombreArchivoSinExtension = malloc(100);

	int cantBloques, nroBloque = 0;
	int i = 0;
	int cantidadDeCopias;
	char **temporal;
	char* bloqueCopiaN;
	char* bloqueBytes;
	char * bloqueNCopias;
	char ** split = string_split(ruta, "/");
	char * nombreEntero = obtenerUltimoElementoDeUnSplit(split);
	char * nombreSinExtension = obtenerNombreDeArchivoSinExtension(nombreEntero);
	strcpy(tablaArchivo->nombreArchivoSinExtension, nombreSinExtension);

	tablaArchivo->tamanioTotal = config_get_int_value(archivo, "TAMANIO");
	strcpy(tablaArchivo->extensionArchivo, config_get_string_value(archivo, "TIPO"));

	cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal);
	tablaArchivo->bloques = malloc(sizeof(Tbloques)*cantBloques);

	while(nroBloque != cantBloques){
		bloqueNCopias = generarStringBloqueNCopias(nroBloque);
		cantidadDeCopias = config_get_int_value(archivo, bloqueNCopias);
		tablaArchivo->bloques[nroBloque].copia = list_create();
		bloqueBytes = generarStringDeBloqueNBytes(nroBloque);
		tablaArchivo->bloques[nroBloque].bytes = config_get_int_value(archivo, bloqueBytes);

		while(i < cantidadDeCopias){
			TcopiaNodo * copiaNodo = malloc(sizeof(TcopiaNodo));
			copiaNodo->nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);

			bloqueCopiaN = generarStringDeBloqueNCopiaN(nroBloque,i);

			temporal = config_get_array_value(archivo, bloqueCopiaN);

			strcpy(copiaNodo->nombreDeNodo,temporal[0]);
			copiaNodo->numeroBloqueDeNodo = atoi(temporal[1]);
			list_add(tablaArchivo->bloques[nroBloque].copia, copiaNodo);

			i++;
		}

		nroBloque++;

		liberarPunteroDePunterosAChar(temporal);
		free(temporal);
		free(bloqueNCopias);
		free(bloqueCopiaN);
		free(bloqueBytes);
	}
	liberarPunteroDePunterosAChar(split);
	free(split);
	free(nombreEntero);
	free(nombreSinExtension);

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

char * generarStringBloqueNCopias(int nroBloque){
	char * stringGenerado = malloc(TAMANIO_BLOQUE_N_COPIA_N);
	sprintf(stringGenerado, "BLOQUE%dCOPIAS", nroBloque);
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
	char * bloqueNCopiaM;
	char * bloqueNBytes;
	char * numeroBloqueEnString;
	char * tamanioTotal;
	char * bytes;
	int i = 0;
	int j = 0;
	int cantidadDeCopias;
	TcopiaNodo * copia;
	t_list * lista;

	tamanioTotal = string_itoa(archivoAAlmacenar->tamanioTotal);
	config_set_value(archivoConfig, "TAMANIO", tamanioTotal);
	config_set_value(archivoConfig, "TIPO", archivoAAlmacenar->extensionArchivo);

	free(tamanioTotal);

	while(i < cantidadDeBloques){
		cantidadDeCopias = list_size(archivoAAlmacenar->bloques[i].copia);
		lista = archivoAAlmacenar->bloques[i].copia;
		while(j < cantidadDeCopias){
			copia = (TcopiaNodo *) list_get(lista,j);

			bloqueNCopiaM = generarStringDeBloqueNCopiaN(i, copia->nroDeCopia);
			numeroBloqueEnString = string_itoa(copia->numeroBloqueDeNodo);
			agregarElementoAArrayArchivoConfig(archivoConfig, bloqueNCopiaM, copia->nombreDeNodo);
			agregarElementoAArrayArchivoConfig(archivoConfig, bloqueNCopiaM, numeroBloqueEnString);
			j++;
		}
		//aca hace el tamanio
		bloqueNBytes = generarStringDeBloqueNBytes(i);
		//hay que pasar un long a string TODO
		bytes = string_itoa(archivoAAlmacenar->bloques[i].bytes);
		config_set_value(archivoConfig, bloqueNBytes, bytes);

		i++;

		free(bytes);
		free(numeroBloqueEnString);
		free(bloqueNCopiaM);
		free(bloqueNBytes);
	}

	puts("guardando TODO");
	config_save(archivoConfig);
	puts("guardado.");
	config_destroy(archivoConfig);

}


void mostrarTablaArchivo(Tarchivo* tablaArchivo){

	int cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal);
	int i = 0;
	printf("TIPO = %s\n",tablaArchivo->extensionArchivo);
	printf("TAMANIO = %llu\n", tablaArchivo->tamanioTotal);

	while(i != cantBloques){
		printf("BLOQUE%dCOPIA0 = [%s, %d]\n", i, tablaArchivo->bloques[i].copiaCero.nombreDeNodo, tablaArchivo->bloques[i].copiaCero.numeroBloqueDeNodo);
		printf("BLOQUE%dCOPIA1 = [%s, %d]\n",i, tablaArchivo->bloques[i].copiaUno.nombreDeNodo, tablaArchivo->bloques[i].copiaUno.numeroBloqueDeNodo);
		printf("BLOQUE%dBYTES = %llu\n",i, tablaArchivo->bloques[i].bytes);
		i++;
	}

}

