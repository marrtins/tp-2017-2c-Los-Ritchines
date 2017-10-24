#include "lib/funcionesMS.h"
#include <sys/sendfile.h>
#include <fcntl.h>


char * rutaTransformador, * rutaReductor;






int main(int argc, char* argv[]) {

	int sockYama,

	//sockWorker,
		cantidadBytesEnviados,
	//	puertoWorker,
	//	ipWorker,
		packSize,
		stat;
	//	cantBytes;
	Tmaster *master;
	Theader * head = malloc(sizeof(Theader));
	Theader headTmp;
	//char * chorroDeBytes;
	t_list *bloquesTransformacion = list_create();

	rutaTransformador = string_new();
	rutaReductor = string_new();
	char *rutaArchivoAReducir = string_new();
	char *rutaResultado = string_new();
	char *buffer;

	if(argc != 5){
			puts("Error en la cantidad de parametros.");
	}



	logger = log_create("master.log", "master.log", false, LOG_LEVEL_INFO);

	head->tipo_de_proceso = MASTER;
	head->tipo_de_mensaje = INICIOMASTER;


	// arg[0]: nombre de la funcion
	// arg[1]: ruta transformador
	// arg[2]: ruta reductor
	// arg[3]: ruta del archivo dentro del filesystem
	// arg[4]: ruta de destino del archivo final


	rutaTransformador=argv[1];

	rutaReductor=argv[2];

	rutaArchivoAReducir=argv[3];
	rutaResultado=argv[4];

	master = obtenerConfiguracionMaster("/home/utnso/tp-2017-2c-Los-Ritchines/master/config_master");
	mostrarConfiguracion(master);

	printf("Transformador Path: %s\n",rutaTransformador);
	printf("Reductor Path: %s\n",rutaReductor);
	printf("Archivo a reducir Path: %s\n",rutaArchivoAReducir);
	printf("Resultado Path: %s\n",rutaResultado);




	sockYama = conectarAServidor(master->ipYama, master->puertoYama);
	cantidadBytesEnviados = enviarHeader(sockYama, head);

	puts("Enviamos a YAMA las rutas a reducir y almacenar");

	headTmp.tipo_de_proceso = MASTER;
	headTmp.tipo_de_mensaje = PATH_FILE_TOREDUCE ;
	packSize = 0;
	buffer=serializeBytes(headTmp,rutaArchivoAReducir,(strlen(rutaArchivoAReducir)+1),&packSize);
	puts("Path del archivo a reducir serializado; lo enviamos");

	//cantidadBytesEnviados = enviarHeader(sockYama, head);

	if ((stat = send(sockYama, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar Path del archivo a reducir a YAMA. ");
		return  FALLO_SEND;
	}
	printf("se enviaron %d bytes del Path del archivo a reducir a YAMA\n",stat);


	headTmp.tipo_de_proceso = MASTER; headTmp.tipo_de_mensaje = PATH_RES_FILE ;packSize = 0;
	buffer=serializeBytes(headTmp,rutaResultado,(strlen(rutaResultado)+1),&packSize);
	puts("Path del resultado serializado; lo enviamos");


	if ((stat = send(sockYama, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar Path del aresultado a YAMA. ");
		return  FALLO_SEND;
	}
	printf("se enviaron %d bytes del Path del resultado a YAMA\n",stat);

	Theader headRcv = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};



	while ((stat=recv(sockYama, &headRcv, HEAD_SIZE, 0)) > 0) {


		puts("Recibimos un paquete de YAMA");

		TpackInfoBloque *infoBloque;

		switch (headRcv.tipo_de_mensaje) {

		case (INFOBLOQUE):
			puts("Nos llega info de un bloque");
			if ((buffer = recvGenericWFlags(sockYama,MSG_WAITALL)) == NULL){
				puts("Fallo recepcion de INFOBLOQUE");
				return -1;
			}

			if ((infoBloque = deserializeInfoBloque(buffer)) == NULL){
				puts("Fallo deserializacion de Bytes del path_res_file");
				return -1;
			}
			printf("Nos llego info del bloque del archivo %d, en el databin %d \n",infoBloque->bloqueDelArchivo,infoBloque->bloqueDelDatabin);
			printf("Nombre nodo;IPNodo;PuertoNodo;Bloque;BytesOcupados;NombreArchivotemporal\n");
			printf("%s,%s:%s,%d,%d,%s\n",infoBloque->nombreNodo,infoBloque->ipWorker,infoBloque->puertoWorker,infoBloque->bloqueDelDatabin,
											infoBloque->bytesOcupados,infoBloque->nombreTemporal);

			list_add(bloquesTransformacion,infoBloque);
			break;



		case (INFOULTIMOBLOQUE):
			puts("Nos llega info del ultimo bloque relacionado con el archivo a reducir");
			if ((buffer = recvGenericWFlags(sockYama,MSG_WAITALL)) == NULL){
				puts("Fallo recepcion de INFOBLOQUE");
				return -1;
			}
			if ((infoBloque = deserializeInfoBloque(buffer)) == NULL){
				puts("Fallo deserializacion de Bytes del path_res_file");
				return -1;
			}
			printf("Nos llego info del bloque del archivo %d, en el databin %d \n",infoBloque->bloqueDelArchivo,infoBloque->bloqueDelDatabin);
			printf("Nombre nodo;IPNodo;PuertoNodo;Bloque;BytesOcupados;NombreArchivotemporal\n");
			printf("%s,%s:%s,%d,%d,%s\n",infoBloque->nombreNodo,infoBloque->ipWorker,infoBloque->puertoWorker,infoBloque->bloqueDelDatabin,
								infoBloque->bytesOcupados,infoBloque->nombreTemporal);
			list_add(bloquesTransformacion,infoBloque);
			printf("Ya nos llego toda la info relacionada al archivo a transformar. Cantidad de bloques a leer: %d\n",list_size(bloquesTransformacion));

			stat = conectarseAWorkersTransformacion(bloquesTransformacion,sockYama);


			break;

		default:
			printf("Proceso: %d \n", headTmp.tipo_de_proceso);
			printf("Mensaje: %d \n", headTmp.tipo_de_mensaje);
			break;
		}

	}

	free(head);
	freeAndNULL((void **) &buffer);
	return EXIT_SUCCESS;
}

