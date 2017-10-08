#include "serializacion.h"

char * empaquetarRutasYamafs(Theader *head,char *rutaArchivoAReducir,char* rutaResultado){
	puts("Creando estructura de empaquetacion.");
	char *chorroBytes = malloc(sizeof(*head) + sizeof(uint32_t) + strlen(rutaArchivoAReducir) +
								sizeof(sizeof(uint32_t)) + strlen(rutaResultado));

	uint32_t t1 = strlen(rutaArchivoAReducir);
	uint32_t t2 = strlen(rutaResultado);

	char * p = chorroBytes;
	memcpy(p, head, sizeof(*head));
	p += sizeof(*head);
	memcpy(p, &t1, sizeof(uint32_t));
	p += sizeof(uint32_t);
	memcpy(p, rutaArchivoAReducir, strlen(rutaArchivoAReducir));
	p += strlen(rutaArchivoAReducir);
	memcpy(p, &t2, sizeof(uint32_t));
	p += sizeof(uint32_t);
	memcpy(p, rutaResultado, strlen(rutaResultado));
	p += strlen(rutaResultado);
	puts("estructura creada y lista para mandar");
	return chorroBytes;
}

void desempaquetarRutasYamafs(TpackageRutas * estructuraDeRutas, int socket){

	if (recv(socket, &estructuraDeRutas->tamanioRutaOrigen, sizeof(uint32_t), 0) < 0) {
		logAndExit("Error en la recepcion del header de master.");
	}
	estructuraDeRutas->rutaOrigen = malloc(estructuraDeRutas->tamanioRutaOrigen);

	if (recv(socket, &estructuraDeRutas->rutaOrigen, sizeof(estructuraDeRutas->tamanioRutaOrigen), 0) < 0) {
		logAndExit("Error en la recepcion del header de master.");
	}

	if (recv(socket, &estructuraDeRutas->tamanioRutaResultado, sizeof(uint32_t), 0) < 0) {
		logAndExit("Error en la recepcion del header de master.");
	}

	estructuraDeRutas->rutaResultado = malloc(estructuraDeRutas->tamanioRutaResultado);

	if (recv(socket, &estructuraDeRutas->rutaResultado, sizeof(estructuraDeRutas->tamanioRutaResultado), 0) < 0) {
		logAndExit("Error en la recepcion del header de master.");
	}
}
