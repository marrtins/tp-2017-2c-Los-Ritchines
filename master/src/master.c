#include "lib/funciones.h"

int main(int argc, char* argv[]) {

	int socketAYama,
		socketAWorker,
		cantidadBytesEnviados,
		puertoWorker,
		ipWorker;
	Tmaster *master;
	Theader * head = malloc(HEAD_SIZE);
	char *rutaTransformador = string_new();
	char *rutaReductor = string_new();
	char *rutaArchivoAReducir = string_new();
	char *rutaResultado = string_new();

	// arg[0]: nombre de la funcion
	// arg[1]: ruta transformador
	// arg[2]: ruta reductor
	// arg[3]: ruta del archivo dentro del filesystem
	// arg[4]: ruta de destino del archivo final
	if(argc != 1){
		puts("Error en la cantidad de parametros.");
	}

	logger = log_create("master.log", "master.log", false, LOG_LEVEL_INFO);

	head->tipo_de_proceso = MASTER;
	head->tipo_de_mensaje = 22;

	/*rutaTransformador=argv[1];
	rutaReductor=argv[2];
	rutaArchivoAReducir=argv[3];
	rutaResultado=argv[4];*/

	master = obtenerConfiguracion("/home/utnso/buenasPracticas/master/config_master");
	mostrarConfiguracion(master);

	socketAYama = conectarAServidor(master->ipYama, master->puertoYama);

	cantidadBytesEnviados = enviarHeader(socketAYama, head);

	/*if (send(socketAYama, rutaTransformador, strlen(rutaTransformador) +1, 0) == -1){
				puts("No se pudo enviar codigo fuente de transformador a YAMA. ");
				return  FALLO_SEND;
			}

	if (send(socketAYama, rutaReductor, strlen(rutaReductor) +1, 0) == -1){
			puts("No se pudo enviar codigo fuente de REDUCTOR a YAMA. ");
			return  FALLO_SEND;
		}

	if (send(socketAYama, rutaArchivoAReducir, strlen(rutaArchivoAReducir) +1, 0) == -1){
			puts("No se pudo enviar la ruta del archivo a reducir a YAMA. ");
			return  FALLO_SEND;
		}

		if (send(socketAYama, rutaResultado, strlen(rutaResultado) +1, 0) == -1){
				puts("No se pudo enviar la ruta del archivo reducido a YAMA. ");
				return  FALLO_SEND;
			}*/

	//YAMA nos envia toda la info para conectarnos a los workers
	while ((recv(socketAYama, head, HEAD_SIZE, 0)) > 0) {

		puts("Recibimos un paquete de YAMA");

		switch (head->tipo_de_mensaje) {
		case (START_LOCALTRANSF):
			//Aca llegaria la respuesta de yama con la info sobre a que workers conectarnos
			//y nos dice los bloques donde hay que aplicar la transformacion
			//aca creamos un hilo por cada worker al que tenemos que conectarnos.

			break;
		case (INFO_NODO):
			puts("El mensaje contiene la información del nodo");
			puts("Nos conectamos a worker");
			socketAWorker = conectarAServidor("127.0.0.1",	"5050");

			head->tipo_de_proceso = MASTER;
			head->tipo_de_mensaje = 0;

			cantidadBytesEnviados = enviarHeader(socketAWorker,head);
			printf("Se envian %d bytes a Worker\n",cantidadBytesEnviados);

			break;
		default:
			printf("Proceso: %d \n", head->tipo_de_proceso);
			printf("Mensaje: %d \n", head->tipo_de_mensaje);
			break;
		}

	}

	free(head);
	return EXIT_SUCCESS;
}
