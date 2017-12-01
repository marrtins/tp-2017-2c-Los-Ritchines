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
	if(list_size(listaDeNodos) >= 2){
		inicializarTablaDirectorios();
		//formatearTablaDeNodos();
		formatearNodos(listaDeNodos);
		formatearNodos(listaDeNodosDesconectados);
		levantarTablasDirectorios();
		sem_post(&yama);
		log_info(logInfo,"FILESYSTEM ESTABLE");
		cantDeNodosDeEstadoAnterior = 2;
		esEstadoRecuperado = 1; //para evitar que se conecten mas nodos
		puts("Filesystem formateado exitosamente");
	}
	else{
		puts("No se puede formatear, conecte al menos dos nodos para pasar a FILESYSTEM ESTABLE");
	}
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
		munmap(archivoMapeado,tamanio);
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
	munmap(archivoMapeado,tamanio);
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
	munmap(archivoMapeado,tamanio);

	close(fd);
}

void leerArchivoComoTextoPlano(char * rutaLocal){
	char * nombreArchivoConExtension = obtenerNombreDeArchivoDeUnaRuta(rutaLocal);
	char * extension = obtenerExtensionDeUnArchivo(nombreArchivoConExtension);
	char * rutaTmp = string_new();
	Tarchivo * archivo = malloc(sizeof(Tarchivo));
	int stat;
	levantarTablaArchivo(archivo,rutaLocal);
	mkdir("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/tmp/", 0777);
	string_append(&rutaTmp, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/tmp/");
	string_append(&rutaTmp, nombreArchivoConExtension);
	stat = levantarArchivo(archivo,rutaTmp);
	if(stat !=-1){
		if(strcmp(extension, "csv") == 0){
			mostrarCsv(rutaTmp);
		}else{
			mostrarBinario(rutaTmp);
		}
		liberarTablaDeArchivo(archivo);
	}
	remove(rutaTmp);
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

void renombrarArchivo(char * rutaYamafs, char * nombre) {
	char ** split = string_split(rutaYamafs, "/");
	char * ultimoElemento = obtenerUltimoElementoDeUnSplit(split);

		if ((int) string_contains(nombre, ".") == 1) {
			char * extensionOriginal = obtenerExtensionDeUnArchivo(ultimoElemento);
			char * extensionNueva = obtenerExtensionDeUnArchivo(nombre);

			if (string_equals_ignore_case(extensionOriginal, extensionNueva)) {

				char * rutaLocal = obtenerRutaLocalDeArchivo(rutaYamafs);
				char *nuevaRuta = obtenerRutaSinArchivo(rutaLocal);
				string_append(&nuevaRuta, "/");
				string_append(&nuevaRuta, nombre);

				if (rename(rutaLocal, nuevaRuta) == 0) {
					puts("Se renombro el archivo exitosamente.");
				} else {
					puts("Ruta o nombre invalido.");
					puts("No se pudo renombrar el archivo.");
				}
				free(rutaLocal);
				free(nuevaRuta);
			} else {
				puts("La extension tiene que ser la misma que la del archivo orginal.");
				puts("No se pudo renombrar el archivo.");
			}
			free(extensionNueva);
			free(extensionOriginal);
		} else {
			puts("Tiene que ingresar el nuevo nombre con la extension del archivo original.");
			puts("No se pudo renombrar el archivo.");
		}


	liberarPunteroDePunterosAChar(split);
	free(split);
	free(ultimoElemento);
}

void renombrarDirectorio(char * rutaYamafs, char * nombre){
	char ** split = string_split(rutaYamafs, "/");
	char * ultimoElemento = obtenerUltimoElementoDeUnSplit(split);

	Tdirectorio * directorio;

	directorio = buscarPorNombreDeDirectorio(ultimoElemento);
	strcpy(directorio->nombre, nombre);
	persistirTablaDeDirectorios();
	puts("Se renombro el directorio exitosamente.");

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
	char * ruta = malloc(280);
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

int removerArchivo(char* ruta){
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
					liberarPunteroDePunterosAChar(nombreYPosicion);
					free(nombreYPosicion);
					free(keyBloqueCopias);
					free(keyBloqueNCopiaM);
					return 0;
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
	return 1;

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

char * generarTabs(int cantidadDeTabs){
	char * tabs = string_new();
	if(cantidadDeTabs == 0){
		return tabs;
	}
	while(cantidadDeTabs > 0){
		string_append(&tabs, "\t");
		cantidadDeTabs--;
	}
	return tabs;
}

void mostrarPadreEHijosDeUnDirectorio(Tdirectorio * directorio, int nivelDelArbol){
	char * imprimible = generarTabs(nivelDelArbol);
	string_append_with_format(&imprimible, "/%s", directorio->nombre);
	puts(imprimible);
	t_list * hijos = buscarHijosDeUnDirectorio(directorio);
	int cantidadDeHijos = list_size(hijos);
	int i = 0;
	Tdirectorio * hijo;
	if(cantidadDeHijos > 0){
		while(i < cantidadDeHijos){
			hijo = list_get(hijos, i);
			mostrarPadreEHijosDeUnDirectorio(hijo, nivelDelArbol+1);
			i++;
		}
	}

	free(imprimible);
	list_destroy(hijos);
	//no destruyo los elementos por que sino destruiria la lista global
	//list_destroy_and_destroy_elements(hijos, free);

	return;
}

void mostrarArbolDeDirectorios(char * rutaADirectorio){
	char * nombreDirectorio;
	Tdirectorio * directorio;
	char ** split = string_split(rutaADirectorio, "/");
	if(contarPunteroDePunteros(split) == 1){
		directorio = buscarPorNombreDeDirectorio("root");
		mostrarPadreEHijosDeUnDirectorio(directorio, 0);
		liberarPunteroDePunterosAChar(split);
		free(split);
		return;
	}
	nombreDirectorio = obtenerUltimoElementoDeUnSplit(split);
	directorio = buscarPorNombreDeDirectorio(nombreDirectorio);
	if(directorio == NULL){
		puts("El directorio especificado, no existe");
		liberarPunteroDePunterosAChar(split);
		free(split);
		free(nombreDirectorio);
		return;
	}
	mostrarPadreEHijosDeUnDirectorio(directorio, 0);
	liberarPunteroDePunterosAChar(split);
	free(split);
	free(nombreDirectorio);
}

void mostrarTablaDeArchivosGlobal(t_list * tablaDeArchivosGlobal){
	int cantidadNodos = list_size(tablaDeArchivosGlobal);
	int cantidadArchivos;
	int cantidadBloques;
	int bloque;
	TelementoDeTablaArchivoGlobal * nodo;
	TarchivoDeTablaArchivoGlobal * archivo;
	int i = 0;
	int j = 0;
	int k = 0;
	ordenarTablaDeArchivosGlobalPorNombre(tablaDeArchivosGlobal);
	while(i < cantidadNodos){
		nodo = list_get(tablaDeArchivosGlobal, i);
		puts(nodo->nombreNodo);
		cantidadArchivos = list_size(nodo->archivos);
		j = 0;
		ordenarArchivosDeTablaDeArchivosGlobalPorNombre(nodo->archivos);
		while(j < cantidadArchivos){
			archivo = list_get(nodo->archivos, j);
			printf("\t%s:\n", archivo->nombreArchivo);
			cantidadBloques = list_size(archivo->bloques);
			k = 0;
			printf("\t[");
			while(k < cantidadBloques){
				bloque = (int)list_get(archivo->bloques, k);
				if(k == cantidadBloques-1){
					printf("%d", bloque);
				}
				else{
					printf("%d,", bloque);
				}
				k++;
			}
			printf("]\n");
			j++;
			puts("");
		}
		puts("");
		puts("");
		i++;
	}
}

char * generarRutaAPartirDeUnSplit(char ** palabras){
	int cantidadDeElementosDelSplit = contarPunteroDePunteros(palabras);
	char * ruta = string_new();
	while(cantidadDeElementosDelSplit > 0){
		if(!strcmp(palabras[cantidadDeElementosDelSplit-1], "root")){
			string_append(&ruta, "/");
		}
		string_append_with_format(&ruta, "%s/", palabras[cantidadDeElementosDelSplit-1]);
		cantidadDeElementosDelSplit--;
	}
	return ruta;
}

char** generarSplitApartirDeUnIndice(int index, char ** palabras, int cantidadPalabras){
	Tdirectorio * directorio = buscarDirectorioPorIndice(index);
	Tdirectorio * padre = buscarDirectorioPorIndice(directorio->padre);
	palabras = realloc(palabras, sizeof(char*) * (cantidadPalabras + 1));
	palabras[cantidadPalabras-1] = strdup(directorio->nombre);
	palabras[cantidadPalabras] = NULL;
	cantidadPalabras++;
	if(padre != NULL){
		palabras = generarSplitApartirDeUnIndice(padre->index, palabras, cantidadPalabras);
	}

	return palabras;
}

void actualizarEnTablaDeArchivosGlobal(t_list * tablaDeArchivosGlobal, Tarchivo * estructuraArchivo, char * nombreDirectorioPadre){
	int i = 0;
	int j = 0;
	int cantidadDeBloques = cantidadDeBloquesDeUnArchivo(estructuraArchivo->tamanioTotal);
	int cantidadDeCopias;
	int cantidadPalabras = 1;
	char ** palabras = malloc(sizeof(char *) * cantidadPalabras);
	palabras[0] = NULL;
	TcopiaNodo * copia;
	t_list * listaDeCopias;
	TelementoDeTablaArchivoGlobal * nodo;
	TarchivoDeTablaArchivoGlobal * archivoDeTablaDeArchivosGlobal;
	int index = atoi(nombreDirectorioPadre);
	palabras = generarSplitApartirDeUnIndice(index, palabras, cantidadPalabras);
	char * nombreDeArchivo = generarRutaAPartirDeUnSplit(palabras);
	string_append_with_format(&nombreDeArchivo, "%s.%s", estructuraArchivo->nombreArchivoSinExtension, estructuraArchivo->extensionArchivo);
	puts(nombreDeArchivo);
	while(i < cantidadDeBloques){
		cantidadDeCopias = estructuraArchivo->bloques[i].cantidadCopias;
		listaDeCopias = estructuraArchivo->bloques[i].copia;
		j = 0;
		while(j < cantidadDeCopias){
			copia = list_get(listaDeCopias, j);
			nodo = (TelementoDeTablaArchivoGlobal*)siNoExisteElNodoAgregar(copia->nombreDeNodo, tablaDeArchivosGlobal);
			archivoDeTablaDeArchivosGlobal = (TarchivoDeTablaArchivoGlobal*)siNoExisteElArchivoAgregar(nombreDeArchivo, nodo->archivos);
			//list_add(nodo->archivos, archivoDeTablaDeArchivosGlobal);
			list_add(archivoDeTablaDeArchivosGlobal->bloques, (void*)i);
			j++;
		}
		i++;
	}
	liberarPunteroDePunterosAChar(palabras);
	free(nombreDeArchivo);
	free(palabras);
}

void generarDistribucionDeBloquesEnNodos(struct dirent * directorio, t_list * tablaDeArchivosGlobal){
	char * rutaDirectorio = string_new();
	DIR * directorioMaestro;
	struct dirent * archivo;
	char * rutaArchivo;
	Tarchivo * estructuraArchivo;
	string_append(&rutaDirectorio, "/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/");
	string_append(&rutaDirectorio, directorio->d_name);
	string_append(&rutaDirectorio, "/");
	directorioMaestro = opendir(rutaDirectorio);
	log_info(logInfo, "Abrí la carpeta %s.", rutaDirectorio);
	if (directorioMaestro != NULL){
		archivo = readdir(directorioMaestro);
		while (archivo){
			if(strcmp(archivo->d_name, ".") && strcmp(archivo->d_name, "..")){
				estructuraArchivo =  malloc(sizeof(Tarchivo));
				rutaArchivo = string_new();
				string_append(&rutaArchivo, rutaDirectorio);
				string_append(&rutaArchivo, archivo->d_name);
				log_info(logInfo, "Archivo %s leído.", rutaArchivo);
				levantarTablaArchivo(estructuraArchivo, rutaArchivo);
				actualizarEnTablaDeArchivosGlobal(tablaDeArchivosGlobal, estructuraArchivo, directorio->d_name);
				liberarTablaDeArchivo(estructuraArchivo);
				free(rutaArchivo);
			}
			archivo = readdir(directorioMaestro);
		}
		closedir(directorioMaestro);
		free(rutaDirectorio);
		return;
	}
	closedir(directorioMaestro);
	free(rutaDirectorio);
	free(rutaArchivo);

}

void mostrarDistribucionDeBloquesEnNodos(){
	DIR * directorioMaestro;
	struct dirent * directorio;
	t_list * tablaDeArchivosGlobal = list_create();
	directorioMaestro = opendir("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/");
	log_info(logInfo, "Archivo Maestro abierto.");
	if (directorioMaestro != NULL){
		directorio = readdir(directorioMaestro);
		while (directorio){
			if(strcmp(directorio->d_name, ".") && strcmp(directorio->d_name, "..")){
				log_info(logInfo, "Carpeta yamafs leída (%s).", directorio->d_name);
				generarDistribucionDeBloquesEnNodos(directorio, tablaDeArchivosGlobal);
			}
			directorio = readdir(directorioMaestro);
		}
	}
	mostrarTablaDeArchivosGlobal(tablaDeArchivosGlobal);
	liberarTablaDeArchivosGlobal(tablaDeArchivosGlobal);
	closedir(directorioMaestro);
}
