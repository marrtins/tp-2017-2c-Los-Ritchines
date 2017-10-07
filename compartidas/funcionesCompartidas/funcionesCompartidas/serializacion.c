#include "serializacion.h"

char * empaquetarRutasYamafs(Theader *head,char *rutaArchivoAReducir,char* rutaResultado){
	char *chorroBytes = malloc(sizeof(*head) + sizeof(uint32_t) + sizeof(*rutaArchivoAReducir)-1 +
								sizeof(sizeof(uint32_t)) + sizeof(rutaResultado)-1);

	char * p = chorroBytes;
	memcpy(p, head, sizeof(Theader));
	p += sizeof(Theader);
	memcpy(p, sizeof(*rutaArchivoAReducir), sizeof(uint32_t));
	p += sizeof(uint32_t);
	memcpy(p, *rutaArchivoAReducir, sizeof(*rutaArchivoAReducir));
	p += sizeof(*rutaArchivoAReducir);
	memcpy(p, sizeof(*rutaResultado), sizeof(uint32_t));
	p += sizeof(uint32_t);
	memcpy(p, *rutaResultado, sizeof(*rutaResultado));
	p += sizeof(*rutaResultado);

	return chorroBytes;
}

void desempaquetarRutasYamafs(TpackageRutas * estructuraDeRutas, int socket){

	if (recv(socket, estructuraDeRutas->tamanioRutaOrigen, sizeof(uint32_t), 0) < 0) {
		logAndExit("Error en la recepcion del header de master.");
	}
	estructuraDeRutas->rutaOrigen = malloc(estructuraDeRutas->tamanioRutaOrigen);

	if (recv(socket, estructuraDeRutas->rutaOrigen, sizeof(estructuraDeRutas->tamanioRutaOrigen), 0) < 0) {
		logAndExit("Error en la recepcion del header de master.");
	}

	if (recv(socket, estructuraDeRutas->tamanioRutaResultado, sizeof(uint32_t), 0) < 0) {
		logAndExit("Error en la recepcion del header de master.");
	}

	estructuraDeRutas->rutaResultado = malloc(estructuraDeRutas->tamanioRutaResultado);

	if (recv(socket, estructuraDeRutas->rutaResultado, sizeof(estructuraDeRutas->tamanioRutaResultado), 0) < 0) {
		logAndExit("Error en la recepcion del header de master.");
	}
}
