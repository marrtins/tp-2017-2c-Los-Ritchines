#include "funciones.h"

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
	int i,
		longitud = sizeof(palabras)/sizeof(palabras[0]);
	for(i = 0;i<longitud;i++){
		free(palabras[i]);
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
	FILE * archivoDirectorios = fopen("/home/utnso/buenasPracticas/fileSystem/src/metadata/directorios.txt", "r");
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



void levantarTablasArchivos(Tarchivos * tablaArchivos){

	t_config *archivo = config_create("/home/utnso/buenasPracticas/fileSystem/src/metadata/2/archivo1.txt");
	Tarchivos *archivos;
	char**temporal1;
    char**temporal2;
    int bytes;
	strcpy(archivos->tamanioTotal, config_get_int_value(archivo, "TAMANIO"));
	strcpy(archivos->extensionArchivo, config_get_string_value(archivo, "TIPO"));
	strcpy(temporal1, config_get_array_value(archivo, "BLOQUECOPIA0"));
	strcpy(temporal2, config_get_array_value(archivo, "BLOQUE0COPIA1"));
	strcpy(bytes, config_get_int_value(archivo, "BLOQUE0BYTES"));


	strcpy(archivos->bloques->copiaCero.nombreDeNodo,**temporal1);
	strcpy(archivos->bloques->copiaCero.numeroBloqueDeNodo,**(temporal1 + 1));
	archivos->bloques->bytes =bytes;
	//destruye la estructura de configuracion, supongo que para liberar memoria o por seguridad
	config_destroy(archivo);

	fclose(archivo);
}

void levantarTablasNodos(){

}

void levantarBitmap(){

}

void levantarTablas(Tdirectorios * tablaDirectorios, Tarchivos * tablaArchivos){
	levantarTablasDirectorios(tablaDirectorios);
	mostrarDirectorios(tablaDirectorios); //no hace nada, pero deberia
	levantarTablasNodos();
	levantarBitmap();
}
