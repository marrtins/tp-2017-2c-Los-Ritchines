#include "lib/funcionesMS.h"

int main(int argc, char* argv[]) {

	int sockYama,
		cantidadBytesEnviados,
		puertoWorker,
		ipWorker,
		packSize,
		stat;
	Tmaster *master;
	Theader * head = malloc(HEAD_SIZE);

	char *rutaTransformador = string_new();
	char *rutaReductor = string_new();
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

	master = obtenerConfiguracion("/home/utnso/tp-2017-2c-Los-Ritchines/master/config_master");
	mostrarConfiguracion(master);

	printf("Transformador Path: %s\n",rutaTransformador);
	printf("Reductor Path: %s\n",rutaReductor);
	printf("Archivo a reducir Path: %s\n",rutaArchivoAReducir);
	printf("Resultado Path: %s\n",rutaResultado);


	sockYama = conectarAServidor(master->ipYama, master->puertoYama);
	cantidadBytesEnviados = enviarHeader(sockYama, head);

	puts("Enviamos a YAMA las rutas a reducir y almacenar");
	head.tipo_de_proceso = MASTER; head.tipo_de_mensaje = PATH_FILE_TOREDUCE ;packSize = 0;
	buffer=serializeBytes(head,rutaArchivoAReducir,(strlen(rutaArchivoAReducir)+1),&packSize);
	puts("Path del archivo a reducir serializado; lo enviamos");


	if ((stat = send(sockYama, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar Path del archivo a reducir a YAMA. ");
		return  FALLO_SEND;
	}
	printf("se enviaron %d bytes del Path del archivo a reducir a YAMA\n",stat);

	//enviamos el path del resultado
	head.tipo_de_proceso = MASTER; head.tipo_de_mensaje = PATH_RES_FILE ;packSize = 0;
	buffer=serializeBytes(head,rutaResultado,(strlen(rutaResultado)+1),&packSize);
	puts("Path del resultado serializado; lo enviamos");


	if ((stat = send(sockYama, buffer, packSize, 0)) == -1){
		puts("no se pudo enviar Path del aresultado a YAMA. ");
		return  FALLO_SEND;
	}
	printf("se enviaron %d bytes del Path del resultado a YAMA\n",stat);


	while ((recv(sockYama, head, HEAD_SIZE, 0)) > 0) {

		puts("Recibimos un paquete de YAMA");

		switch (head->tipo_de_mensaje) {
		case (START_LOCALTRANSF):
			//Aca llegaria la respuesta de yama con la info sobre a que workers conectarnos
			//y nos dice los bloques donde hay que aplicar la transformacion
			//aca creamos un hilo por cada worker al que tenemos que conectarnos.


			break;
//		case (INFO_NODO):
//			puts("El mensaje contiene la información del nodo");
//			puts("Nos conectamos a worker");
//			//sockWorker = conectarAServidor("127.0.0.1",	"5050");
//
//			head->tipo_de_proceso = MASTER;
//			head->tipo_de_mensaje = 0;
//
//			cantidadBytesEnviados = enviarHeader(socketAWorker,head);
//			printf("Se envian %d bytes a Worker\n",cantidadBytesEnviados);
//
//			break;
		default:
			printf("Proceso: %d \n", head->tipo_de_proceso);
			printf("Mensaje: %d \n", head->tipo_de_mensaje);
			break;
		}

	}

	free(head);
	freeAndNULL((void **) &buffer);
	return EXIT_SUCCESS;
}
