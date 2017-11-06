#include "lib/funcionesDN.h"


int main(int argc, char* argv[]) {

	TdataNode *dataNode;
	int socketFS, fd, estado, bloqueAEliminar;
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

	if(archivo == NULL){
		puts("No existe el databin, o esta mal la ruta al archivo.");
		logAndExit("No existe el databin, o esta mal la ruta al archivo.");
	}

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
				case OBTENER_BLOQUE_Y_NRO:
					puts("Es fileSystem y quiere un bloque con su nro");
					int nroBloque;

					if ((estado = recv(socketFS, &nroBloque, sizeof(int), 0)) == -1) {
							logAndExit("Error al recibir el numero de bloque");
						}
					printf("El numero de bloque de mi data bin que quiere FS es %d\n",nroBloque);
					enviarBloqueAFS(nroBloque, socketFS);
					break;
				case OBTENER_BLOQUE:
					puts("ES fileSystem y quiere un bloque");
					int nroBloque_;
					unsigned long long int tamanioBloque_;
					if (recv(socketFS, &nroBloque_, sizeof(int), 0) == -1) {
						logAndExit("Error al recibir el numero do bloque");
					}
					if (recv(socketFS, &tamanioBloque_, sizeof(unsigned long long int),0) == -1){
						logAndExit("Error al recibir el tamanio del bloque");
					}
					enviarBloque(nroBloque_ , tamanioBloque_ ,socketFS);
					puts("Envie el bloque a FS");
					break;
				case ELIMINAR_BLOQUE:
					puts("Eliminando bloque");
					if (recv(socketFS, &bloqueAEliminar, sizeof(bloqueAEliminar),0) == -1){
						logAndExit("Error al recibir el tamanio del bloque");
					}
					printf("recibi el bloque a eliminar: %d\n", bloqueAEliminar);
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
