#include "lib/funcionesDN.h"


int main(int argc, char* argv[]) {

	TdataNode *dataNode;
	int socketFS, estado;
	char *bufferRecv = malloc(sizeof(Theader));
	char *mensaje = malloc(100);
	Theader *head = malloc(sizeof(Theader));
	head->tipo_de_proceso = DATANODE;
	head->tipo_de_mensaje = INFO_NODO;

	if(argc!=2){
			printf("Error en la cantidad de parametros\n");
			return EXIT_FAILURE;
		}

	logger = log_create("dataNode.log", "dataNode", false, LOG_LEVEL_INFO);
	dataNode = obtenerConfiguracion(argv[1]);
	mostrarConfiguracion(dataNode);
	puts("Intentando conectar con file system");

	socketFS = conectarAServidor(dataNode->ip_filesystem,dataNode->puerto_filesystem);

	puts("Conectado con file system");

	//Aca hay que mandar ip y puerto a fs
	estado = enviarHeader(socketFS, head);
	printf("Envie %d bytes\n",estado);

	if ((estado = recv(socketFS, &bufferRecv, sizeof(Theader), 0)) == -1) {
		logAndExit("Error al recibir informacion");

	} else if (estado == 0) {
		sprintf(mensaje, "Se desconecto el socket de fd: %d\n", socketFS);
		log_trace(logger, mensaje);

	}
	printf("Recibi %d bytes\n",estado);


	free(head);
	free(bufferRecv);
	free(mensaje);
	return EXIT_SUCCESS;
}
