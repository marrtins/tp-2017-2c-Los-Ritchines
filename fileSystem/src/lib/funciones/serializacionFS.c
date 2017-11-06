#include "../funcionesFS.h"

void empaquetarBloqueAEliminar(Tbuffer * buffer, Theader * head, int numeroDeBloque){
	buffer->tamanio = sizeof(head) + sizeof(numeroDeBloque);
	buffer->buffer = malloc(buffer->tamanio);
	char * p = buffer->buffer;
	memcpy(p, head, sizeof(*head));
	p += sizeof(*head);
	memcpy(p, &numeroDeBloque, sizeof(numeroDeBloque));
}
