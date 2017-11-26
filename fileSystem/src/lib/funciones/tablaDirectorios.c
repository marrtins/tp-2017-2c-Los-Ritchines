#include "../funcionesFS.h"

int esRutaYamafs(char * ruta){
	char ** carpetas = string_split(ruta, "/");
	int retorno;
	if(string_equals_ignore_case(carpetas[0], "yamafs:")){
		retorno = 1;
	}
	else{
		retorno = 0;
	}
	liberarPunteroDePunterosAChar(carpetas);
	free(carpetas);
	return retorno;
}

void persistirTablaDeDirectorios(){
	int tamanio, i=0;
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "w");
	tamanio = list_size(listaTablaDirectorios);
	Tdirectorio * directorio;

	while(tamanio != i){
		directorio = (Tdirectorio*) list_get(listaTablaDirectorios, i);
		fprintf(archivoDirectorios, "%d %s %d\n", directorio->index, directorio->nombre, directorio->padre);
		i++;
	}

	fclose(archivoDirectorios);
}

void levantarTablasDirectorios(){
	int i;
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "r");
	int tam = tamanioArchivo(archivoDirectorios);
	int fd = fileno(archivoDirectorios);
	char * archivoMapeado;
	char ** directorios;

	if ((archivoMapeado = mmap(NULL,tam, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		logErrorAndExit("Error al hacer mmap al levantar tabla de directorios.");
	}
	fclose(archivoDirectorios);
	close(fd);

	directorios = string_split(archivoMapeado,"\n");
	int cant = contarPunteroDePunteros(directorios);

	for(i=0; i < cant; i++){
		Tdirectorio * tDirectorio = malloc(sizeof(Tdirectorio));
		char ** directorio = string_split(directorios[i]," ");
		tDirectorio->index = atoi(directorio[0]);
		strcpy(tDirectorio->nombre, directorio[1]);
		tDirectorio->padre = atoi(directorio[2]);
		//printf("%d \t %s \t %d\n",tDirectorio->index,tDirectorio->nombre,tDirectorio->padre);
		list_add(listaTablaDirectorios, tDirectorio);
		liberarPunteroDePunterosAChar(directorio);
		free(directorio);
	}

	liberarPunteroDePunterosAChar(directorios);
	free(directorios);
	munmap(archivoMapeado,tam);
}

int crearDirectorio(char * ruta) {
	int nroDirectorio, cant, index, indicePadre;
	char ** carpetas = string_split(ruta, "/");
	cant = contarPunteroDePunteros(carpetas);
	char* directorio = malloc(100);
	strcpy(directorio, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/");
	char * indice;
	Tdirectorio * tDirectorio;

	if(list_size(listaTablaDirectorios)>=100){
		puts("Ya hay 100 directorios creados, no se puede crear el directorio.");
		liberarPunteroDePunterosAChar(carpetas);
		free(carpetas);
		free(directorio);
		return -1;
	}
	if ((nroDirectorio = directorioNoExistente(carpetas)) < 0) {
		//TODO falta decir por que no se pudo crear el directorio
		puts("No se puede crear el directorio.");
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
			indice = string_itoa(index);
			string_append(&directorio, indice);
			free(indice);
			//syscall(SYS_mkdir, directorio);
			mkdir(directorio,0777);
			tDirectorio = malloc(sizeof(Tdirectorio));
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
			puts("No se puede crear directorio dentro de un directorio que no existe.");
			liberarPunteroDePunterosAChar(carpetas);
			free(carpetas);
			free(directorio);
			return -1;
	}
}
void crearRoot(){
	mkdir("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/0/",0777);
}

void inicializarTablaDirectorios(){
	char * ruta = malloc(150);
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "w");
	strcpy(ruta,"/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/");
	vaciarLista();
	fprintf(archivoDirectorios, "%d %s %d", 0, "root", -1);
	mkdir(ruta,0777);
	fclose(archivoDirectorios);
	removerDirectorios(ruta);
	crearRoot();
	free(ruta);
}

void formatearTablaDeNodos(){
	t_config * archivo = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");
	char * nodoNTotalKey;
	char * nodoNLibreKey;
	int nodoNTotal;
	char * tamanioString;
	char * nodoNTotalString;
	int tamanio;
	char ** nodos = config_get_array_value(archivo,"NODOS");
	int i = 0;

	while(nodos[i]!=NULL){
		nodoNTotalKey = generarStringNodoNTotal(nodos[i]);
		nodoNLibreKey = generarStringNodoNLibre(nodos[i]);
		nodoNTotal = config_get_int_value(archivo, nodoNTotalKey);
		nodoNTotalString = string_itoa(nodoNTotal);
		config_set_value(archivo, nodoNLibreKey, nodoNTotalString);
		free(nodoNTotalKey);
		free(nodoNLibreKey);
		free(nodoNTotalString);
		i++;
	}

	tamanio = config_get_int_value(archivo, "TAMANIO");
	tamanioString = string_itoa(tamanio);
	config_set_value(archivo,"LIBRE",tamanioString);
	config_save(archivo);
	config_destroy(archivo);
	liberarPunteroDePunterosAChar(nodos);
	free(nodos);
	free(tamanioString);

}

void formatearFS(){
	inicializarTablaDirectorios();
	formatearTablaDeNodos();
	formatearNodos(listaDeNodos);
	formatearNodos(listaDeNodosDesconectados);
	levantarTablasDirectorios();
}

char * obtenerNombreDeArchivoDeUnaRuta(char * ruta){
	char * archivoConExtension;
	char ** split = string_split(ruta, "/");
	archivoConExtension = obtenerUltimoElementoDeUnSplit(split);
	liberarPunteroDePunterosAChar(split);
	free(split);
	if(strstr(archivoConExtension, ".") != NULL){
		return archivoConExtension;
	}
	return NULL;

}

char * obtenerExtensionDeArchivoDeUnaRuta(char * rutaLocal){
	char * archivoConExtension = obtenerNombreDeArchivoDeUnaRuta(rutaLocal);
	char * extension = obtenerExtensionDeUnArchivo(archivoConExtension);
	free(archivoConExtension);
	return extension;
}

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

void mostrarCsv(char * rutaLocal){
	FILE * archivo = fopen(rutaLocal, "rb");
	unsigned long long tamanio = tamanioArchivo(archivo);
	char * archivoMapeado;
	int fd = fileno(archivo);
	if ((archivoMapeado = mmap(NULL, tamanio, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		log_error(logError, "No se pudo abrir el archivo csv.");
		puts("No se pudo abrir el archivo especificado csv.");
		return;
	}
	fclose(archivo);
	char entrada = 's';
	int tamanioLineas = 20;
	unsigned long long leido = 0;
	while(leido < tamanio && (entrada == 's' || entrada == 'a')){
		leido += mostrarlineaDeUnMMap(archivoMapeado, tamanio, leido, tamanioLineas);
		if(entrada == 's'){
			puts("Presione (s) para mostrar mas lineas.");
			puts("Presione (a) si quiere mostrar todo.");
			puts("Presione cualquier otra tecla si desea terminar.");
			scanf(" %c", &entrada);
		}

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
		log_error(logError, "No se pudo abrir el archivo especificado.");
		puts("No se pudo abrir el archivo especificado.");
		return;
	}
	fclose(archivo);
	char entrada = 's';
	unsigned long long contador = 0;
	int sumador = 500;
	while(contador < tamanio && (entrada == 's' || entrada == 'a')){
		mostrarNCaracteresDeUnMMap(archivoMapeado, tamanio, contador, contador+sumador);
		contador += sumador + 1;
		if(contador+sumador >= tamanio){
			sumador = tamanio - contador - 1;
		}
		if(entrada == 's'){
			puts("Presione (s) para mostrar mas lineas.");
			puts("Presione (a) si quiere mostrar todo.");
			puts("Presione cualquier otra tecla si desea terminar.");
			scanf(" %c", &entrada);
		}
	}
	puts("Finalizado, archivo leido.");
	close(fd);
}

void leerArchivoComoTextoPlano(char * rutaLocal){
	char * nombreArchivoConExtension = obtenerNombreDeArchivoDeUnaRuta(rutaLocal);
	char * extension = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);
	char * rutaTmp = string_new();
	Tarchivo * archivo = malloc(sizeof(Tarchivo));

	levantarTablaArchivo(archivo,rutaLocal);
	mkdir("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/tmp/", 0777);
	string_append(&rutaTmp, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/tmp/");
	string_append(&rutaTmp, nombreArchivoConExtension);
	levantarArchivo(archivo,rutaTmp);
	if(strcmp(extension, "csv") == 0){
		mostrarCsv(rutaTmp);
	}else{
		mostrarBinario(rutaTmp);
	}

	remove(rutaTmp);
	liberarTablaDeArchivo(archivo);
	free(nombreArchivoConExtension);
	free(extension);
	free(rutaTmp);

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
		log_error(logError, "No se pudo crear el directorio por que no existe la referencia 'yamafs:'");
		free(yamafs);
		return -1;
	}
}

int obtenerIndexDeUnaRuta(char * rutaDestino){
	int indice;
	char ** palabras = string_split(rutaDestino, "/");
	int cant = contarPunteroDePunteros(palabras);
	char * directorio = obtenerUltimoElementoDeUnSplit(palabras);
	//
	liberarPunteroDePunterosAChar(palabras);
	free(palabras);
	if(cant == 1){
		indice = 0;
	}else{
		indice = buscarIndexPorNombreDeDirectorio(directorio);
	}
	free(directorio);
	return indice;
}

int buscarIndexPorNombreDeDirectorio(char * directorio){
	Tdirectorio * estructuraDirectorio = (Tdirectorio *)buscarPorNombreDeDirectorio(directorio);

	if(estructuraDirectorio != NULL){
		return estructuraDirectorio->index;
	}
	log_error(logError, "No existe el nombre de directorio.");
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

void renombrarArchivoODirectorio(char * rutaYamafs, char * nombre) {
	char ** split = string_split(rutaYamafs, "/");
	char * ultimoElemento = obtenerUltimoElementoDeUnSplit(split);

	Tdirectorio * directorio;

	if ((int) string_contains(ultimoElemento, ".") == 1) {
		if ((int) string_contains(nombre, ".") == 1) {
			char * extensionOriginal = obtenerExtensionDeUnArchivo(
					ultimoElemento);
			char * extensionNueva = obtenerExtensionDeUnArchivo(nombre);

			if (string_equals_ignore_case(extensionOriginal, extensionNueva)) {

				char * rutaLocal = obtenerRutaLocalDeArchivo(rutaYamafs);
				char *nuevaRuta = obtenerRutaSinArchivo(rutaLocal);
				string_append(&nuevaRuta, "/");
				string_append(&nuevaRuta, nombre);

				if (rename(rutaLocal, nuevaRuta) == 0) {
					puts("Se renombro el archivo exitosamente.");
				} else {
					puts("La ruta especificada no concuerda con la ruta del archivo a renombrar.");
				}
				free(rutaLocal);
				free(nuevaRuta);
			} else {
				puts("La extension tiene que ser la misma que la del archivo orginal.");
			}
			free(extensionNueva);
			free(extensionOriginal);
		} else {
			puts("Tiene que ingresar el nuevo nombre con la extension del archivo original");
		}
	} else {
		directorio = buscarPorNombreDeDirectorio(ultimoElemento);
		strcpy(directorio->nombre, nombre);
		persistirTablaDeDirectorios();
		puts("Se renombro el directorio exitosamente.");
	}

	liberarPunteroDePunterosAChar(split);
	free(split);
	free(ultimoElemento);
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
		  log_error(logError,"No se pudo abrir el directorio.");
	  }
	  else{
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

	  directorioActual = opendir(ruta);

	  if (directorioActual == NULL){
	    puts("No se pudo abrir el directorio.");
	    log_error(logError,"No se pudo abrir el directorio.");

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

	while (directorios[i] != NULL) {

		removerArchivos(directorios[i]);
		rmdir(directorios[i]);
		i++;

	}
	liberarPunteroDePunterosAChar(directorios);
	free(directorios);
}


void listarArchivos(char* ruta){

	char ** archivos;
	int i = 0, index;
	char * rutaArchivosDirectorio = malloc(200);
	char ** carpetas;
	char * nombreArchivoConExtension;
	index = obtenerIndexDeUnaRuta(ruta);
	sprintf(rutaArchivosDirectorio, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/%d", index);
	archivos = buscarArchivos(rutaArchivosDirectorio);

		if(archivos[i] != NULL){
			while(archivos[i] != NULL){

				carpetas = string_split(archivos[i], "/");
				nombreArchivoConExtension = obtenerUltimoElementoDeUnSplit(carpetas);
				puts(nombreArchivoConExtension);
				i++;

				free(nombreArchivoConExtension);
			}
			liberarPunteroDePunterosAChar(archivos);
			liberarPunteroDePunterosAChar(carpetas);
			free(carpetas);
			free(archivos);
			free(rutaArchivosDirectorio);
		}
		else {
			printf("El directorio de ruta %s no tiene archivos.\n", ruta);
			log_error(logError,"El directorio no tiene archivos.");
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

int esRutaDeUnArchivo(char * ruta){
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

int verificarRutaArchivo(char * rutaYamafs) {

	int indice;
	char * rutaSinArchivo;
	if (esRutaYamafs(rutaYamafs)) {
		if (esRutaDeUnArchivo(rutaYamafs)) {
			rutaSinArchivo = obtenerRutaSinArchivo(rutaYamafs);
			if (existeDirectorio(rutaSinArchivo)) {
				indice = obtenerIndexDeUnaRuta(rutaSinArchivo);
				if (existeArchivo(indice, rutaYamafs)) {
					free(rutaSinArchivo);
					return 1;
				} else {
					puts("No existe el archivo en yamafs");
				}
			} else {
				puts("No existe el directorio");
			}
			free(rutaSinArchivo);
		} else {
			puts("La ruta yamafs debe contener el nombre del archivo y la extensión.");
		}
	} else {
		puts("La ruta debe contener la referencia a 'yamafs:'");
	}
	return 0;

}

int verificarRutaArchivoSinPuts(char * rutaYamafs){

	int indice;
	char * rutaSinArchivo;
	if(esRutaDeUnArchivo(rutaYamafs)){
		rutaSinArchivo =  obtenerRutaSinArchivo(rutaYamafs);
		if(existeDirectorio(rutaSinArchivo)){
			indice = obtenerIndexDeUnaRuta(rutaSinArchivo);
			if(existeArchivo(indice,rutaYamafs)){
				free(rutaSinArchivo);
				return 1;
			}
		}
		free(rutaSinArchivo);
	}
	return 0;

}

void removerArchivo(char* ruta){
	char* rutaArchivo = obtenerRutaLocalDeArchivo(ruta);
	t_config * archivo = config_create(rutaArchivo);
	Tnodo * nodo;
	char * keyBloqueCopias;
	char * keyBloqueNCopiaM;
	char **nombreYPosicion;
	int cantidadCopias;
	int i = 0;
	int j;
	int cantidadBloques = cantidadDeBloquesDeUnArchivo(config_get_long_value(archivo,"TAMANIO"));
	while(i < cantidadBloques){
		keyBloqueCopias = generarStringBloqueNCopias(i);
		cantidadCopias = config_get_int_value(archivo, keyBloqueCopias);
		j = 0;
		while(j < cantidadCopias){
			keyBloqueNCopiaM = generarStringDeBloqueNCopiaN(i,j);
			nombreYPosicion = config_get_array_value(archivo,keyBloqueNCopiaM);
			nodo = buscarNodoPorNombre(listaDeNodos, nombreYPosicion[0]);
			if(nodo == NULL){
				nodo = buscarNodoPorNombre(listaDeNodosDesconectados, nombreYPosicion[0]);
				if(nodo == NULL){
					puts("No se pudo completar la operacion, el nodo con la copia no esta en ningun lado.");
					liberarPunteroDePunterosAChar(nombreYPosicion);
					free(nombreYPosicion);
					free(keyBloqueCopias);
					free(keyBloqueNCopiaM);
					return;
				}
			}
			desocuparBloque(nodo, atoi(nombreYPosicion[1]));
			liberarPunteroDePunterosAChar(nombreYPosicion);
			free(nombreYPosicion);
			free(keyBloqueNCopiaM);
			j++;
		}
		free(keyBloqueCopias);
		i++;
	}

	remove(rutaArchivo);
	free(rutaArchivo);
	config_destroy(archivo);
	puts("El archivo se elimino correctamente.");
	}

void pasarInfoDeUnArchivoAOtro(char * archivoAMoverMapeado, char * archivoMapeado, unsigned long long tamanio){
	memcpy(archivoMapeado, archivoAMoverMapeado, tamanio);
}

void moverArchivo(char* rutaConArchivo, char* rutaDirectorioDestino){
	char* rutaLocalArchivo = obtenerRutaLocalDeArchivo(rutaConArchivo);
	char** palabras = string_split(rutaLocalArchivo, "/");
	char* nombreArchivoConExtension = obtenerUltimoElementoDeUnSplit(palabras);
	char * extension = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);
	char * archivoMapeado;
	char * archivoAMoverMapeado;

	int index = obtenerIndexDeUnaRuta(rutaDirectorioDestino);
	char* rutaLocalDirectorio = malloc(200);
	FILE * archivo;
	if(strcmp(extension, "csv") == 0){
		archivo = fopen(rutaLocalArchivo, "r");
	}
	else{
		archivo = fopen(rutaLocalArchivo, "rb");
	}
	unsigned long long tamanio = tamanioArchivo(archivo);
	int fdAMover = fileno(archivo);

	if ((archivoAMoverMapeado = mmap(NULL, tamanio, PROT_READ, MAP_SHARED, fdAMover, 0)) == MAP_FAILED) {
		free(rutaLocalArchivo);
		liberarPunteroDePunterosAChar(palabras);
		free(palabras);
		free(nombreArchivoConExtension);
		free(extension);
		fclose(archivo);
		close(fdAMover);
		log_error(logError, "No se pudo abrir el archivo especificado.");
		puts("No se pudo abrir el archivo especificado.");
		return;
	}

	sprintf(rutaLocalDirectorio, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/%d", index);
	string_append(&rutaLocalDirectorio,"/");
	string_append(&rutaLocalDirectorio, nombreArchivoConExtension);

	FILE * archivoMovido = fopen(rutaLocalDirectorio, "w+");
	int fd = fileno(archivoMovido);
	ftruncate(fd, tamanio);
	if ((archivoMapeado = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		free(rutaLocalArchivo);
		liberarPunteroDePunterosAChar(palabras);
		free(palabras);
		free(nombreArchivoConExtension);
		free(extension);
		fclose(archivo);
		fclose(archivoMovido);
		close(fd);
		close(fdAMover);
		log_error(logError, "No se pudo abrir el archivo especificado.");
		puts("No se pudo abrir el archivo especificado.");
		return;
	}

	pasarInfoDeUnArchivoAOtro(archivoAMoverMapeado, archivoMapeado, tamanio);

	remove(rutaLocalArchivo);

	fclose(archivo);
	fclose(archivoMovido);
	close(fd);
	close(fdAMover);
	munmap(archivoMapeado, tamanio);
	munmap(archivoAMoverMapeado, tamanio);

	remove(rutaLocalArchivo);
	free(rutaLocalDirectorio);
	free(rutaLocalArchivo);
	liberarPunteroDePunterosAChar(palabras);
	free(palabras);
	free(nombreArchivoConExtension);
	free(extension);

	puts("Se movio el archivo correctamente.");
}

int esHijoDirectoOIndirecto(Tdirectorio * posiblePadre, Tdirectorio * posibleHijo){
	if(posibleHijo->index == 0){
		return 0;
	}
	if(posiblePadre->index == posibleHijo->padre){
		return 1;
	}
	else{
		return esHijoDirectoOIndirecto(posiblePadre, buscarDirectorioPorIndice(posibleHijo->padre));
	}
}

void moverDirectorio(char*rutaDirectorioOrigen, char*rutaDirectorioDestino){
	Tdirectorio* directorioOrigen;
	Tdirectorio* directorioDestino;
	char ** split1 = string_split(rutaDirectorioOrigen,"/");
	char * nombreDirectorioOriginal = obtenerUltimoElementoDeUnSplit(split1);
	directorioOrigen = buscarPorNombreDeDirectorio(nombreDirectorioOriginal);

	char ** split2 = string_split(rutaDirectorioDestino,"/");
	char * nombreDirectorioFinal = obtenerUltimoElementoDeUnSplit(split2);
	directorioDestino = buscarPorNombreDeDirectorio(nombreDirectorioFinal);

	//if(directorioDestino->padre == directorioOrigen->index){
	if(esHijoDirectoOIndirecto(directorioOrigen, directorioDestino)){
		printf("No se puede mover porque el directorio %s esta dentro del directorio %s.\n", directorioDestino->nombre,directorioOrigen->nombre);
	}
	else{
		directorioOrigen->padre = directorioDestino->index;
		persistirTablaDeDirectorios();
		puts("Directorio movido.");
	}
	liberarPunteroDePunterosAChar(split1);
	free(split1);
	liberarPunteroDePunterosAChar(split2);
	free(split2);
	free(nombreDirectorioOriginal);
	free(nombreDirectorioFinal);
}

void removerDirectorio(char* ruta){

	int index = obtenerIndexDeUnaRuta(ruta);
	char** palabras = string_split(ruta, "/");
	char* nombreDirectorio = obtenerUltimoElementoDeUnSplit(palabras);
	char* rutaDirectorio = malloc(200);
	sprintf(rutaDirectorio, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/%d", index);
	rmdir(rutaDirectorio);
	removerDirectorioDeTabla(nombreDirectorio);
	free(rutaDirectorio);
	liberarPunteroDePunterosAChar(palabras);
	free(palabras);
	free(nombreDirectorio);
}

void removerDirectorioDeTabla(char* nombreDirectorio){
	int tamanio, i=0;
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "r+");
	tamanio = list_size(listaTablaDirectorios);
	Tdirectorio * directorio;

	while(tamanio != i){
		directorio = list_get(listaTablaDirectorios, i);
		if(string_equals_ignore_case(nombreDirectorio, directorio->nombre)){
			list_remove(listaTablaDirectorios, i);
			break;
		}
		i++;
	}
	fclose(archivoDirectorios);
	persistirTablaDeDirectorios();

}



int esDirectorioVacio (char*ruta){
	int index = obtenerIndexDeUnaRuta(ruta);
	int i = 0;
	char* rutaLocalDirectorio = malloc (200);
	sprintf(rutaLocalDirectorio, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/%d", index);
	char** archivos;
	int condicion;

	archivos = buscarArchivos(rutaLocalDirectorio);
	condicion = (archivos[i] == NULL) && (!esDirectorioPadre(ruta));
	liberarPunteroDePunterosAChar(archivos);
	free(archivos);
	free(rutaLocalDirectorio);
	return condicion;
}

int esDirectorioPadre (char* ruta){
	int index = obtenerIndexDeUnaRuta(ruta);
	int tamanio, i=0;
	tamanio = list_size(listaTablaDirectorios);
	Tdirectorio * directorio;

	while(tamanio != i){
			directorio = list_get(listaTablaDirectorios, i);
			if(index == directorio->padre){
				return 1;
			}
			i++;
	}
			return 0;
}

int esDirectorioRaiz (char*ruta){
	char ** palabras = string_split(ruta, "/");
	int tamanioRuta = contarPunteroDePunteros(palabras);

	if (tamanioRuta == 1){
		liberarPunteroDePunterosAChar(palabras);
		free(palabras);
		return 1;

	}
	liberarPunteroDePunterosAChar(palabras);
		free(palabras);
	return 0;
}

void crearDirectorioTemporal(){
	mkdir("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/tmp/",0777);
}
