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
		printf("ya pude formatear el fs\n");
	} else if (string_equals_ignore_case(*palabras, "rm")) {
		printf("ya pude remover el archivo\n");
	} else if (string_equals_ignore_case(*palabras, "rename")) {
		printf("ya pude renombrar el archivo\n");
	} else if (string_equals_ignore_case(*palabras, "mv")) {
		printf("ya pude mover el archivo\n");
	} else if (string_equals_ignore_case(*palabras, "cat")) {
		printf("ya pude leer el archivo\n");
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
		printf("ya pude crear una copia de un bloque del archivo en un nodo\n");
	} else if (string_equals_ignore_case(*palabras, "md5")) {
			if (cantidad ==1){
				//getMD5(palabras);
				printf("ya pude solicitar el md5 de un archivo del file system\n");
			}
			else {
				puts("Error en la cantidad de parametros");
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
		/*if (cantidad == 1){
			Tarchivo* tablaArchivo = malloc(sizeof(Tarchivo));
			levantarTablaArchivo(tablaArchivo, palabras[1]);
			mostrarTablaArchivo(tablaArchivo);

		}*/
		printf("ya pude mostrar la informacion del archivo\n");
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
