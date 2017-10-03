#include "serializacion.h"

char * empaquetarRutasYamafs(Theader *head,char *rutaArchivoAReducir,char* rutaResultado){
	char *chorroBytes = malloc(sizeof(*head) + sizeof(int) + sizeof(*rutaArchivoAReducir)-1 +
								sizeof(int) + sizeof(rutaResultado)-1);

	char * p = chorroBytes;
	memcpy(p, head, sizeof(Theader));
	p += sizeof(Theader);
	memcpy(p, sizeof(*rutaArchivoAReducir), sizeof(int));
	p += sizeof(int);
	memcpy(p, *rutaArchivoAReducir, sizeof(*rutaArchivoAReducir));
	p += sizeof(*rutaArchivoAReducir);
	memcpy(p, sizeof(*rutaResultado), sizeof(int));
	p += sizeof(int);
	memcpy(p, *rutaResultado, sizeof(*rutaResultado));
	p += sizeof(*rutaResultado);

	return chorroBytes;
}
