#include "lib/funcionesDN.h"


int main(int argc, char* argv[]) {

	TdataNode *dataNode;
	int socketFS, fd, estado;
	char *bufferHead = malloc(sizeof(Theader));
	char *mensaje = malloc(100);
	Theader *head = malloc(sizeof(Theader));
	Tbloque * bloque;

	if(argc!=2){
			printf("Error en la cantidad de parametros\n");
			return EXIT_FAILURE;
		}

	logger = log_create("dataNode.log", "dataNode", false, LOG_LEVEL_INFO);
	dataNode = obtenerConfiguracionDN(argv[1]);
	mostrarConfiguracion(dataNode);

	FILE * archivo = fopen(dataNode->ruta_databin, "rb+");

	fd = fileno(archivo);
	if ((archivoMapeado = mmap(NULL, dataNode->tamanio_databin_mb*BLOQUE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
					logAndExit("Error al hacer mmap");
			}
	fclose(archivo);
	close(fd);

	puts("Intentando conectar con file system");

	socketFS = conectarAServidor(dataNode->ip_filesystem,dataNode->puerto_filesystem);

	puts("Conectado con file system");

	//manda el nombre la ip y el puerto del nodo
	estado = enviarInfoNodo(socketFS, dataNode);
	printf("Envie %d bytes con la informacion del nodo\n",estado);

	while (1) {

		if ((estado = recv(socketFS, head, sizeof(Theader), 0)) == -1) {
			logAndExit("Error al recibir informacion");
			break;

		} else if (estado == 0) {
			sprintf(mensaje, "Se desconecto el socket de fd: %d\n", socketFS);
			log_trace(logger, mensaje);
			break;
		}
		printf("Recibi el head %d bytes\n", estado);

		if (head->tipo_de_proceso == FILESYSTEM) {
			switch (head->tipo_de_mensaje) {
			case ALMACENAR_BLOQUE:
				puts("Es FileSystem y quiere almacenar un bloque");

				bloque = recvBloque(socketFS);
				puts("voy a almacenar el bloque");
				setBloque(bloque->nroBloque, bloque);
				puts("Bloque almacenado");

				free(bloque->contenido);
				free(bloque);

				break;
			default:
				break;
			}

		}
	}


	free(head);
	free(bufferHead);
	free(mensaje);

	return EXIT_SUCCESS;
}
