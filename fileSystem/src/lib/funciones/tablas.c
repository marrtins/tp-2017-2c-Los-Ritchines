#include "../funcionesFS.h"

void ocuparBloqueEnTablaArchivos(char * nombreNodo){
	t_config * tablaDeNodos = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");

	//LIBRE
	int libre = config_get_int_value(tablaDeNodos, "LIBRE");
	libre--;
	char * libreString = string_itoa(libre);
	config_set_value(tablaDeNodos, "LIBRE", libreString);

	char * nodoLibreAString = string_new();
	string_append_with_format(&nodoLibreAString,"%sLibre", nombreNodo);
	int nodoLibre = config_get_int_value(tablaDeNodos, nodoLibreAString);
	nodoLibre--;
	char * nodoLibreString = string_itoa(nodoLibre);
	config_set_value(tablaDeNodos, nodoLibreAString, nodoLibreString);

	config_save(tablaDeNodos);
	config_destroy(tablaDeNodos);

	free(nodoLibreAString);
	free(nodoLibreString);
	free(libreString);
}

void persistirTablaDeDirectorios(){
	int tamanio, i=0;
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "w");
	tamanio = list_size(listaTablaDirectorios);
	Tdirectorio * directorio = malloc(sizeof(Tdirectorio));

	while(tamanio != i){
		directorio = list_get(listaTablaDirectorios, i);
		fprintf(archivoDirectorios, "%d %s %d\n", directorio->index, directorio->nombre, directorio->padre);
		i++;
	}

	fclose(archivoDirectorios);

}

void inicializarTablaDeNodos(){
	puts("antes de borrar");
	remove("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");
	puts("despues de borrar");
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
	int tamanio = config_get_int_value(tablaDeNodos, "TAMANIO");
	tamanio += nuevoNodo->cantidadBloquesTotal;
	char * tamanioString = string_itoa(tamanio);
	config_set_value(tablaDeNodos, "TAMANIO", tamanioString);

	//LIBRE
	int libre = config_get_int_value(tablaDeNodos, "LIBRE");
	libre += nuevoNodo->cantidadBloquesLibres;
	char * libreString = string_itoa(libre);
	config_set_value(tablaDeNodos, "LIBRE", libreString);

	//NODOS
	char ** nodos = config_get_array_value(tablaDeNodos, "NODOS");
	char * nodosConNodoAgregado = agregarNodoAArrayDeNodos(nodos, nuevoNodo->nombre);
	puts(nodosConNodoAgregado);
	puts("cargando Nodos");
	config_set_value(tablaDeNodos, "NODOS", nodosConNodoAgregado);

	//agregar Nodos Dinamicamente
	char * nodoTotalAString = string_new();
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
	free(libreString);
	free(tamanioString);
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

void levantarTablasDirectorios(){
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "r");

	while(!feof(archivoDirectorios)){
		Tdirectorio * directorio = malloc(sizeof(Tdirectorio));
		fscanf(archivoDirectorios, "%d %s %d", &directorio->index, directorio->nombre, &directorio->padre);
		printf("%d \t %s \t %d \n", directorio->index, directorio->nombre, directorio->padre);
		list_add(listaTablaDirectorios, directorio);
	}

	fclose(archivoDirectorios);

}

void mostrarDirectorios(){

}

void levantarTablaArchivo(Tarchivo * tablaArchivos){

	t_config *archivo = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/2/archivo1.txt");

	tablaArchivos->extensionArchivo = malloc(TAMANIO_EXTENSION_ARCHIVO);


	int cantBloques,
			nroBloque = 0;
	char **temporal1;
	char **temporal2;
	char* bloqueCopia0;
	char* bloqueCopia1;
	char* bloqueBytes;

	tablaArchivos->tamanioTotal = config_get_int_value(archivo, "TAMANIO");
	strcpy(tablaArchivos->extensionArchivo, config_get_string_value(archivo, "TIPO"));

	cantBloques = cantidadDeBloquesDeUnArchivo(tablaArchivos->tamanioTotal);
	tablaArchivos->bloques = malloc(sizeof(Tbloques)*cantBloques);
	printf("cant bloques %d\n",cantBloques);

	printf("Tamanio %llu\n", tablaArchivos->tamanioTotal);
	printf("Extension %s\n", tablaArchivos->extensionArchivo);

	while(nroBloque != cantBloques){

		tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
		//tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo = malloc(sizeof(char)*4);
		tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo = malloc(TAMANIO_NOMBRE_NODO);
		//tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo = malloc(sizeof(char)*4);

		bloqueCopia0 = generarStringDeBloqueNCopiaN(nroBloque,0);
		bloqueCopia1 = generarStringDeBloqueNCopiaN(nroBloque,1);
		bloqueBytes = generarStringDeBloqueNBytes(nroBloque);

		temporal1 = config_get_array_value(archivo, bloqueCopia0);
		temporal2 = config_get_array_value(archivo, bloqueCopia1);
		tablaArchivos->bloques[nroBloque].bytes = config_get_int_value(archivo, bloqueBytes);

		strcpy(tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo,temporal1[0]);
		tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo = atoi(temporal1[1]);

		strcpy(tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo,temporal2[0]);
		tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo = atoi(temporal2[1]);

		printf("Nombre de nodo copia cero %s\n",tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo);
		printf("Numero de nodo copia cero %d\n",tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo);
		printf("Nombre de nodo copia uno %s\n",tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo);
		printf("Numero de nodo copia uno %d\n",tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo);
		printf("Bytes %llu\n",tablaArchivos->bloques[nroBloque].bytes);
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

int crearDirectorio(char * ruta) {
	int nroDirectorio, cant, index, indicePadre;
	char ** carpetas = string_split(ruta, "/");
	cant = contarPunteroDePunteros(carpetas);
	char* directorio = malloc(100);
	strcpy(directorio, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/");
	char * indice;
	Tdirectorio * tDirectorio = malloc(sizeof(Tdirectorio));

	if(list_size(listaTablaDirectorios)>=100){
		puts("Ya hay 100 directorios creados, no se puede crear el directorio");
		liberarPunteroDePunterosAChar(carpetas);
		free(carpetas);
		free(directorio);
		return -1;
	}
	if ((nroDirectorio = directorioNoExistente(carpetas)) < 0) {
		puts("No se puede crear el directorio");
		liberarPunteroDePunterosAChar(carpetas);
		free(carpetas);
		free(directorio);
		return -1;
	} else {
		if (nroDirectorio == cant - 1) {
			index = buscarIndexMayor() + 1;
			indicePadre = buscarIndexPorNombreDeDirectorio(carpetas[nroDirectorio - 1]);
			if(indicePadre == -1){
				indicePadre = 0;
			}
			printf("Indice padre del nuevo directorio %d\n", indicePadre);
			printf("Index asignado al nuevo directorio %d\n", index);

			indice = string_itoa(index);
			string_append(&directorio, indice);
			free(indice);
			//syscall(SYS_mkdir, directorio);
			mkdir(directorio,0777);

			printf("Directorio /%s creado\n", carpetas[nroDirectorio]);

			tDirectorio->index = index;
			strcpy(tDirectorio->nombre,carpetas[nroDirectorio]);
			tDirectorio->padre = indicePadre;

			list_add(listaTablaDirectorios,tDirectorio);
			liberarPunteroDePunterosAChar(carpetas);
			free(carpetas);
			free(directorio);
			return 0;
		}
			puts("No se puede crear directorio dentro de un directorio que no existe");
			liberarPunteroDePunterosAChar(carpetas);
			free(carpetas);
			free(directorio);
			return -1;
	}
}

void inicializarTablaDirectorios(){
		FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "w");
		list_clean(listaTablaDirectorios);
		fprintf(archivoDirectorios, "%d %s %d", 0, "root", -1);
		fclose(archivoDirectorios);
}

int directorioNoExistente(char ** carpetas) {
	int cant, indicePadre = 0, i = 0;
	char * yamafs = malloc(10);
	cant = contarPunteroDePunteros(carpetas);
	strcpy(yamafs,"yamafs:");
	if (string_equals_ignore_case(carpetas[i], yamafs)) {
			i++;
	for (i = 1; i < cant; i++) {
		Tdirectorio * estructuraDirectorio = (Tdirectorio*) buscarPorNombreDeDirectorio(carpetas[i]);
		if (estructuraDirectorio != NULL) {
			if (estructuraDirectorio->padre == indicePadre) {
				indicePadre = estructuraDirectorio->index;
			} else {
				free(yamafs);
				return i;
			}
		} else {
			free(yamafs);
			return i;
		}
	}
	free(yamafs);
	return -1;
	}else{
		puts("Falta la referencia al filesystem local 'yamafs:'");
		free(yamafs);
		return -1;
	}
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

void almacenarEstructuraArchivoEnUnArchivo(Tarchivo * archivoAAlmacenar, char * rutaArchivo){
	t_config * archivoConfig = config_create(rutaArchivo);
	int cantidadDeBloques = cantidadDeBloquesDeUnArchivo(archivoAAlmacenar->tamanioTotal);
	char * bloque0CopiaN;
	char * bloque1CopiaN;
	char * bloqueNBytes;
	char * concatenacionLoca;
	char * concatenacionLoca2;
	char * numeroBloqueEnString;
	char * numeroBloqueEnString2;
	int i = 0;

	config_set_value(archivoConfig, "TAMANIO", string_itoa(archivoAAlmacenar->tamanioTotal));
	config_set_value(archivoConfig, "TIPO", archivoAAlmacenar->extensionArchivo);

	while(cantidadDeBloques != 0){

		numeroBloqueEnString = malloc(4);
		numeroBloqueEnString = malloc(4);

		bloque0CopiaN = generarStringDeBloqueNCopiaN(i,0);
		numeroBloqueEnString = string_itoa(archivoAAlmacenar->bloques[i].copiaCero.numeroBloqueDeNodo);
		concatenacionLoca = generarArrayParaArchivoConfig(archivoAAlmacenar->bloques[i].copiaCero.nombreDeNodo, numeroBloqueEnString);

		config_set_value(archivoConfig, bloque0CopiaN, concatenacionLoca);

		bloque1CopiaN = generarStringDeBloqueNCopiaN(i,1);
		numeroBloqueEnString2 = string_itoa(archivoAAlmacenar->bloques[i].copiaUno.numeroBloqueDeNodo);
		concatenacionLoca2 = generarArrayParaArchivoConfig(archivoAAlmacenar->bloques[i].copiaUno.nombreDeNodo, numeroBloqueEnString2);
		config_set_value(archivoConfig, bloque1CopiaN, concatenacionLoca2);

		//aca hace el tamanio
		bloqueNBytes = generarStringDeBloqueNBytes(i);
		//hay que pasar un long a string
		config_set_value(archivoConfig, bloqueNBytes, string_itoa(archivoAAlmacenar->bloques[i].bytes));
		cantidadDeBloques--;
		i++;

		free(numeroBloqueEnString);
		free(numeroBloqueEnString2);
		free(bloque0CopiaN);
		free(bloque1CopiaN);
		free(bloqueNBytes);
		free(concatenacionLoca);
		free(concatenacionLoca2);
	}

	puts("guardando TODO");
	config_save(archivoConfig);
	puts("guardado.");
	config_destroy(archivoConfig);

}

char * generarArrayParaArchivoConfig(char * dato1, char * dato2){
	char * concatenacionLoca = malloc(1 + TAMANIO_NOMBRE_NODO + 1 + 3);
	string_append(&concatenacionLoca, "[");
	string_append(&concatenacionLoca, dato1);
	string_append(&concatenacionLoca, ",");
	string_append(&concatenacionLoca, dato2);
	string_append(&concatenacionLoca, "]");
	return concatenacionLoca;
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

int obtenerIndexDeUnaRuta(char * rutaDestino){
	puts("obteniendo index de una ruta");
	char ** palabras = string_split(rutaDestino, "/");
	char * directorio = obtenerUltimoElementoDeUnSplit(palabras);
	puts("index obtenido");
	return buscarIndexPorNombreDeDirectorio(directorio);

}

int buscarIndexPorNombreDeDirectorio(char * directorio){
	Tdirectorio * estructuraDirectorio = (Tdirectorio *)buscarPorNombreDeDirectorio(directorio);

	if(estructuraDirectorio != NULL){
		return estructuraDirectorio->index;
	}
	return -1;

}

int existeDirectorio(char * directorio){

	char ** carpetas = string_split(directorio, "/");
	char * yamafs = malloc(10);
	int i = 0;
	int indicePadre = 0;
	strcpy(yamafs,"yamafs:");
	if (string_equals_ignore_case(carpetas[i], yamafs)) {
		i++;
		while (carpetas[i] != NULL) {
			Tdirectorio * estructuraDirectorio = (Tdirectorio*) buscarPorNombreDeDirectorio(carpetas[i]);
			if (estructuraDirectorio != NULL) {
				if (estructuraDirectorio->padre == indicePadre) {
					indicePadre = estructuraDirectorio->index;
					i++;
				} else {
					liberarPunteroDePunterosAChar(carpetas);
					free(carpetas);
					free(yamafs);
					return 0;
				}
			} else {
				liberarPunteroDePunterosAChar(carpetas);
				free(carpetas);
				free(yamafs);
				return 0;
			}
		}
		liberarPunteroDePunterosAChar(carpetas);
		free(carpetas);
		free(yamafs);
		return 1;
	} else {
		liberarPunteroDePunterosAChar(carpetas);
		free(carpetas);
		free(yamafs);
		return 0;
	}

}
