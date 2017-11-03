#include "../funcionesFS.h"

void consolaFS(){
	puts("Bienvenido a la consola. Ingrese un comando:");
	while(1){
		char *linea = readline(">");
		add_history(linea);
		procesarInput(linea);
	}
}

void procesarInput(char* linea) {
	int cantidad = 0;
	char **palabras = string_split(linea, " ");
	cantidad = cantidadParametros(palabras);
	if (string_equals_ignore_case(*palabras, "format")) {
		consolaFormat(palabras, cantidad);
	} else if (string_equals_ignore_case(*palabras, "rm")) {
		consolaRemove(palabras,cantidad);
	} else if (string_equals_ignore_case(*palabras, "rename")) {
		consolaRename(palabras,cantidad);
	} else if (string_equals_ignore_case(*palabras, "mv")) {
		consolaMove(palabras,cantidad);
	} else if (string_equals_ignore_case(*palabras, "cat")) {
		consolaCat(palabras,cantidad);
	} else if (string_equals_ignore_case(*palabras, "mkdir")) {
		consolaMkdir(palabras,cantidad);
	} else if (string_equals_ignore_case(*palabras, "cpfrom")) {
		consolaCpfrom(palabras,cantidad);
	} else if (string_equals_ignore_case(*palabras, "cpto")) {
		if(verificarRutaArchivo(palabras[1])){
		copiarArchivo(palabras);
		}
	} else if (string_equals_ignore_case(*palabras, "cpblock")) {
		procesarCpblock(palabras);
	} else if (string_equals_ignore_case(*palabras, "md5")) {
		consolaMd5(palabras,cantidad);

	} else if (string_equals_ignore_case(*palabras, "ls")) {
		consolaLs(palabras,cantidad);
	} else if (string_equals_ignore_case(*palabras, "info")) {
		consolaInfo(palabras,cantidad);
	} else if(string_equals_ignore_case(*palabras, "exit")){
		printf("Finalizando consola\n");
		liberarPunteroDePunterosAChar(palabras);
		free(palabras);
		free(linea);
		pthread_kill(pthread_self(),SIGKILL);
	} else {
		printf("No existe el comando\n");
	}
	liberarPunteroDePunterosAChar(palabras);
	free(palabras);
	free(linea);

}

void procesarCpblock(char ** palabras){
	if(verificarRutaArchivo(palabras[1])){
		char * rutaLocalArchivo = obtenerRutaLocalDeArchivo(palabras[1]);
		Tnodo *nodo = buscarNodoPorNombre(listaDeNodos, palabras[3]);
		if(nodo != NULL){
			if(obtenerBloqueDisponible(nodo) != -1){
				Tarchivo* tablaArchivo = malloc(sizeof(Tarchivo));
				int nroBloque = atoi(palabras[2]);
				Tbuffer* bloque;
				TbloqueAEnviar* bloqueAEnviar;
				levantarTablaArchivo(tablaArchivo, rutaLocalArchivo);
				free(rutaLocalArchivo);
				if (nroBloque>=cantidadDeBloquesDeUnArchivo(tablaArchivo->tamanioTotal)) {
					puts("Numero de bloque incorrecto");
					liberarTablaDeArchivo(tablaArchivo);
					return;
				}
				if (nodosDisponiblesParaBloqueDeArchivo(tablaArchivo, nroBloque) == 0) {
					puts("No se encontraron los nodos con las copias del bloque");
					liberarTablaDeArchivo(tablaArchivo);
					return;
				}
				pthread_mutex_init(&bloqueMutex, NULL);
				if (pedirBloque(tablaArchivo, nroBloque) == -1) {
					puts("Error al solicitar bloque");
					liberarTablaDeArchivo(tablaArchivo);
					return;
				}
				liberarTablaDeArchivo(tablaArchivo);
				pthread_mutex_lock(&bloqueMutex);
				pthread_mutex_lock(&bloqueMutex);

				bloque = malloc(sizeof(Tbuffer));
				if (copiarBloque(bloqueACopiar, bloque) == -1) {
					puts("Error al copiar bloque recibido");

				}
				liberarEstructuraBuffer(bloqueACopiar);
				bloqueAEnviar = malloc(sizeof(TbloqueAEnviar));
				bloqueAEnviar->contenido = bloque->buffer;
				bloqueAEnviar->tamanio = bloque->tamanio;
				bloqueAEnviar->numeroDeBloque = nroBloque;
				if (enviarBloqueA(bloqueAEnviar, palabras[3]) == -1) {
					puts("Error no se pudo enviar el bloque");
					liberarEstructuraBuffer(bloque);
					return;
				}
				liberarEstructuraBuffer(bloque);
				puts("Perfeeecto, el bloque se copio lo mas bien");
			}
			else{
				puts("El nodo destino esta lleno.");
			}
		}
		else {
			puts("El nodo destino no existe o no esta conectado.");
		}
	}
	else{
		puts("Ruta de archivo incorrecta.");
	}
}

void consolaFormat(char**palabras, int cantidad){
	if(cantidad == 0){
		formatearFS();
		puts("FileSystem formateado.");
	}
	else{
		puts("Error en la cantidad de parametros.");
	}
}
void consolaRename(char** palabras, int cantidad) {
	if (cantidad == 2) {
		if (verificarRutaArchivo(palabras[1]) || 1) {
			//falta corroborar que el archivo y los directorios existen
			char * rutaLocal = obtenerRutaLocalDeArchivo(palabras[1]);
			renombrarArchivoODirectorio(rutaLocal, palabras[2]);
			free(rutaLocal);
		}
		else{
			puts("No existe el directorio o falta la referencia a yamafs:");
		}
	}
	else{
		puts("Error en la cantidad de parametros");
	}
}

void consolaCat(char**palabras, int cantidad) {
	if (cantidad == 1) {
		if (verificarRutaArchivo(palabras[1])) {
			char * rutaLocal = obtenerRutaLocalDeArchivo(palabras[1]);
			leerArchivoComoTextoPlano(rutaLocal);
			puts("pase");
		}
		else {
			puts("No existe el directorio o falta la referencia a yamafs:");
		}
	}
	else {
		puts("Error en la cantidad de parametros");
	}
}

void consolaMkdir(char**palabras, int cantidad){
	if(cantidad == 1){
		if(existeDirectorio(palabras[1])){
			puts("Existe el directorio");
		}
		else{
			puts("No existe el directorio");
			if(crearDirectorio(palabras[1])>=0){
				persistirTablaDeDirectorios();
			}
		}
	}
	else{
		puts("Error en la cantidad de parametros");
	}
}

void consolaCpfrom(char** palabras, int cantidad){
	if(cantidad == 2){
		if(existeDirectorio(palabras[2])){
			puts("Existe el directorio");
			almacenarArchivo(palabras);
		}
		else {
			puts("No existe el directorio");
		}
	}
	else {
		puts("Error en la cantidad de parametros");
	}
}

void consolaMd5(char** palabras, int cantidad){
	if (cantidad == 1){
		getMD5(palabras[1]);
		printf("ya pude solicitar el md5 de un archivo del file system\n");
	}
	else {
		puts("Error en la cantidad de parametros.");
	}
}

void consolaLs(char**palabras, int cantidad){
	if(cantidad == 1){
		if(existeDirectorio(palabras[1])){
			puts("Existe el directorio");
			listarArchivos(palabras[1]);
		}
		else{
			puts("No existe el directorio");
		}
	}
	else{
		puts("Error en la cantidad de parametros");
	}
}

void consolaInfo(char**palabras, int cantidad){
	if (cantidad == 1){
		if(verificarRutaArchivo(palabras[1])){
			Tarchivo* tablaArchivo = malloc(sizeof(Tarchivo));
			char * rutaLocal = obtenerRutaLocalDeArchivo(palabras[1]);
			levantarTablaArchivo(tablaArchivo, rutaLocal);
			mostrarTablaArchivo(tablaArchivo);
			liberarTablaDeArchivo(tablaArchivo);
			free(rutaLocal);
		}
		else{
			puts("No existe el directorio o falta la referencia a yamafs:");
		}
	}
	else{
		puts("Error en la cantidad de parametros");
	}
}

void consolaRemove (char** palabras, int cantidad){
	if (cantidad == 1){

		if(verificarRutaArchivo(palabras[1])){
			removerArchivo(palabras[1]);
		} else{
			puts("El archivo no existe en la ruta especificada");
		}
	} else if (cantidad ==2){
		if (string_equals_ignore_case(palabras[1], "-d")){

			if(existeDirectorio(palabras[2])){
				if(esDirectorioVacio(palabras[2])){
				removerDirectorio(palabras[2]);
				puts("Ya pude remover el directorio");
				} else{
				puts("EL directorio no esta vacio. No se puede remover");
				}
			} else{
				puts("No existe el directorio");
			}
		} else if (string_equals_ignore_case(palabras[1], "-b")){
			puts("Voy a eliminar un nodo");
			//removerNodo
		}
		} else{
			puts("Error en la cantidad de parametros");
		}
}

int getMD5(char* ruta){
	char* rutaArchivo = obtenerRutaLocalDeArchivo(ruta);
	char* comando = string_duplicate("md5sum ");
	string_append(&comando, rutaArchivo);
	system(comando);
	printf("Obtuve el MD5 del archivo");
	free(comando);
	free(rutaArchivo);
	return 0;
}

void consolaMove(char** palabras, int cantidad){
	if(cantidad==2){
		if(verificarRutaArchivo(palabras[1])){
			moverArchivo(palabras[1], palabras[2]);
		} else{
			puts("No se quiere mover un archivo");
		}
	}else{
		puts("Error en la cantidad de parametros");
	}
}
