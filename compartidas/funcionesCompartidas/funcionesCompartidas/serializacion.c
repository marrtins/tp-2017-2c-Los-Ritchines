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
		int espacioEnteros = sizeof(int) * 6;
		int espaciosVariables = infoBloque->tamanioIp+infoBloque->tamanioNombre+infoBloque->nombreTemporalLen+infoBloque->tamanioPuerto;
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



		memcpy(bytes_serial + *pack_size, &infoBloque->tamanioNombre, sizeof(int));
		*pack_size += sizeof(int);

		memcpy(bytes_serial + *pack_size, infoBloque->nombreNodo, infoBloque->tamanioNombre);
		*pack_size += infoBloque->tamanioNombre;



		memcpy(bytes_serial + *pack_size, &infoBloque->tamanioIp, sizeof(int));
			*pack_size += sizeof(int);

		memcpy(bytes_serial + *pack_size, infoBloque->ipWorker, infoBloque->tamanioIp);
		*pack_size += infoBloque->tamanioIp;



		memcpy(bytes_serial + *pack_size, &infoBloque->tamanioPuerto, sizeof(int));
			*pack_size += sizeof(int);

		memcpy(bytes_serial + *pack_size, infoBloque->puertoWorker, infoBloque->tamanioPuerto);
		*pack_size += infoBloque->tamanioPuerto;


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


TpackInfoBloque *deserializeInfoBloque(char *bytes_serial){

	int off;
		TpackInfoBloque *infoBloque;

		if ((infoBloque = malloc(sizeof *infoBloque)) == NULL){
			fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
			return NULL;
		}

		off = 0;

		memcpy(&infoBloque->tamanioNombre, bytes_serial + off, sizeof (int));
		off += sizeof (int);

		if ((infoBloque->nombreNodo = malloc(infoBloque->tamanioNombre)) == NULL){
			printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", infoBloque->tamanioNombre);
			return NULL;
		}

		memcpy(infoBloque->nombreNodo, bytes_serial + off, infoBloque->tamanioNombre);
		off += infoBloque->tamanioNombre;



		memcpy(&infoBloque->tamanioIp, bytes_serial + off, sizeof (int));
		off += sizeof (int);

		if ((infoBloque->ipWorker = malloc(infoBloque->tamanioIp)) == NULL){
			printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", infoBloque->tamanioIp);
			return NULL;
		}

		memcpy(infoBloque->ipWorker, bytes_serial + off, infoBloque->tamanioIp);
		off += infoBloque->tamanioIp;





		memcpy(&infoBloque->tamanioPuerto, bytes_serial + off, sizeof (int));
		off += sizeof (int);

		if ((infoBloque->puertoWorker = malloc(infoBloque->tamanioPuerto)) == NULL){
			printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", infoBloque->tamanioPuerto);
			return NULL;
		}

		memcpy(infoBloque->puertoWorker, bytes_serial + off, infoBloque->tamanioPuerto);
		off += infoBloque->tamanioPuerto;




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
		off += infoBloque->tamanioNombre;

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

TpackSrcCode *readFileIntoPack(Tproceso sender, char* ruta){

	FILE *file = fopen(ruta, "rb");
	TpackSrcCode *src_code = malloc(sizeof *src_code);
	src_code->head.tipo_de_proceso = sender;
	src_code->head.tipo_de_mensaje = SRC_CODE;

	unsigned long fileSize = fsize(file) + 1 ; // + 1 para el '\0'
	printf("fsize es: %lu",fileSize);
	src_code->bytelen = fileSize;
	src_code->bytes = malloc(src_code->bytelen);
	fread(src_code->bytes, src_code->bytelen, 1, file);
	fclose(file);
	// ponemos un '\0' al final porque es probablemente mandatorio para que se lea, send'ee y recv'ee bien despues
	src_code->bytes[src_code->bytelen - 1] = '\0';

	return src_code;
}

unsigned long fsize(FILE* f){

    fseek(f, 0, SEEK_END);
    unsigned long len = (unsigned long) ftell(f);
    fseek(f, 0, SEEK_SET);
    return len;

}

Tbuffer *empaquetarBloque(Theader * head, int nroBloque, unsigned long long tamanio, char *contenido){

	Tbuffer *buffer = malloc(sizeof(Tbuffer));
	buffer->tamanio = (HEAD_SIZE + sizeof(int) + sizeof(unsigned long long) + tamanio);
	char *chorroBytes = malloc(buffer->tamanio);

	char * p = chorroBytes;
	memcpy(p, head, sizeof(*head));
	p += sizeof(*head);
	memcpy(p, &nroBloque, sizeof(int));
	p += sizeof(int);
	memcpy(p, &tamanio, sizeof(unsigned long long));
	p += sizeof(unsigned long long);
	memcpy(p, contenido, tamanio);
	p += tamanio;

	buffer->buffer = malloc(buffer->tamanio);
	buffer->buffer = chorroBytes;

	return buffer;
}

Tbuffer * empaquetarInfoBloqueDNaFS(TpackInfoBloqueDN * infoBloque){

		Tbuffer *buffer = malloc(sizeof(Tbuffer));
		int espacioEnteros = sizeof(int) * 4;
		int espaciosVariables = infoBloque->tamanioIp + infoBloque->tamanioPuerto + infoBloque->tamanioNombre;
		buffer->tamanio = HEAD_SIZE + espacioEnteros + espaciosVariables;

		char * chorroBytes = malloc(buffer->tamanio);
		char * p = chorroBytes;

		memcpy(p, &infoBloque->head, sizeof(infoBloque->head));
		p += sizeof(infoBloque->head);
		memcpy(p, &infoBloque->tamanioNombre, sizeof(int));
		p += sizeof(int);
		memcpy(p, infoBloque->nombreNodo, infoBloque->tamanioNombre);
		p += infoBloque->tamanioNombre;
		memcpy(p, &infoBloque->tamanioIp, sizeof(int));
		p += sizeof(int);
		memcpy(p, infoBloque->ipNodo, infoBloque->tamanioIp);
		p += infoBloque->tamanioIp;
		memcpy(p, &infoBloque->tamanioPuerto, sizeof(int));
		p += sizeof(int);
		memcpy(p, infoBloque->puertoNodo, infoBloque->tamanioPuerto);
		p += infoBloque->tamanioPuerto;
		memcpy(p, &infoBloque->databinEnMB, sizeof(int));
		p += sizeof(int);

		buffer->buffer = malloc(buffer->tamanio);
		buffer->buffer = chorroBytes;
		return buffer;

}

TpackInfoBloqueDN * desempaquetarInfoNodo(TpackInfoBloqueDN * infoBloque, char * nombreNodo, char * ipNodo, char * puertoNodo){

	infoBloque->nombreNodo = malloc(infoBloque->tamanioNombre);
	infoBloque->ipNodo = malloc(infoBloque->tamanioIp);
	infoBloque->puertoNodo = malloc(infoBloque->tamanioPuerto);

	memcpy(infoBloque->nombreNodo, nombreNodo, infoBloque->tamanioNombre);
	memcpy(infoBloque->ipNodo, ipNodo, infoBloque->tamanioIp);
	memcpy(infoBloque->puertoNodo, puertoNodo, infoBloque->tamanioPuerto);

	return infoBloque;

}

char *serializarInfoTransformacionMasterWorker(Theader head,int nroBloque, int bytesOcupadosBloque,int nombreTemporalLen ,char* nombreTemporal, int *pack_size){

	char *bytes_serial;
	if ((bytes_serial = malloc(HEAD_SIZE + sizeof(int) + sizeof(int)*3 + nombreTemporalLen)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}

	*pack_size = 0;
	memcpy(bytes_serial + *pack_size, &head, HEAD_SIZE);
	*pack_size += HEAD_SIZE;

	// hacemos lugar para el payload_size
	*pack_size += sizeof(int);

	memcpy(bytes_serial + *pack_size, &nroBloque, sizeof (int));
	*pack_size += sizeof (int);

	memcpy(bytes_serial + *pack_size, &bytesOcupadosBloque, sizeof (int));
	*pack_size += sizeof (int);

	memcpy(bytes_serial + *pack_size, &nombreTemporalLen, sizeof (int));
	*pack_size += sizeof (int);

	memcpy(bytes_serial + *pack_size, nombreTemporal, nombreTemporalLen);
	*pack_size += nombreTemporalLen;

	memcpy(bytes_serial + HEAD_SIZE, pack_size, sizeof(int));

	printf("Pack size: %d\n",*pack_size);
	return bytes_serial;
}


TpackDatosTransformacion *deserializarInfoTransformacionMasterWorker(char *bytes_serial){

	int off;
	TpackDatosTransformacion *datosTransf;

	if ((datosTransf = malloc(sizeof *datosTransf)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete datos transf\n");
		return NULL;
	}

	off = 0;
	memcpy(&datosTransf->nroBloque, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	memcpy(&datosTransf->bytesOcupadosBloque, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	memcpy(&datosTransf->nombreTemporalLen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((datosTransf->nombreTemporal = malloc(datosTransf->nombreTemporalLen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", datosTransf->nombreTemporalLen);
		return NULL;
	}

	memcpy(datosTransf->nombreTemporal, bytes_serial + off, datosTransf->nombreTemporalLen);
	off += datosTransf->nombreTemporalLen;

	return datosTransf;
}
