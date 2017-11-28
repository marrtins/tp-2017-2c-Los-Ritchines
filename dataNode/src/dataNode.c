#include "lib/funcionesDN.h"


int main(int argc, char* argv[]) {

	TdataNode *dataNode;
	int socketFS, fd, estado;
	char *bufferHead = malloc(sizeof(Theader));
	char *mensaje = malloc(100);
	Theader *head = malloc(sizeof(Theader));
	Tbloque * bloque;
	char * archivoLogInfo = string_new();
	char * archivoLogError = string_new();

	if(argc!=2){
			printf("Error en la cantidad de parametros\n");
			return EXIT_FAILURE;
		}

	dataNode = obtenerConfiguracionDN(argv[1]);
	mostrarConfiguracion(dataNode);

	string_append_with_format(&archivoLogInfo, "/home/utnso/tp-2017-2c-Los-Ritchines/dataNode/info-%s.log", dataNode->nombre_nodo);
	string_append_with_format(&archivoLogError, "/home/utnso/tp-2017-2c-Los-Ritchines/dataNode/error-%s.log", dataNode->nombre_nodo);
	inicializarArchivoDeLogs(archivoLogInfo);
	inicializarArchivoDeLogs(archivoLogError);
	logError = log_create(archivoLogError, "dataNode", false, LOG_LEVEL_ERROR);
	logInfo = log_create(archivoLogInfo, "dataNode", true, LOG_LEVEL_INFO);
	free(archivoLogInfo);
	free(archivoLogError);

	FILE * archivo = fopen(dataNode->ruta_databin, "rb+");
	log_info(logInfo,"Se abre el databin.");

	if(archivo == NULL){
		log_info(logInfo,"No se encontro el databin en la ruta especificada, se procedera a crear el mismo");

		archivo = fopen(dataNode->ruta_databin, "wb");
		truncate(dataNode->ruta_databin, dataNode->tamanio_databin_mb * BLOQUE_SIZE);
		fclose(archivo);
		archivo = fopen(dataNode->ruta_databin, "rb+");
		log_info(logInfo,"Se creo el databin");
	}

	fd = fileno(archivo);
	if ((archivoMapeado = mmap(NULL, dataNode->tamanio_databin_mb*BLOQUE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,	fd, 0)) == MAP_FAILED) {
		logErrorAndExit("Error al hacer mmap");
	}
	fclose(archivo);
	close(fd);

	log_info(logInfo,"Intentando conectar con file system");

	socketFS = conectarAServidor(dataNode->ip_filesystem,dataNode->puerto_filesystem);

	log_info(logInfo,"Conectado con file system");

	//manda el nombre la ip y el puerto del nodo

	estado = enviarInfoNodo(socketFS, dataNode);
	log_info(logInfo,"Envie %d bytes con la informacion del nodo.",estado);	;
	log_info(logInfo,"Se envio la informacion del nodo a FILESYSTEM");

	while (1) {

		if ((estado = recv(socketFS, head, sizeof(Theader), 0)) == -1) {
			log_error(logError,"Error al recibir informacion");
			break;

		} else if (estado == 0) {
			sprintf(mensaje, "Se desconecto el socket de fd: %d\n", socketFS);
			log_error(logError, mensaje);
			break;
		}
		printf("Recibi el head %d bytes\n", estado);

		if (head->tipo_de_proceso == FILESYSTEM) {
			switch (head->tipo_de_mensaje) {
				case ALMACENAR_BLOQUE:
					log_info(logInfo,"Es FileSystem y quiere almacenar un bloque");

					bloque = recvBloque(socketFS);
					log_info(logInfo,"Voy a almacenar el bloque.");

					setBloque(bloque->nroBloque, bloque);

					log_info(logInfo,"Bloque almacenado");

					free(bloque->contenido);
					free(bloque);

					break;
				case OBTENER_BLOQUE:
					log_info(logInfo,"Es FileSystem y quiere obtener el bloque");
					int nroBloque_;
					unsigned long long int tamanioBloque_;
					if (recv(socketFS, &nroBloque_, sizeof(int), 0) == -1) {
						log_info(logInfo,"Error al recibir el numero do bloque");
					}
					if (recv(socketFS, &tamanioBloque_, sizeof(unsigned long long int),0) == -1){
						log_info(logInfo,"Error al recibir el tamanio del bloque");
					}
					enviarBloque(nroBloque_ , tamanioBloque_ ,socketFS);
					log_info(logInfo,"Se envio el bloque %d a FILESYSTEM",nroBloque_);
					break;
				default:
					log_error(logError,"Se recibio un mensaje que no esta incluido en el protocolo");
					break;
				}
			}
	}


	free(head);
	free(bufferHead);
	free(mensaje);

	return EXIT_SUCCESS;
}
