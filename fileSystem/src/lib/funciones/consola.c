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
		if(cantidad == 0){
			formatearFS();
			puts("FileSystem formateado.");
		}
		else{
			puts("Error en la cantidad de parametros.");
		}

	} else if (string_equals_ignore_case(*palabras, "rm")) {
		if (cantidad == 1){
			if(verificarRutaArchivo(palabras[1])){
				removerArchivo(palabras[1]);
			} else{
			puts("El archivo no existe en la ruta especificada");
			}
		}
	} else if (string_equals_ignore_case(*palabras, "rm-d")) {
		if (cantidad==1){
			if(existeDirectorio(palabras[1])){
			removerDirectorio(palabras[1]);
			puts("Ya pude remover el directorio");
		} else{
			puts("No pude remover el directorio");
		}
		}
	} else if (string_equals_ignore_case(*palabras, "rename")) {
		if(cantidad == 2){
			if(verificarRutaArchivo(palabras[1])){
				//falta corroborar que el archivo y los directorios existen
				char * rutaLocal = obtenerRutaLocalDeArchivo(palabras[1]);
				puts(rutaLocal);
				renombrarArchivoODirectorio(rutaLocal, palabras[2]);
				free(rutaLocal);
			}
			else{
				puts("No existe el directorio o falta la referencia a yamafs:");
			}
		}
		else{
			puts("Error en la cantidad de parametros.");
		}
		printf("ya pude renombrar el archivo\n");
	} else if (string_equals_ignore_case(*palabras, "mv")) {
		printf("ya pude mover el archivo\n");
	} else if (string_equals_ignore_case(*palabras, "cat")) {
		if(cantidad == 1){
			if(verificarRutaArchivo(palabras[1])){
				char * rutaLocal = obtenerRutaLocalDeArchivo(palabras[1]);
				leerArchivoComoTextoPlano(rutaLocal);
				puts("pase");
			}
			else{
				puts("No existe el directorio o falta la referencia a yamafs:");
			}
		}
		else{
			puts("Error en la cantidad de parametros");
		}
	} else if (string_equals_ignore_case(*palabras, "mkdir")) {
		if(cantidad == 1){
			if(existeDirectorio(palabras[1])){
				puts("Existe el directorio");
			}else {
				puts("No existe el directorio");
				if(crearDirectorio(palabras[1])>=0){
					persistirTablaDeDirectorios();
				}
			}
		}
		else{
			puts("Error en la cantidad de parametros");
		}
	} else if (string_equals_ignore_case(*palabras, "cpfrom")) {
		if(cantidad == 2){
			if(existeDirectorio(palabras[2])){
			puts("Existe el directorio");
			almacenarArchivo(palabras);
			}else {
				puts("No existe el directorio");
			}
		}
		else {
			puts("Error en la cantidad de parametros");
		}

	} else if (string_equals_ignore_case(*palabras, "cpto")) {
		if(verificarRutaArchivo(palabras[1])){
		//copiarArchivo(palabras);
		}
		printf("ya pude copiar un archivo local al file system\n");
	} else if (string_equals_ignore_case(*palabras, "cpblock")) {
		procesarCpblock(palabras);
	} else if (string_equals_ignore_case(*palabras, "md5")) {
			if (cantidad ==1){
				getMD5(palabras[1]);
				printf("ya pude solicitar el md5 de un archivo del file system\n");
			}
			else {
				puts("Error en la cantidad de parametros.");
			}

	} else if (string_equals_ignore_case(*palabras, "ls")) {
		if(cantidad == 1){
			if(existeDirectorio(palabras[1])){
					puts("Existe el directorio");
					listarArchivos(palabras[1]);
					}else {
					puts("No existe el directorio");
			}
		}
	} else if (string_equals_ignore_case(*palabras, "info")) {
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

		if(tieneBloque(rutaLocalArchivo, palabras[2])){
			if((buscarNodoPorNombre(listaDeNodos, palabras[3]))->nombre != NULL){
				Tarchivo* tablaArchivo = malloc(sizeof(Tarchivo));
				int nroBloque = atoi(palabras[2]);
				Tbuffer* bloque = malloc(sizeof(Tbuffer));
				TbloqueAEnviar* bloqueAEnviar = malloc(sizeof(TbloqueAEnviar));
				levantarTablaArchivo(tablaArchivo, rutaLocalArchivo);
				if(nodosDisponiblesParaBloqueDeArchivo(tablaArchivo, nroBloque) == 0){
					puts("No se encontraron los nodos con las copias del bloque");
					return;
				}
				if(pedirBloque(tablaArchivo, nroBloque) == -1){
					puts("Error al solicitar bloque");
					return;
				}
				pthread_cond_init(&bloqueCond, NULL);
				pthread_mutex_init(&bloqueMutex,NULL);

				pthread_mutex_lock(&bloqueMutex);
				pthread_cond_wait(&bloqueCond, &bloqueMutex);
				if(copiarBloque(bloqueACopiar, bloque) == -1){
					puts("Error al copiar bloque recibido");
					return;
				}
				bloqueAEnviar->contenido = bloque->buffer;
				bloqueAEnviar->tamanio = bloque->tamanio;
				bloqueAEnviar->numeroDeBloque = nroBloque;
				if(enviarBloqueA(bloqueAEnviar, palabras[3])){
					puts("Error no se pudo enviar el bloque");
					return;
				}
			}
			else {
				puts("El nodo destino no existe o no esta conectado.");
			}
		}
		else{
			puts("Numero de bloque incorrecto");
		}
	}
	else{
		puts("Ruta de archivo incorrecta.");
	}
}
