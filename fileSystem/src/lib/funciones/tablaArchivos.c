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
	char* bloqueNCopiaM;
	char* bloqueNBytes;
	char ** split = string_split(ruta, "/");
	char * nombreEntero = obtenerUltimoElementoDeUnSplit(split);
	char * nombreSinExtension = obtenerNombreDeArchivoSinExtension(nombreEntero);
	strcpy(tablaArchivo->nombreArchivoSinExtension, nombreSinExtension);

	tablaArchivo->tamanioTotal = config_get_int_value(archivo, "TAMANIO");
	strcpy(tablaArchivo->extensionArchivo, config_get_string_value(archivo, "TIPO"));

	cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal);
	tablaArchivo->bloques = malloc(sizeof(Tbloques)*cantBloques);

	while(nroBloque < cantBloques){
		tablaArchivo->bloques[nroBloque].copia = list_create();
		bloqueNCopias = generarStringBloqueNCopias(nroBloque);
		cantidadDeCopias = config_get_int_value(archivo, bloqueNCopias);
		bloqueNBytes = generarStringDeBloqueNBytes(nroBloque);
		tablaArchivo->bloques[nroBloque].bytes = config_get_int_value(archivo, bloqueNBytes);
		tablaArchivo->bloques[nroBloque].cantidadCopias = cantidadDeCopias;
		i = 0;
		while(i < cantidadDeCopias){
			TcopiaNodo * copiaNodo = malloc(sizeof(TcopiaNodo));
			copiaNodo->nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
			bloqueNCopiaM = generarStringDeBloqueNCopiaN(nroBloque,i);

			temporal = config_get_array_value(archivo, bloqueNCopiaM);
			//probando
			copiaNodo->nroDeCopia = i;

			strcpy(copiaNodo->nombreDeNodo,temporal[0]);
			copiaNodo->numeroBloqueDeNodo = atoi(temporal[1]);
			list_add(tablaArchivo->bloques[nroBloque].copia, copiaNodo);

			free(bloqueNCopiaM);
			i++;
		}

		nroBloque++;
		liberarPunteroDePunterosAChar(temporal);
		free(temporal);
		free(bloqueNCopias);
		free(bloqueNBytes);
	}
	liberarPunteroDePunterosAChar(split);
	free(split);
	free(nombreEntero);
	free(nombreSinExtension);
	config_destroy(archivo);
}

void eliminarBloqueDeTablaDeArchivos(t_config * archivo, int numeroDeBloque, int numeroDeCopia){
	char * bloqueNCopiaN;
	char * bloqueNCopias;
	bloqueNCopiaN = generarStringDeBloqueNCopiaN(numeroDeBloque, numeroDeCopia);
	puts(bloqueNCopiaN);
	config_set_value(archivo, bloqueNCopiaN, "[]");
	bloqueNCopias = generarStringBloqueNCopias(numeroDeBloque);
	setearAtributoDeArchivoConfigConInts(archivo, bloqueNCopias, 1, restaDeDosNumerosInt);
	free(bloqueNCopiaN);
	free(bloqueNCopias);
}

int eliminarBloqueDeNodo(Tnodo * nodo, int numeroDeBloque){
	Tbuffer * buffer = malloc(sizeof(Tbuffer));
	Theader * head = malloc(sizeof(Theader));
	head->tipo_de_proceso = FILESYSTEM;
	head->tipo_de_mensaje = ELIMINAR_BLOQUE;

	empaquetarBloqueAEliminar(buffer, head, numeroDeBloque);

	if ((send(nodo->fd, buffer->buffer, buffer->tamanio, 0)) == -1) {
		puts("No se pudo comunicar con datanode, para que elimine el nodo.");
		free(head);
		liberarEstructuraBuffer(buffer);
		return 0;
	}

	puts("Bloque eliminado");
	return 1;
}

void eliminarBloqueDeUnArchivo(char * rutaLocal, int numeroDeBloque, int numeroDeCopia){
	t_config * archivo = config_create(rutaLocal);
	unsigned long long tamanio = config_get_int_value(archivo, "TAMANIO");
	int cantidadDeBloques = cantidadDeBloquesDeUnArchivo(tamanio);
	char ** array;
	Tnodo * nodo;
	char * bloqueNCopiaN;
	char * bloqueNCopias;
	int cantidadDeCopias;
	int numeroDeBloqueEnNodo;

	if(numeroDeBloque > cantidadDeBloques-1){
		puts("El bloque que quiere eliminar, no existe en el archivo");
		config_destroy(archivo);
		return;
	}

	bloqueNCopias = generarStringBloqueNCopias(numeroDeBloque);
	cantidadDeCopias = config_get_int_value(archivo, bloqueNCopias);
	free(bloqueNCopias);

	if(cantidadDeCopias > 1){
		bloqueNCopiaN = generarStringDeBloqueNCopiaN(numeroDeBloque, numeroDeCopia);
		array = config_get_array_value(archivo, bloqueNCopiaN);
		free(bloqueNCopiaN);
		nodo = buscarNodoPorNombre(listaDeNodos, array[0]);

		if(nodo == NULL){
			liberarPunteroDePunterosAChar(array);
			free(array);
			puts("El nodo aun no esta conectado, intentelo mas tarde.");
			config_destroy(archivo);
			return;
		}
		numeroDeBloqueEnNodo = atoi(array[1]);

		/*if(!eliminarBloqueDeNodo(nodo, numeroDeBloqueEnNodo)){
			config_destroy(archivo);
			liberarPunteroDePunterosAChar(array);
			free(array);
			return;
		}*/

		desocuparBloqueEnBitmap(nodo, numeroDeBloque);
		mostrarBitmap(nodo->bitmap);

		eliminarBloqueDeTablaDeArchivos(archivo, numeroDeBloque, numeroDeCopia);

		config_save(archivo);
		config_destroy(archivo);

		liberarPunteroDePunterosAChar(array);
		free(array);

	}
	else{
		config_destroy(archivo);
		puts("Existe solo una copia del bloque especificado y por lo tanto no se puede eliminar");
	}


}

void ocuparBloqueEnTablaArchivos(char * nombreNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//LIBRE
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, "LIBRE", 1, restaDeDosNumerosInt);

	char * nodoLibreAString = generarStringNodoNLibre(nombreNodo);
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, nodoLibreAString, 1, restaDeDosNumerosInt);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	free(nodoLibreAString);
}

void desocuparBloqueEnTablaDeArchivo(char * nombreNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//LIBRE
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, "LIBRE", 1, sumaDeDosNumerosInt);

	char * nodoLibreAString = generarStringNodoNLibre(nombreNodo);
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, nodoLibreAString, 1,	restaDeDosNumerosInt);

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
		j=0;
		while(j < cantCopias){
			copia = list_get(tablaArchivo->bloques[i].copia,j);
			printf("BLOQUE%dCOPIA%d = [%s, %d]\n", i, copia->nroDeCopia, copia->nombreDeNodo,copia->numeroBloqueDeNodo);
			j++;
		}
		printf("BLOQUE%dBYTES = %llu\n",i, tablaArchivo->bloques[i].bytes);
		i++;
	}

}

void agregarCopiaAtablaArchivo(char * rutaLocalArchivo,char * nodo, int bloqueDatabin, int nroBloque){
	char * bloqueNCopias;
	char * bloqueNCopiaM;
	int cantidadDeCopias;

	puts(rutaLocalArchivo);
	t_config *archivo = config_create(rutaLocalArchivo);
	puts("Voy a agregar copia a tabla archivo");

	bloqueNCopias = generarStringBloqueNCopias(nroBloque);
	puts(bloqueNCopias);
	puts("paso generar string bloqueNCopias");
	cantidadDeCopias = config_get_int_value(archivo,bloqueNCopias);
	printf("cant de copias obtenidas antes de setear %d\n",cantidadDeCopias);
	setearAtributoDeArchivoConfigConInts(archivo,bloqueNCopias,1,sumaDeDosNumerosInt);
	puts("seteado el atributo bloqueNCOPIAS");

	char * bloqueDN = string_itoa(bloqueDatabin);
puts("Aca no rompe");
	bloqueNCopiaM = generarStringDeBloqueNCopiaN(nroBloque,cantidadDeCopias);
	puts("genero bloqeNcopiaM");
	generarArrayParaArchivoConfig(archivo,bloqueNCopiaM,nodo,bloqueDN);
	puts("ya esta hecho");
	config_save(archivo);
	config_destroy(archivo);
	free(bloqueNCopias);
	free(bloqueNCopiaM);
	free(bloqueDN);
}
