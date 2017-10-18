#include "lib/funcionesDN.h"


int main(int argc, char* argv[]) {

	TdataNode *dataNode;
	int socketFS, fd, estado, nroBloque,tamanioBloque;
	char* contenidoBloque;
	char *bufferRecv = malloc(sizeof(Theader));
	char *mensaje = malloc(100);
	Theader *head = malloc(sizeof(Theader));
	head->tipo_de_proceso = DATANODE;
	head->tipo_de_mensaje = INFO_NODO;
	FILE * archivo = fopen("/home/utnso/tp-2017-2c-Los-Ritchines/dataNode/data1.bin", "rb+");

	if(argc!=2){
			printf("Error en la cantidad de parametros\n");
			return EXIT_FAILURE;
		}

	fd = fileno(archivo);
	if ((archivoMapeado = mmap(NULL, BLOQUE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
					logAndExit("Error al hacer mmap");
			}
	fclose(archivo);
	close(fd);

	logger = log_create("dataNode.log", "dataNode", false, LOG_LEVEL_INFO);
	dataNode = obtenerConfiguracionDN(argv[1]);
	mostrarConfiguracion(dataNode);
	puts("Intentando conectar con file system");

	socketFS = conectarAServidor(dataNode->ip_filesystem,dataNode->puerto_filesystem);

	puts("Conectado con file system");

	//Aca hay que mandar ip y puerto a fs
	estado = enviarHeader(socketFS, head);
	printf("Envie %d bytes\n",estado);

	if ((estado = recv(socketFS, head, sizeof(Theader), 0)) == -1) {
		logAndExit("Error al recibir informacion");

	} else if (estado == 0) {
		sprintf(mensaje, "Se desconecto el socket de fd: %d\n", socketFS);
		log_trace(logger, mensaje);

	}
	printf("Recibi %d bytes\n",estado);


	if(head->tipo_de_proceso == FILESYSTEM){
		switch(head->tipo_de_mensaje){
			case ALMACENAR_BLOQUE:
				puts("Es FileSystem y quiere almacenar un bloque");
				if ((estado = recv(socketFS, nroBloque, sizeof(int), 0)) == -1) {
						logAndExit("Error al recibir el numero de bloque");
				}
				printf("Recibí el numero de bloque %d\n", nroBloque);
				if ((estado = recv(socketFS, tamanioBloque, sizeof(int), 0)) == -1) {
						logAndExit("Error al recibir el tamaño de bloque");
				}
				printf("Tamanio de bloque = %d\n", tamanioBloque);
				if ((estado = recv(socketFS, contenidoBloque, tamanioBloque, 0)) == -1) {
						logAndExit("Error al recibir el contenido del bloque");
				}
	}


}


	free(head);
	free(bufferRecv);
	free(mensaje);

	return EXIT_SUCCESS;
}
