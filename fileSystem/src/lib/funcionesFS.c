#include "funcionesFS.h"

void logAndExit(char * mensaje){
	log_error(logger,mensaje);
	exit(-1);
}

TfileSystem *obtenerConfiguracion(char* ruta){
	TfileSystem *fileSystem = malloc(sizeof(TfileSystem));

	fileSystem->puerto_entrada = malloc(MAXIMA_LONGITUD_PUERTO);
	fileSystem->puerto_datanode = malloc(MAXIMA_LONGITUD_PUERTO);
	fileSystem->puerto_yama = malloc(MAXIMA_LONGITUD_PUERTO);
	fileSystem->ip_yama = malloc(MAXIMA_LONGITUD_IP);

	//funcion de gaston, recibe la ruta del archivo de configuracion y te devuelve
	//un puntero a una estructura con todos los datos que va leyendo del archivo de conf
	t_config *fsConfig = config_create(ruta);

	//config_get_string_value recibe el nombre del parametro, y te devuelve el valor
	//con esto voy cargando en una estructura, todos los datos del archivo de conf
	strcpy(fileSystem->puerto_entrada, config_get_string_value(fsConfig, "PUERTO_FILESYSTEM"));
	strcpy(fileSystem->puerto_datanode, config_get_string_value(fsConfig, "PUERTO_DATANODE"));
	strcpy(fileSystem->puerto_yama, config_get_string_value(fsConfig, "PUERTO_YAMA"));
	strcpy(fileSystem->ip_yama, config_get_string_value(fsConfig, "IP_YAMA"));

	//cargo el tipo de proceso (harcodeado)
	fileSystem->tipo_de_proceso = FILESYSTEM;

	//destruye la estructura de configuracion, supongo que para liberar memoria o por seguridad
	config_destroy(fsConfig);
	//retorno la configuracion
	return fileSystem;
}

void mostrarConfiguracion(TfileSystem *fileSystem){

	printf("Puerto Entrada: %s\n",  fileSystem->puerto_entrada);
	printf("Puerto Datanode: %s\n",       fileSystem->puerto_datanode);
	printf("Puerto Yama: %s\n", fileSystem->puerto_yama);
	printf("IP Yama: %s\n", fileSystem->ip_yama);
	printf("Tipo de proceso: %d\n", fileSystem->tipo_de_proceso);
}

void liberarPunteroDePunterosAChar(char** palabras){
	int i = 0;

	while(palabras[i] != NULL){
		free(palabras[i]);
		i++;
	}

}

void procesarInput(char* linea) {
	char **palabras = string_split(linea, " ");
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
		printf("ya pude crear el directorio\n");
	} else if (string_equals_ignore_case(*palabras, "cpfrom")) {
		printf(
				"ya pude copiar el archivo local al file system siguiendo lineamientos\n");
	} else if (string_equals_ignore_case(*palabras, "cpto")) {
		printf("ya pude copiar un archivo local al file system\n");
	} else if (string_equals_ignore_case(*palabras, "cpblock")) {
		printf("ya pude crear una copia de un bloque del archivo en un nodo\n");
	} else if (string_equals_ignore_case(*palabras, "md5")) {
		printf("ya pude solicitar el md5 de un archivo del file system\n");
	} else if (string_equals_ignore_case(*palabras, "ls")) {
		printf("ya pude listar los archivos del directorio\n");
	} else if (string_equals_ignore_case(*palabras, "info")) {
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

void consolaFS(){
	puts("Bienvenido a la consola. Ingrese un comando:");
	while(1){
		char *linea = readline(">");
		procesarInput(linea);
	}
}



int conectarNuevoCliente( int fileDescriptor, fd_set * bolsaDeFileDescriptors){

		int nuevoFileDescriptor = aceptarCliente(fileDescriptor);
		FD_SET(nuevoFileDescriptor, bolsaDeFileDescriptors);
		/*if ((listen(fileDescriptor, BACKLOG)) == -1){
			perror("Fallo listen al socket. error");
			printf("Fallo listen() en el sock_listen: %d", fileDescriptor);
			return FALLO_GRAL;
		}*/
		return nuevoFileDescriptor;
}

void clearAndClose(int fileDescriptor, fd_set* masterFD){
	FD_CLR(fileDescriptor, masterFD);
	close(fileDescriptor);

}

void levantarTablasDirectorios(Tdirectorios * tablaDirectorios){
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "r");
	int i = 0;

	while(!feof(archivoDirectorios)){
		fscanf(archivoDirectorios, "%s %d", tablaDirectorios[i].nombre, &tablaDirectorios[i].padre);
		printf("%d \t %s \t %d \n", i, tablaDirectorios[i].nombre, tablaDirectorios[i].padre);
		i++;
	}

	fclose(archivoDirectorios);

}

void mostrarDirectorios(Tdirectorios * tablaDirectorios){

}


void levantarTablaArchivos(Tarchivos * tablaArchivos){

	t_config *archivo = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/2/archivo1.txt");

	tablaArchivos->extensionArchivo = malloc(sizeof(Tarchivos));
	tablaArchivos->bloques = malloc(sizeof(Tbloques));

	int cantBloques,
			nroBloque = 0;
	char **temporal1,
				**temporal2;
	char* bloqueCopia0 = malloc(sizeof(char)*20);
	char* bloqueCopia1 = malloc(sizeof(char)*20);
	char* bloqueBytes = malloc(sizeof(char)*20);

	tablaArchivos->tamanioTotal = config_get_int_value(archivo, "TAMANIO");
	strcpy(tablaArchivos->extensionArchivo, config_get_string_value(archivo, "TIPO"));

	cantBloques = ceil(tablaArchivos->tamanioTotal/1048576.0);
	printf("cant bloques %d\n",cantBloques);

	printf("Tamanio %d\n", tablaArchivos->tamanioTotal);
	printf("Extension %s\n", tablaArchivos->extensionArchivo);

	while(nroBloque != cantBloques){

		tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo = malloc(sizeof(char*));
		tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo = malloc(sizeof(char*));
		tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo = malloc(sizeof(char*));
		tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo = malloc(sizeof(char*));

		sprintf(bloqueCopia0,"BLOQUE%dCOPIA0",nroBloque);
		sprintf(bloqueCopia1,"BLOQUE%dCOPIA1",nroBloque);
		sprintf(bloqueBytes,"BLOQUE%dBYTES",nroBloque);

		temporal1 = config_get_array_value(archivo, bloqueCopia0);
		temporal2 = config_get_array_value(archivo, bloqueCopia1);
		tablaArchivos->bloques[nroBloque].bytes = config_get_int_value(archivo, bloqueBytes);

		strcpy(tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo,temporal1[0]);
		strcpy(tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo,temporal1[1]);

		strcpy(tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo,temporal2[0]);
		strcpy(tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo,temporal2[1]);

		printf("Nombre de nodo copia cero %s\n",tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo);
		printf("Numero de nodo copia cero %s\n",tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo);
		printf("Nombre de nodo copia uno %s\n",tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo);
		printf("Numero de nodo copia uno %s\n",tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo);
		printf("Bytes %d\n",tablaArchivos->bloques[nroBloque].bytes);
		nroBloque++;

		liberarPunteroDePunterosAChar(temporal1);
		free(temporal1);
		liberarPunteroDePunterosAChar(temporal2);
		free(temporal2);
	}
	//NO ESTA HECHO EL FREE DE LA TABLA DE ARCHIVOS PORQUE SON DATOS QUE SIEMPRE NECESITAMOS CREO
	//config_destroy(archivo);
}


int contarPunteroDePunteros(char ** puntero){
	char ** aux = puntero;
	int contador = 0;
	while(*aux != NULL){
		contador++;
		aux++;
	}
	return contador;
}


void crearBitmaps(t_list * listaBitmaps){
/*	char ** nodos;
	char * bitArray;
	char * nombreNodoTotal = malloc(sizeof(char) * 12);
	char * nombreNodoLibre = malloc(sizeof(char) * 12);
	int longitud, i, totalNodos, nodosLibres;
	t_bitarray * bitmap;
	t_config * tablaNodosConfig = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");
	nodos = config_get_array_value(tablaNodosConfig, "NODOS");
	longitud = contarPunteroDePunteros(nodos);
	for(i = 1; i <= longitud; i++){
		sprintf(nombreNodoTotal, "Nodo%dTotal", i);
		totalNodos = config_get_int_value(tablaNodosConfig, nombreNodoTotal);
		sprintf(nombreNodoLibre, "Nodo%dLibre", i);
		nodosLibres = config_get_int_value(tablaNodosConfig, nombreNodoLibre);
		bitArray = malloc(totalNodos);
		bitmap = bitarray_create_with_mode(bitArray, sizeof(bitArray), LSB_FIRST);

		list_add(listaBitmaps, bitmap);
	}*/

}

void levantarTablasNodos(){
	/*t_config * tablaNodosConfig = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/nodos.bin");
	config_set_value(tablaNodosConfig, "Nodo1Libre", "5");
	config_save(tablaNodosConfig); // escribe algo en el config*/
}

void levantarTablas(Tdirectorios * tablaDirectorios, t_list * listaBitMaps){
	levantarTablasDirectorios(tablaDirectorios);
	mostrarDirectorios(tablaDirectorios); //no hace nada, pero deberia
	//levantarTablasNodos(); el torri dijo que no va
	crearBitmaps(listaBitMaps);
}
