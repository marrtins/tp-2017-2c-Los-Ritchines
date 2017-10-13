#include "funcionesFS.h"

TfileSystem * obtenerConfiguracionFS(char* ruta){
	TfileSystem *fileSystem = malloc(sizeof(TfileSystem));

	fileSystem->puerto_entrada = malloc(MAXIMA_LONGITUD_PUERTO);
	fileSystem->puerto_datanode = malloc(MAXIMA_LONGITUD_PUERTO);
	fileSystem->puerto_yama = malloc(MAXIMA_LONGITUD_PUERTO);
	fileSystem->ip_yama = malloc(MAXIMA_LONGITUD_IP);

	t_config *fsConfig = config_create(ruta);

	strcpy(fileSystem->puerto_entrada, config_get_string_value(fsConfig, "PUERTO_FILESYSTEM"));
	strcpy(fileSystem->puerto_datanode, config_get_string_value(fsConfig, "PUERTO_DATANODE"));
	strcpy(fileSystem->puerto_yama, config_get_string_value(fsConfig, "PUERTO_YAMA"));
	strcpy(fileSystem->ip_yama, config_get_string_value(fsConfig, "IP_YAMA"));
	fileSystem->cant_nodos = config_get_int_value(fsConfig, "CANT_NODOS");

	printf("Cant de nodos %d\n", fileSystem->cant_nodos);

	fileSystem->tipo_de_proceso = FILESYSTEM;

	config_destroy(fsConfig);

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

int cantidadParametros(char ** palabras){
	int i = 1;
	while(palabras[i] != NULL){
		i++;
	}
	return i-1;
}

void almacenarArchivo(char **palabras){
	char * archivoMapeado;
	FILE * archivoOrigen = fopen(palabras[1], "r");
	int fd = fileno(archivoOrigen);
	int tamanio = tamanioArchivo(archivoOrigen);
	if ((archivoMapeado = mmap(NULL, tamanio, PROT_READ, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
					logAndExit("Error al hacer mmap");
			}
	fclose(archivoOrigen);
	close(fd);
	int cantidadBloques = ceil((float)tamanio / BLOQUE_SIZE);
	char * buffer = malloc(BLOQUE_SIZE);
	if(tamanio == 0){
		puts("Error al almacenar archivo, está vacío");
	}
	//else
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
		printf("ya pude crear el directorio\n");
	} else if (string_equals_ignore_case(*palabras, "cpfrom")) {
		if(cantidad == 2){
			almacenarArchivo(palabras);
		}
		else {
			puts("Error en la cantidad de parametros");
		}
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
		return nuevoFileDescriptor;
}

void clearAndClose(int fileDescriptor, fd_set* masterFD){
	FD_CLR(fileDescriptor, masterFD);
	close(fileDescriptor);

}

void conexionesDatanode(void * estructura){
	TfileSystem * fileSystem = (TfileSystem *) estructura;
	fd_set readFD, masterFD;
	int socketDeEscuchaDatanodes,
		fileDescriptorMax = -1,
		cantModificados = 0,
		nuevoFileDescriptor,
		fileDescriptor,
		estable,
		cantNodosPorConectar = fileSystem->cant_nodos,
		estado;
	t_list * listaBitmaps = list_create();
	Theader * head = malloc(sizeof(Theader));
	char * mensaje = malloc(100);

	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);
	mostrarConfiguracion(fileSystem);
	socketDeEscuchaDatanodes = crearSocketDeEscucha(fileSystem->puerto_datanode);
	fileDescriptorMax = MAXIMO(socketDeEscuchaDatanodes, fileDescriptorMax);
	puts("antes de entrar al while");

	while (listen(socketDeEscuchaDatanodes, BACKLOG) == -1){
				log_trace(logger, "Fallo al escuchar el socket servidor de file system.");
				puts("Reintentamos...");
	}

	FD_SET(socketDeEscuchaDatanodes, &masterFD);
	printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);
	while(1){

			printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);

			puts("Entre al while");
			readFD = masterFD;
			puts("Voy a usar el select");
			if((cantModificados = select(fileDescriptorMax + 1, &readFD, NULL, NULL, NULL)) == -1){
				logAndExit("Fallo el select.");
			}
			printf("Cantidad de fd modificados: %d \n", cantModificados);
			puts("pude usar el select");

			for(fileDescriptor = 3; fileDescriptor <= fileDescriptorMax; fileDescriptor++){
				printf("Entre al for con el fd: %d\n", fileDescriptor);
				if(FD_ISSET(fileDescriptor, &readFD)){
					printf("Hay un file descriptor listo. El id es: %d\n", fileDescriptor);

					if(fileDescriptor == socketDeEscuchaDatanodes){
						nuevoFileDescriptor = conectarNuevoCliente(fileDescriptor, &masterFD);
						printf("Nuevo nodo conectado: %d\n", nuevoFileDescriptor);

						cantNodosPorConectar--;
						puts("Filesystem estable");

						fileDescriptorMax = MAXIMO(nuevoFileDescriptor, fileDescriptorMax);
						printf("El FILEDESCRIPTORMAX es %d", fileDescriptorMax);
						break;
					}
						puts("Recibiendo...");

						estado = recv(fileDescriptor, head, sizeof(Theader), 0);

						if(estado == -1){
							log_trace(logger, "Error al recibir información de un cliente.");
							break;
						}
						else if( estado == 0){
							sprintf(mensaje, "Se desconecto el cliente de fd: %d.", fileDescriptor);
							log_trace(logger, mensaje);
							clearAndClose(fileDescriptor, &masterFD);
						}
					if(head->tipo_de_proceso==DATANODE){
						switch(head->tipo_de_mensaje){
							case INFO_NODO:
									list_add(listaBitmaps, crearBitmap(20)); //hardcodeado
									mostrarBitmap(list_get(listaBitmaps,0));
								break;

							default:
								puts("Hacker detected");
								break;
						}

					printf("Recibi %d bytes\n",estado);
					printf("el proceso es %d\n", head->tipo_de_proceso);
					printf("el mensaje es %d\n", head->tipo_de_mensaje);
					break;

				}

				} //termine el if


				puts("Sali del if de ISSET");

			} //termine el for

			puts("sali del for");

		} // termina el while

	//POR ACA VA UN SIGNAL PARA INDICAR QUE FS YA TIENE TODOS LOS NODOS CONECTADOS.
}

void levantarTablasDirectorios(Tdirectorios * tablaDirectorios){
	FILE * archivoDirectorios = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/directorios.txt", "r");
	int i = 0;

	while(!feof(archivoDirectorios)){
		fscanf(archivoDirectorios, "%d %s %d", &tablaDirectorios[i].index, tablaDirectorios[i].nombre, &tablaDirectorios[i].padre);
		printf("%d \t %s \t %d \n", tablaDirectorios[i].index, tablaDirectorios[i].nombre, tablaDirectorios[i].padre);
		i++;
	}

	fclose(archivoDirectorios);

}

void mostrarDirectorios(Tdirectorios * tablaDirectorios){

}


void levantarTablaArchivos(Tarchivos * tablaArchivos){

	t_config *archivo = config_create("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/archivos/2/archivo1.txt");

	tablaArchivos->extensionArchivo = malloc(sizeof(char)*10);


	int cantBloques,
			nroBloque = 0;
	char **temporal1;
	char **temporal2;
	char* bloqueCopia0 = malloc(sizeof(char)*20);
	char* bloqueCopia1 = malloc(sizeof(char)*20);
	char* bloqueBytes = malloc(sizeof(char)*20);

	tablaArchivos->tamanioTotal = config_get_int_value(archivo, "TAMANIO");
	strcpy(tablaArchivos->extensionArchivo, config_get_string_value(archivo, "TIPO"));

	cantBloques = ceil(tablaArchivos->tamanioTotal/1048576.0);
	tablaArchivos->bloques = malloc(sizeof(Tbloques)*cantBloques);
	printf("cant bloques %d\n",cantBloques);

	printf("Tamanio %d\n", tablaArchivos->tamanioTotal);
	printf("Extension %s\n", tablaArchivos->extensionArchivo);

	while(nroBloque != cantBloques){

		tablaArchivos->bloques[nroBloque].copiaCero.nombreDeNodo = malloc(sizeof(char)*7);
		tablaArchivos->bloques[nroBloque].copiaCero.numeroBloqueDeNodo = malloc(sizeof(char)*4);
		tablaArchivos->bloques[nroBloque].copiaUno.nombreDeNodo = malloc(sizeof(char)*7);
		tablaArchivos->bloques[nroBloque].copiaUno.numeroBloqueDeNodo = malloc(sizeof(char)*4);

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
		free(bloqueCopia0);
		free(bloqueCopia1);
		free(bloqueBytes);

	//NO ESTA HECHO EL FREE DE LA TABLA DE ARCHIVOS PORQUE SON DATOS QUE SIEMPRE NECESITAMOS CREO
	config_destroy(archivo);
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

t_bitarray* crearBitmap(int tamanioDatabin){
	int tamanioEnBits = ceil(tamanioDatabin/8.0);
	char * bitarray = calloc(tamanioEnBits,1);
	t_bitarray* bitmap = bitarray_create_with_mode(bitarray,tamanioEnBits,LSB_FIRST);
	return bitmap;
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



	/*
	config_set_value(tablaNodosConfig, "Nodo1Libre", "5");
	config_save(tablaNodosConfig); // escribe algo en el configs
*/	free(nodoTotal);
	free(nodoLibre);
	config_destroy(archivoNodos);
}

void levantarTablas(Tdirectorios * tablaDirectorios, Tnodos * tablaNodos){
	levantarTablasDirectorios(tablaDirectorios);
	levantarTablaNodos(tablaNodos);
	mostrarDirectorios(tablaDirectorios); //no hace nada, pero deberia
}

void mostrarBitmap(t_bitarray* bitmap){
	int i;
	puts("El bitmap es:");
	for(i=0;i < bitarray_get_max_bit(bitmap);i++){
		printf("%d",bitarray_test_bit(bitmap,i));
	}
	puts("");
}

void liberarTablaDeArchivos(Tarchivos * tablaDeArchivos){
	int i;
	int cantBloques = ceil(tablaDeArchivos->tamanioTotal/1048576.0);
	tablaDeArchivos->extensionArchivo = malloc(sizeof(Tarchivos));
	tablaDeArchivos->bloques = malloc(sizeof(Tbloques));

	for(i = 0; i != cantBloques; i++){
		free(tablaDeArchivos->bloques[i].copiaCero.nombreDeNodo);
		free(tablaDeArchivos->bloques[i].copiaCero.numeroBloqueDeNodo);
		free(tablaDeArchivos->bloques[i].copiaUno.nombreDeNodo);
		free(tablaDeArchivos->bloques[i].copiaUno.numeroBloqueDeNodo);
	}
}
