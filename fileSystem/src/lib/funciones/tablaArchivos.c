#include "../funcionesFS.h"

void levantarTablaArchivo(Tarchivo * tablaArchivo, char * ruta){

	t_config *archivo = config_create(ruta);

	tablaArchivo->extensionArchivo = malloc(TAMANIO_EXTENSION_ARCHIVO);
	tablaArchivo->nombreArchivoSinExtension = malloc(100);

	int cantBloques, nroBloque = 0;
	int i;
	int cantidadDeCopias;
	char **temporal;
	char * bloqueNCopias;
	char* bloqueCopiaN;
	char* bloqueBytes;
	char ** split = string_split(ruta, "/");
	char * nombreEntero = obtenerUltimoElementoDeUnSplit(split);
	char * nombreSinExtension = obtenerNombreDeArchivoSinExtension(nombreEntero);
	strcpy(tablaArchivo->nombreArchivoSinExtension, nombreSinExtension);

	tablaArchivo->tamanioTotal = config_get_int_value(archivo, "TAMANIO");
	strcpy(tablaArchivo->extensionArchivo, config_get_string_value(archivo, "TIPO"));

	cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal);
	tablaArchivo->bloques = malloc(sizeof(Tbloques)*cantBloques);

	while(nroBloque < cantBloques){
		bloqueNCopias = generarStringBloqueNCopias(nroBloque);
		cantidadDeCopias = config_get_int_value(archivo, bloqueNCopias);
		tablaArchivo->bloques[nroBloque].copia = list_create();
		bloqueBytes = generarStringDeBloqueNBytes(nroBloque);
		tablaArchivo->bloques[nroBloque].bytes = config_get_int_value(archivo, bloqueBytes);
		tablaArchivo->bloques[nroBloque].cantidadCopias = cantidadDeCopias;
		i = 0;
		while(i < cantidadDeCopias){
			TcopiaNodo * copiaNodo = malloc(sizeof(TcopiaNodo));
			copiaNodo->nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
			bloqueCopiaN = generarStringDeBloqueNCopiaN(nroBloque,i);

			temporal = config_get_array_value(archivo, bloqueCopiaN);

			strcpy(copiaNodo->nombreDeNodo,temporal[0]);
			copiaNodo->numeroBloqueDeNodo = atoi(temporal[1]);
			list_add(tablaArchivo->bloques[nroBloque].copia, copiaNodo);

			free(bloqueCopiaN);
			i++;
		}

		nroBloque++;
		liberarPunteroDePunterosAChar(temporal);
		free(temporal);
		free(bloqueNCopias);
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
	char * bloqueNCopia;
	char * numeroBloqueEnString;
	char * tamanioTotal;
	char * bytes;
	char * cantidadDeCopiasString;
	int i = 0;
	int j;
	int cantidadDeCopias;
	TcopiaNodo * copia;
	t_list * lista;

	tamanioTotal = string_itoa(archivoAAlmacenar->tamanioTotal);
	config_set_value(archivoConfig, "TAMANIO", tamanioTotal);
	config_set_value(archivoConfig, "TIPO", archivoAAlmacenar->extensionArchivo);

	free(tamanioTotal);

	while(i < cantidadDeBloques){
		lista = archivoAAlmacenar->bloques[i].copia;
		bloqueNCopia = generarStringBloqueNCopias(i);
		cantidadDeCopias = list_size(lista);
		cantidadDeCopiasString = string_itoa(cantidadDeCopias);
		config_set_value(archivoConfig, bloqueNCopia, cantidadDeCopiasString);
		j = 0;
		while(j < cantidadDeCopias){
			copia = (TcopiaNodo *) list_get(lista,j);

			bloqueNCopiaM = generarStringDeBloqueNCopiaN(i, copia->nroDeCopia);
			numeroBloqueEnString = string_itoa(copia->numeroBloqueDeNodo);
			generarArrayParaArchivoConfig(archivoConfig, bloqueNCopiaM, copia->nombreDeNodo, numeroBloqueEnString);
			free(bloqueNCopiaM);
			free(numeroBloqueEnString);
			j++;
		}
		//aca hace el tamanio
		bloqueNBytes = generarStringDeBloqueNBytes(i);
		//hay que pasar un long a string TODO
		bytes = string_itoa(archivoAAlmacenar->bloques[i].bytes);
		config_set_value(archivoConfig, bloqueNBytes, bytes);

		i++;

		free(bytes);
		free(bloqueNBytes);
		free(bloqueNCopia);
		free(cantidadDeCopiasString);
	}

	puts("guardando TODO");
	config_save(archivoConfig);
	puts("guardado.");
	config_destroy(archivoConfig);

}


void mostrarTablaArchivo(Tarchivo* tablaArchivo){

	int cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal);
	TcopiaNodo * copia;
	int cantCopias;
	int i = 0, j = 0;
	printf("TIPO = %s\n",tablaArchivo->extensionArchivo);
	printf("TAMANIO = %llu\n", tablaArchivo->tamanioTotal);

	while(i < cantBloques){
		cantCopias = tablaArchivo->bloques[i].cantidadCopias;
		while(j < cantCopias){
			copia = list_get(tablaArchivo->bloques[i].copia,j);
			printf("BLOQUE%dCOPIA%d = [%s, %d]\n", i, copia->nroDeCopia, copia->nombreDeNodo,copia->numeroBloqueDeNodo);

		}
		printf("BLOQUE%dBYTES = %llu\n",i, tablaArchivo->bloques[i].bytes);
		i++;
	}

}

