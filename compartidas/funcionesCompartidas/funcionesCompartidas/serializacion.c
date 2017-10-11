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


char *serializeInfoBloque(Theader head, TpackInfoBloque * infoBloque, int *pack_size){

	char *bytes_serial;

	int espacioPackSize = sizeof(int);
	int espacioEnteros = sizeof(int) * 4;
	int espaciosVariables = infoBloque->nombreLen+infoBloque->nombreTemporalLen;
	int espacioAMallocar = HEAD_SIZE + espacioPackSize+espacioEnteros+espaciosVariables;

	if ((bytes_serial = malloc(espacioAMallocar)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}

	*pack_size = 0;
	memcpy(bytes_serial + *pack_size, &head, HEAD_SIZE);
	*pack_size += HEAD_SIZE;

	// hacemos lugar para el payload_size
	*pack_size += sizeof(int);



	memcpy(bytes_serial + *pack_size, &infoBloque->nombreLen, sizeof(int));
	*pack_size += sizeof(int);

	memcpy(bytes_serial + *pack_size, infoBloque->nombreNodo, infoBloque->nombreLen);
	*pack_size += infoBloque->nombreLen;





	memcpy(bytes_serial + *pack_size, &infoBloque->bloque, sizeof(int));
	*pack_size += sizeof(int);


	memcpy(bytes_serial + *pack_size, &infoBloque->bytesOcupados, sizeof(int));
	*pack_size += sizeof(int);


	memcpy(bytes_serial + *pack_size, &infoBloque->nombreTemporalLen, sizeof(int));
	*pack_size += sizeof(int);
	memcpy(bytes_serial + *pack_size, infoBloque->nombreTemporal, infoBloque->nombreTemporalLen);
	*pack_size += infoBloque->nombreTemporalLen;


	memcpy(bytes_serial + HEAD_SIZE, pack_size, sizeof(int));

	return bytes_serial;
}


TpackBytes *deserializeInfoBloque(char *bytes_serial){

	int off;
	TpackInfoBloque *infoBloque;

	if ((infoBloque = malloc(sizeof *infoBloque)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}

	off = 0;

	memcpy(&infoBloque->nombreLen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((infoBloque->nombreNodo = malloc(infoBloque->nombreLen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", infoBloque->nombreLen);
		return NULL;
	}

	memcpy(infoBloque->nombreNodo, bytes_serial + off, infoBloque->nombreLen);
	off += infoBloque->nombreLen;






	memcpy(&infoBloque->bloque, bytes_serial + off, sizeof (int));
	off += sizeof (int);


	memcpy(&infoBloque->bytesOcupados, bytes_serial + off, sizeof (int));
	off += sizeof (int);



	memcpy(&infoBloque->nombreTemporalLen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((infoBloque->nombreTemporal = malloc(infoBloque->nombreTemporalLen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", infoBloque->nombreTemporalLen);
		return NULL;
	}

	memcpy(infoBloque->nombreTemporal, bytes_serial + off, infoBloque->nombreTemporalLen);
	off += infoBloque->nombreLen;

	return infoBloque;
}



char *serializeBytes(Theader head, char* buffer, int buffer_size, int *pack_size){

	char *bytes_serial;
	if ((bytes_serial = malloc(HEAD_SIZE + sizeof(int) + sizeof(int) + buffer_size)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}

	*pack_size = 0;
	memcpy(bytes_serial + *pack_size, &head, HEAD_SIZE);
	*pack_size += HEAD_SIZE;

	// hacemos lugar para el payload_size
	*pack_size += sizeof(int);

	memcpy(bytes_serial + *pack_size, &buffer_size, sizeof buffer_size);
	*pack_size += sizeof (int);
	memcpy(bytes_serial + *pack_size, buffer, buffer_size);
	*pack_size += buffer_size;

	memcpy(bytes_serial + HEAD_SIZE, pack_size, sizeof(int));

	return bytes_serial;
}


TpackBytes *deserializeBytes(char *bytes_serial){

	int off;
	TpackBytes *pbytes;

	if ((pbytes = malloc(sizeof *pbytes)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}

	off = 0;
	memcpy(&pbytes->bytelen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((pbytes->bytes = malloc(pbytes->bytelen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", pbytes->bytelen);
		return NULL;
	}

	memcpy(pbytes->bytes, bytes_serial + off, pbytes->bytelen);
	off += pbytes->bytelen;

	return pbytes;
}

