#include "../funcionesFS.h"

void persistirTablaDeDirectorios(){
	int tamanio, i=0;
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "w");
	tamanio = list_size(listaTablaDirectorios);
	Tdirectorio * directorio;

	while(tamanio != i){
		directorio = list_get(listaTablaDirectorios, i);
		fprintf(archivoDirectorios, "%d %s %d\n", directorio->index, directorio->nombre, directorio->padre);
		i++;
	}

	fclose(archivoDirectorios);
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
		log_trace(logger, "Ya exiten 100 directorios, no se pudo crear otro.");
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
			log_trace(logger, "No se pudo crear un directorio dentro de un directorio que no existe");
			return -1;
	}
}

void inicializarTablaDirectorios(){
		char * ruta = malloc(100);
		FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "w");

		strcpy(ruta,"/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/");

		list_clean(listaTablaDirectorios);
		fprintf(archivoDirectorios, "%d %s %d", 0, "root", -1);
		mkdir("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/",0777);
		fclose(archivoDirectorios);
		removerDirectorios(ruta);
		free(ruta);
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
		log_trace(logger, "No se pudo crear el directorio por que no existe la referencia 'yamafs:'");
		free(yamafs);
		return -1;
	}
}

int obtenerIndexDeUnaRuta(char * rutaDestino){
	int indice;
	char ** palabras = string_split(rutaDestino, "/");
	char * directorio = obtenerUltimoElementoDeUnSplit(palabras);
	liberarPunteroDePunterosAChar(palabras);
	free(palabras);
	indice = buscarIndexPorNombreDeDirectorio(directorio);
	free(directorio);
	return indice;
}

int buscarIndexPorNombreDeDirectorio(char * directorio){
	Tdirectorio * estructuraDirectorio = (Tdirectorio *)buscarPorNombreDeDirectorio(directorio);

	if(estructuraDirectorio != NULL){
		return estructuraDirectorio->index;
	}
	log_trace(logger, "No existe el nombre de directorio.");
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

int esDirectorio(char * ruta){
	struct stat estado;
	int i;

	stat(ruta,&estado);
	i = S_ISDIR(estado.st_mode);


	return i;
}

int esArchivo (char* ruta){
	struct stat estado;
	int i;

	stat(ruta,&estado);
	i= S_ISREG(estado.st_mode);

	return i;
}

char** buscarDirectorios(char * ruta){
	
	  DIR *directorioActual;
	  struct dirent *directorio;
	  char ** directorios = malloc(100);
	  char * rutaNueva;
	  int i = 0;

	  directorioActual = opendir (ruta);

	  if (directorioActual == NULL){
	    puts("No puedo abrir el directorio");

	  }else{
	  // Leo uno por uno los directorios que estan adentro del directorio actual
	  while ((directorio = readdir(directorioActual)) != NULL) {

		  //Con readdir aparece siempre . y .. como no me interesa no lo contemplo
		if ((strcmp(directorio->d_name, ".") != 0) && (strcmp(directorio->d_name, "..") != 0)) {


			rutaNueva = string_duplicate(ruta);
			string_append(&rutaNueva,directorio->d_name);

			directorios[i] = malloc(256);
			if(esDirectorio(rutaNueva)){
				strcpy(directorios[i],rutaNueva);
				i++;
			}
			free(rutaNueva);
		}

	}


	  closedir (directorioActual);
}
	  directorios[i] = NULL;
	  return directorios;
}

char** buscarArchivos(char * ruta){

	  DIR *directorioActual;
	  struct dirent *archivo;
	  char ** archivos = malloc(100);
	  char * rutaNueva;
	  int i = 0;

	  directorioActual = opendir (ruta);

	  if (directorioActual == NULL){
	    puts("No puedo abrir el directorio");

	  }else{
	  // Leo uno por uno los archivos que estan adentro del directorio actual
	  while ((archivo = readdir(directorioActual)) != NULL) {

		  //Con readdir aparece siempre . y .. como no me interesa no lo contemplo
		if ((strcmp(archivo->d_name, ".") != 0) && (strcmp(archivo->d_name, "..") != 0)) {

			rutaNueva = string_duplicate(ruta);
			string_append(&rutaNueva,"/");
			string_append(&rutaNueva,archivo->d_name);
			archivos[i] = malloc(256);
				if(esArchivo(rutaNueva)){
					strcpy(archivos[i],rutaNueva);
					i++;
				}
			free(rutaNueva);

		}
	  }
	  closedir (directorioActual);
	  }

	  archivos[i] = NULL;

	  return archivos;
}

void removerArchivos(char * ruta){
	char ** archivos;
	int i = 0;
	archivos = buscarArchivos(ruta);

	if(archivos[i] != NULL){
		while(archivos[i] != NULL){

			remove(archivos[i]);
			i++;

		}
		liberarPunteroDePunterosAChar(archivos);
		free(archivos);
	}else{
	free(archivos);
	}
}

void removerDirectorios(char *ruta){
	char ** directorios;
	int i = 0;

	directorios = buscarDirectorios(ruta);

	if(directorios[i] != NULL){

		while(directorios[i] != NULL){

			removerArchivos(directorios[i]);
			rmdir(directorios[i]);
			i++;

		}
		liberarPunteroDePunterosAChar(directorios);
		free(directorios);
	}else {

	free(directorios);
	}

}


void listarArchivos(char* ruta){

	char ** archivos;
	int i = 0, index;
	char * rutaArchivosDirectorio = malloc(200);
	char ** divisionRuta;
	char * nombreArchivoConExtension;
	index = obtenerIndexDeUnaRuta(ruta);
	sprintf(rutaArchivosDirectorio, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/%d", index);
	archivos = buscarArchivos(rutaArchivosDirectorio);

	if(archivos[i] != NULL){
		while(archivos[i] != NULL){

				divisionRuta = string_split(archivos[i], "/");
				nombreArchivoConExtension = obtenerUltimoElementoDeUnSplit(divisionRuta);
				puts(nombreArchivoConExtension);
				i++;

			}
			liberarPunteroDePunterosAChar(archivos);
			liberarPunteroDePunterosAChar(divisionRuta);
			free(divisionRuta);
			free(archivos);
			free(rutaArchivosDirectorio);
		}else {
		printf("El directorio de ruta %s no tiene archivos\n", ruta);
		free(archivos);
		free(rutaArchivosDirectorio);
	}
	}


char * obtenerRutaSinArchivo(char * ruta){
	char ** directorios;
	char * archivo;
	int tamanioNombreArchivo;
	int tamanioRuta;

	directorios = string_split(ruta, "/");
	archivo = obtenerUltimoElementoDeUnSplit(directorios);
	tamanioNombreArchivo = strlen(archivo);
	tamanioRuta = strlen(ruta);

	liberarPunteroDePunterosAChar(directorios);
	free(directorios);
	free(archivo);
	return string_substring_until(ruta,tamanioRuta-tamanioNombreArchivo);

}

int verificarRutaArchivo(char * ruta){

	char * rutaSinArchivo;
	rutaSinArchivo =  obtenerRutaSinArchivo(ruta);

	free(rutaSinArchivo);
	return existeDirectorio(ruta);

}


