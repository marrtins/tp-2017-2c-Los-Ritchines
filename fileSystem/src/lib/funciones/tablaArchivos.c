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

			liberarPunteroDePunterosAChar(temporal);
			free(temporal);
			free(bloqueNCopiaM);
			i++;
		}

		nroBloque++;
		free(bloqueNCopias);
		free(bloqueNBytes);
	}
	liberarPunteroDePunterosAChar(split);
	free(split);
	free(nombreEntero);
	free(nombreSinExtension);
	config_destroy(archivo);
}

int eliminarBloqueDeNodo(Tnodo * nodo, int numeroDeBloque){
	Tbuffer * buffer = malloc(sizeof(Tbuffer));
	Theader * head = malloc(sizeof(Theader));
	head->tipo_de_proceso = FILESYSTEM;
	head->tipo_de_mensaje = ELIMINAR_BLOQUE;

	empaquetarBloqueAEliminar(buffer, head, numeroDeBloque);

	if ((send(nodo->fd, buffer->buffer, buffer->tamanio, 0)) == -1) {
		log_error(logError, "No se pudo enviar informacion a datanode para que se borre un bloque.");
		free(head);
		liberarEstructuraBuffer(buffer);
		return 0;
	}

	return 1;
}

void pasarArrayDeUnaKeyAOtra(t_config * archivo, char * key1, char * key2){
	char ** array = config_get_array_value(archivo, key2);
	int i = 0;
	config_set_value(archivo, key1, "[]");
	while(array[i] != NULL){
		agregarElementoAArrayArchivoConfig(archivo, key1, array[i]);
		i++;
	}
	liberarPunteroDePunterosAChar(array);
	free(array);
}

void reordenarCopias(t_config * archivo, int numeroDeBloque, int numeroDeCopia){
	char * bloqueNCopiaN;
	char * bloqueNCopiaNSiguiente;
	char * bloqueNCopias = generarStringBloqueNCopias(numeroDeBloque);
	int cantidadDeCopias = config_get_int_value(archivo, bloqueNCopias);
	while(numeroDeCopia < cantidadDeCopias){
		bloqueNCopiaN = generarStringDeBloqueNCopiaN(numeroDeBloque, numeroDeCopia);
		bloqueNCopiaNSiguiente = generarStringDeBloqueNCopiaN(numeroDeBloque, numeroDeCopia + 1);
		pasarArrayDeUnaKeyAOtra(archivo, bloqueNCopiaN, bloqueNCopiaNSiguiente);
		free(bloqueNCopiaN);
		free(bloqueNCopiaNSiguiente);
		numeroDeCopia++;
	}
	free(bloqueNCopias);
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
	char * bloqueNCopiaFinal;

	if(numeroDeBloque > cantidadDeBloques-1){
		puts("El bloque que quiere eliminar, no existe en el archivo");
		log_error(logError, "El bloque a eliminar, no existe en el archivo");
		config_destroy(archivo);
		return;
	}

	bloqueNCopias = generarStringBloqueNCopias(numeroDeBloque);
	cantidadDeCopias = config_get_int_value(archivo, bloqueNCopias);

	if(cantidadDeCopias > 1){
		bloqueNCopiaN = generarStringDeBloqueNCopiaN(numeroDeBloque, numeroDeCopia);
		array = config_get_array_value(archivo, bloqueNCopiaN);
		nodo = buscarNodoPorNombre(listaDeNodos, array[0]);

		if(nodo == NULL){
			liberarPunteroDePunterosAChar(array);
			free(array);
			free(bloqueNCopias);
			free(bloqueNCopiaN);
			puts("El nodo aun no esta conectado, intentelo mas tarde.");
			config_destroy(archivo);
			return;
		}
		numeroDeBloqueEnNodo = atoi(array[1]);

		desocuparBloque(nodo, numeroDeBloqueEnNodo);

		setearAtributoDeArchivoConfigConInts(archivo, bloqueNCopias, 1, restaDeDosNumerosInt);

		reordenarCopias(archivo, numeroDeBloque, numeroDeCopia);

		config_save(archivo);
		config_destroy(archivo);

		bloqueNCopiaFinal = generarStringDeBloqueNCopiaN(numeroDeBloque, cantidadDeCopias - 1);
		eliminarKeyDeArchivo(rutaLocal, bloqueNCopiaFinal);
		free(bloqueNCopiaFinal);
		liberarPunteroDePunterosAChar(array);
		free(array);
		free(bloqueNCopias);
		free(bloqueNCopiaN);
		puts("Se elimino el bloque correctamente");

	}
	else{
		config_destroy(archivo);
		puts("Existe solo una copia del bloque especificado y por lo tanto no se puede eliminar");
	}


}

void ocuparBloqueEnTablaNodos(char * nombreNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//LIBRE
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, "LIBRE", 1, restaDeDosNumerosInt);

	char * nodoLibreAString = generarStringNodoNLibre(nombreNodo);
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, nodoLibreAString, 1, restaDeDosNumerosInt);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	free(nodoLibreAString);
}

void desocuparBloqueEnTablaDeNodos(char * nombreNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//LIBRE
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, "LIBRE", 1, sumaDeDosNumerosInt);

	char * nodoLibreAString = generarStringNodoNLibre(nombreNodo);
	setearAtributoDeArchivoConfigConInts(tablaDeNodos, nodoLibreAString, 1,	sumaDeDosNumerosInt);

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

	config_save(archivoConfig);
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

	bloqueNCopias = generarStringBloqueNCopias(nroBloque);

	cantidadDeCopias = config_get_int_value(archivo,bloqueNCopias);
	setearAtributoDeArchivoConfigConInts(archivo,bloqueNCopias,1,sumaDeDosNumerosInt);

	char * bloqueDN = string_itoa(bloqueDatabin);

	bloqueNCopiaM = generarStringDeBloqueNCopiaN(nroBloque,cantidadDeCopias);

	generarArrayParaArchivoConfig(archivo,bloqueNCopiaM,nodo,bloqueDN);

	config_save(archivo);
	config_destroy(archivo);
	free(bloqueNCopias);
	free(bloqueNCopiaM);
	free(bloqueDN);
}

char ** obtenerNodosDeUnArchivo(Tarchivo * archivo){
	TcopiaNodo * copia;
	int cantBloques, cantCopias, z = 0 ,j = 0, i = 0;
	cantBloques = cantidadDeBloquesDeUnArchivo(archivo->tamanioTotal);
	char ** nodos = calloc(10,sizeof(char*));

	while(i < cantBloques){
		cantCopias = archivo->bloques[i].cantidadCopias;
		j = 0;
		while(j < cantCopias){
			copia = list_get(archivo->bloques[i].copia,j);

			if(!punteroDeStringsContieneString(nodos,copia->nombreDeNodo)){
				nodos[z] = strdup(copia->nombreDeNodo);
				z++;
			}
			j++;
		}
		i++;
	}

	return nodos;
}

int todosLosBloquesTienenDosCopias(Tarchivo *  archivo){
	int cantBloques;
	int nroBloque = 0;
	cantBloques = cantidadDeBloquesDeUnArchivo(archivo->tamanioTotal);

	while(nroBloque < cantBloques){
		if(archivo->bloques[nroBloque].cantidadCopias!=2){
			//TODO agregar la copia que falta
			printf("El bloque numero %d del archivo %s no tiene copias\n", nroBloque, archivo->nombreArchivoSinExtension);
			puts("Agregar la copia que falta");
			log_info(logInfo,"El bloque numero %d del archivo %s no tiene copias\n", nroBloque, archivo->nombreArchivoSinExtension);
			return 0;
		}
		nroBloque++;
	}
	return 1;
}

int todosLosArchivosTienenCopias(){

	char ** directorios;
	char ** archivos;
	Tarchivo * archivo;
	int i = 0;
	int j;
	directorios = buscarDirectorios("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/");

	while (directorios[i] != NULL) {
		archivos = buscarArchivos(directorios[i]);
		j = 0;
		while (archivos[j] != NULL) {
			archivo = malloc(sizeof(Tarchivo));
			levantarTablaArchivo(archivo,archivos[j]);
			if(!todosLosBloquesTienenDosCopias(archivo)){
				liberarPunteroDePunterosAChar(directorios);
				free(directorios);
				liberarPunteroDePunterosAChar(archivos);
				free(archivos);
				liberarTablaDeArchivo(archivo);
				return 0;
			}
			liberarTablaDeArchivo(archivo);
			j++;
		}
		i++;
	}
	liberarPunteroDePunterosAChar(directorios);
	free(directorios);
	liberarPunteroDePunterosAChar(archivos);
	free(archivos);
	log_info(logInfo,"Todos los archivos tiene 2 copias");
	return 1;
}

