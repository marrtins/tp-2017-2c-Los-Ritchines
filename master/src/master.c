#include "lib/funcionesMS.h"

int main(int argc, char* argv[]) {

	int socketAYama,
		socketAWorker,
		cantidadBytesEnviados,
		puertoWorker,
		ipWorker,
		cantBytes;
	Tmaster *master;
	Theader * head = malloc(sizeof(Theader));
	char * chorroDeBytes;
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

	rutaTransformador=argv[1];
	rutaReductor=argv[2];
	//hardcodeado
	rutaArchivoAReducir = "ruta/hardcodeada/a/reducir";
	rutaResultado = "ruta/harcodeada/destino";
	//rutaArchivoAReducir=argv[3];
	//rutaResultado=argv[4];

	master = obtenerConfiguracionMaster("/home/utnso/tp-2017-2c-Los-Ritchines/master/config_master");
	mostrarConfiguracion(master);

	socketAYama = conectarAServidor(master->ipYama, master->puertoYama);
	printf("SocketAYama es %d\n",socketAYama);
	puts("Conectado a Yama");
	head->tipo_de_proceso = MASTER;
	head->tipo_de_mensaje = INICIOMASTER;

	//chorroDeBytes = empaquetarRutasYamafs(head, rutaArchivoAReducir, rutaResultado);
	puts("Por empaquetar...");
	chorroDeBytes = empaquetarRutasYamafs(head, rutaArchivoAReducir, rutaResultado);
	puts("Se empaqueto");
	//printf("Empaquetacion terminada. Se empaqueto: %s\n", chorroDeBytes);

	cantBytes = (sizeof(Theader) + sizeof(uint32_t) + strlen(rutaArchivoAReducir) +	sizeof(sizeof(uint32_t)) + strlen(rutaResultado));
	printf("Se va a enviar con un tamaño de %d\n",cantBytes);

	if ((cantidadBytesEnviados = send(socketAYama, chorroDeBytes, cantBytes, 0)) == -1){
			logAndExit("Fallo al enviar el header");
		}
	printf("Se enviaron %d\n",cantidadBytesEnviados);

	//YAMA nos envia toda la info para conectarnos a los workers
	while ((recv(socketAYama, head, sizeof(Theader), 0)) > 0) {

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
			head->tipo_de_mensaje = INICIOMASTER;

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
