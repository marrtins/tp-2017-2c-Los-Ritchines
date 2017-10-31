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

void formatearFS(){
	inicializarTablaDirectorios();
	inicializarTablaDeNodos();
	formatearNodos();
}

char * obtenerNombreDeArchivoDeUnaRuta(char * rutaLocal){
	char * archivoConExtension;
	char ** split = string_split(rutaLocal, "/");
	archivoConExtension = obtenerUltimoElementoDeUnSplit(split);
	liberarPunteroDePunterosAChar(split);
	free(split);
	return archivoConExtension;

}

//no se si funciona, verificar
void mostrarNCaracteresDeUnMMap(char * archivoMapeado, unsigned long long tamanio, unsigned long long desde, unsigned long long hasta){
	unsigned long long incrementador = desde;
	while(incrementador < tamanio && incrementador <= hasta){
		printf("%c", archivoMapeado[incrementador]);
		incrementador++;
	}
	puts("");
}

unsigned long long mostrarlineaDeUnMMap(char * archivoMapeado, unsigned long long tamanio, unsigned long long desde, int cantidadLineas){
	unsigned long long incrementador = desde;
	unsigned long long leido = 0;
	while(incrementador < tamanio && cantidadLineas > 0){
		printf("%c", archivoMapeado[incrementador]);
		if(archivoMapeado[incrementador] == '\n'){
			cantidadLineas--;
		}
		incrementador++;
		leido++;
	}
	return leido;
}

//no se si funciona, verificar
void mostrarCsv(char * rutaLocal){
	FILE * archivo = fopen(rutaLocal, "rb");
	unsigned long long tamanio = tamanioArchivo(archivo);
	char * archivoMapeado;
	int fd = fileno(archivo);
	if ((archivoMapeado = mmap(NULL, tamanio, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		log_trace(logger, "No se pudo abrir el archivo especificado.");
		puts("No se pudo abrir el archivo especificado.");
		return;
	}
	fclose(archivo);
	char entrada = 's';
	int tamanioLineas = 5;
	unsigned long long leido = 0;
	while(leido <= tamanio && entrada == 's'){
		leido += mostrarlineaDeUnMMap(archivoMapeado, tamanio, leido, tamanioLineas);
		printf("Desea Seguir? (s/n) ");
		scanf(" %c", &entrada);
	}
	puts("Finalizado, archivo leído.");
	close(fd);
}

void mostrarBinario(char * rutaLocal){
	FILE * archivo = fopen(rutaLocal, "rb");
	unsigned long long tamanio = tamanioArchivo(archivo);
	char * archivoMapeado;
	int fd = fileno(archivo);
	if ((archivoMapeado = mmap(NULL, tamanio, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		log_trace(logger, "No se pudo abrir el archivo especificado.");
		puts("No se pudo abrir el archivo especificado.");
		return;
	}
	fclose(archivo);
	char entrada = 's';
	unsigned long long contador = 0;
	int sumador = 150;
	while(contador < tamanio && entrada == 's'){
		mostrarNCaracteresDeUnMMap(archivoMapeado, tamanio, contador, contador+sumador);
		contador += sumador + 1;
		if(contador+sumador >= tamanio){
			sumador = tamanio - contador - 1;
		}
		printf("Desea Seguir? (s/n) ");
		scanf(" %c", &entrada);
	}
	puts("Finalizado, archivo leído.");
	close(fd);
}

void leerArchivoComoTextoPlano(char * rutaLocal){
	char * nombreArchivoConExtension = obtenerNombreDeArchivoDeUnaRuta(rutaLocal);
	char * extension = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);
	if(strcmp(extension, "csv") == 0){
		mostrarCsv(rutaLocal);
	}else{
		mostrarBinario(rutaLocal);
	}

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

char ** obtenerSubconjuntoDeUnSplit(char ** split, int desde, int hasta){
	char ** nuevoSplit = malloc(1);
	int i = 0;
	while(desde <= hasta){
		nuevoSplit = realloc(nuevoSplit, i+1);
		nuevoSplit[i] = strdup(split[desde]);
		i++;
		desde++;
	}

	return nuevoSplit;
}

void renombrarArchivoODirectorio(char * ruta, char * nombre){
	char * nuevaRuta = obtenerRutaSinArchivo(ruta);
	string_append(&nuevaRuta, "/");
	string_append(&nuevaRuta, nombre);
	if(rename(ruta, nuevaRuta) == 0){
		puts("Se ha renombrado el archivo");
	}
	else{
		puts("La ruta especificada no concuerda con la ruta del archivo a renombrar");
	}
	free(nuevaRuta);
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

	  directorioActual = opendir(ruta);

	  if (directorioActual == NULL){
	    puts("No pudo abrir el directorio");

	    log_trace(logger,"No se pudo abrir el directorio, hubo un error.");

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
	    log_trace(logger,"No se pudo abrir el directorio, hubo un error.");

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

				free(nombreArchivoConExtension);
			}
			liberarPunteroDePunterosAChar(archivos);
			liberarPunteroDePunterosAChar(divisionRuta);
			free(divisionRuta);
			free(archivos);
			free(rutaArchivosDirectorio);
		}else {
		printf("El directorio de ruta %s no tiene archivos\n", ruta);

		log_trace(logger,"El directorio no tiene archivos");
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
	tamanioNombreArchivo = strlen(archivo) +1;
	tamanioRuta = strlen(ruta);

	liberarPunteroDePunterosAChar(directorios);
	free(directorios);
	free(archivo);
	return string_substring_until(ruta,tamanioRuta-tamanioNombreArchivo);

}

char * obtenerRutaLocalDeArchivo(char * rutaYamafs){
	int indice;
	char * rutaSinArchivo;
	char * archivo;
	char * ruta = malloc(100);
	char ** carpetas = string_split(rutaYamafs,"/");

	rutaSinArchivo =  obtenerRutaSinArchivo(rutaYamafs);
	indice = obtenerIndexDeUnaRuta(rutaSinArchivo);

	strcpy(ruta,"/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/");
	string_append_with_format(&ruta,"%d",indice);
	string_append(&ruta,"/");
	archivo = obtenerUltimoElementoDeUnSplit(carpetas);
	string_append(&ruta,archivo);

	liberarPunteroDePunterosAChar(carpetas);
	free(carpetas);
	free(rutaSinArchivo);
	free(archivo);
	return ruta;
}

int existeArchivo(int indiceDirectorio , char * rutaYamafs){
	int i =0;
	char ** archivos;
	char *archivo;
	char * ruta = malloc(100);
	char ** carpetas = string_split(rutaYamafs,"/");

	strcpy(ruta,"/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/");
	string_append_with_format(&ruta,"%d",indiceDirectorio);
	archivos = buscarArchivos(ruta);

	string_append(&ruta,"/");
	archivo = obtenerUltimoElementoDeUnSplit(carpetas);
	string_append(&ruta,archivo);

	if (archivos[i] != NULL) {
		while (archivos[i] != NULL) {
			if (string_equals_ignore_case(archivos[i], ruta)) {
				liberarPunteroDePunterosAChar(carpetas);
				free(carpetas);
				liberarPunteroDePunterosAChar(archivos);
				free(archivos);
				free(archivo);
				free(ruta);
				return 1;
			}
			i++;
		}
		liberarPunteroDePunterosAChar(archivos);
	}
	liberarPunteroDePunterosAChar(carpetas);
	free(carpetas);
	free(archivos);
	free(archivo);
	free(ruta);
	return 0;
}

int validarQueLaRutaTengaElNombreDelArchivo(char * ruta){
	char ** carpetas = string_split(ruta,"/");
	char * archivo;
	int valor;
	archivo = obtenerUltimoElementoDeUnSplit(carpetas);

	valor = (int) string_contains(archivo, ".");

	liberarPunteroDePunterosAChar(carpetas);
	free(carpetas);
	free(archivo);
	return valor;

}

int verificarRutaArchivo(char * rutaYamafs){

	int indice;
	char * rutaSinArchivo;
	if(validarQueLaRutaTengaElNombreDelArchivo(rutaYamafs)){
		rutaSinArchivo =  obtenerRutaSinArchivo(rutaYamafs);
		if(existeDirectorio(rutaSinArchivo)){
			indice = obtenerIndexDeUnaRuta(rutaSinArchivo);
			if(existeArchivo(indice,rutaYamafs)){
				free(rutaSinArchivo);
				return 1;
			}
		}
		free(rutaSinArchivo);
	}else{
	puts("La ruta yamafs debe contener el nombre del archivo y la extensión");
	}
	return 0;

}

void removerArchivo(char* ruta){
	char* rutaArchivo = obtenerRutaLocalDeArchivo(ruta);
	remove(rutaArchivo);
	puts("Ya pude remover el archivo");
	}


