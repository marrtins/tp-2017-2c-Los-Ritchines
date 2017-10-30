#include "serializacion.h"


char *serializeInfoBloque(Theader head, TpackInfoBloque * infoBloque, int *pack_size){

	char *bytes_serial;

	int espacioPackSize = sizeof(int);
	int espacioEnteros = sizeof(int) * 8;
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

	memcpy(bytes_serial + *pack_size, &infoBloque->idTarea, sizeof(int));
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


	memcpy(bytes_serial + *pack_size, &infoBloque->bloqueDelArchivo, sizeof(int));
	*pack_size += sizeof(int);

	memcpy(bytes_serial + *pack_size, &infoBloque->bloqueDelDatabin, sizeof(int));
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

		memcpy(&infoBloque->idTarea, bytes_serial + off, sizeof (int));
		off += sizeof (int);

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




		memcpy(&infoBloque->bloqueDelArchivo, bytes_serial + off, sizeof (int));
		off += sizeof (int);

		memcpy(&infoBloque->bloqueDelDatabin, bytes_serial + off, sizeof (int));
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

Tbuffer *empaquetarBloque(Theader * head, TbloqueAEnviar* bloque, Tnodo* nodo){

	Tbuffer *buffer = malloc(sizeof(Tbuffer));
	buffer->tamanio = (HEAD_SIZE + sizeof(int) + sizeof(unsigned long long) + bloque->tamanio);
	buffer->buffer = malloc(buffer->tamanio);

	puts("rompe1");
	char * p = buffer->buffer;
	memcpy(p, head, sizeof(*head));
	p += sizeof(*head);
	memcpy(p, &nodo->primerBloqueLibreBitmap, sizeof(int));
	p += sizeof(int);
	memcpy(p, &bloque->tamanio, sizeof(unsigned long long));
	p += sizeof(unsigned long long);
	memcpy(p, bloque->contenido, bloque->tamanio);
	p += bloque->tamanio;
	puts("rompe2");

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

	printf("Pack size info transf master worker: %d\n",*pack_size);
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

int enviarHeaderYValor(Theader head, int valorAEnviar,int socketDestino){

	char *bytes_serial;
	int pack_size;
	int estado;
	if ((bytes_serial = malloc(HEAD_SIZE + sizeof(int) + sizeof(int))) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return -1;
	}

	pack_size = 0;
	memcpy(bytes_serial + pack_size, &head, HEAD_SIZE);
	pack_size += HEAD_SIZE;

	// hacemos lugar para el payload_size
	pack_size += sizeof(int);

	memcpy(bytes_serial + pack_size, &valorAEnviar, sizeof (int));
	pack_size += sizeof (int);


	memcpy(bytes_serial + HEAD_SIZE, &pack_size, sizeof(int));

	if ((estado = send(socketDestino, bytes_serial, pack_size, 0)) == -1){
		logAndExit("Fallo al enviar el header");
	}


	return estado;
}

int recibirValor(int fd){
	int valorRet;
	char* buffer=recvGeneric(fd);
	memcpy(&valorRet,buffer, sizeof (int));
	free(buffer);
	return valorRet;
}


char *serializeInfoReduccionLocal(Theader head, TreduccionLocal * infoReduccion, int *pack_size){
	char *bytes_serial;
	int i;
	int espacioPackSize = sizeof(int);
	int espacioEnteros = sizeof(int) * 7;
	int espacioLista;

	int sizeLista = list_size(infoReduccion->listaTemporalesTransformacion);
	for(i=0;i< sizeLista;i++){
		TreduccionLista * aux = list_get(infoReduccion->listaTemporalesTransformacion,i);
		espacioLista += aux->nombreTemporalLen;
	}
	espacioLista+=sizeof(int)*sizeLista;


	int espaciosVariables = infoReduccion->ipLen+infoReduccion->puertoLen+infoReduccion->nombreNodoLen+infoReduccion->tempRedLen+espacioLista;
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

	memcpy(bytes_serial + *pack_size, &infoReduccion->job, sizeof(int));
	*pack_size += sizeof(int);

	memcpy(bytes_serial + *pack_size, &infoReduccion->idTarea, sizeof(int));
	*pack_size += sizeof(int);


	memcpy(bytes_serial + *pack_size, &infoReduccion->nombreNodoLen, sizeof(int));
	*pack_size += sizeof(int);
	memcpy(bytes_serial + *pack_size, infoReduccion->nombreNodo, infoReduccion->nombreNodoLen);
	*pack_size += infoReduccion->nombreNodoLen;



	memcpy(bytes_serial + *pack_size, &infoReduccion->ipLen, sizeof(int));
	*pack_size += sizeof(int);
	memcpy(bytes_serial + *pack_size, infoReduccion->ipNodo, infoReduccion->ipLen);
	*pack_size += infoReduccion->ipLen;



	memcpy(bytes_serial + *pack_size, &infoReduccion->puertoLen, sizeof(int));
	*pack_size += sizeof(int);
	memcpy(bytes_serial + *pack_size, infoReduccion->puertoNodo, infoReduccion->puertoLen);
	*pack_size += infoReduccion->puertoLen;



	memcpy(bytes_serial + *pack_size, &infoReduccion->tempRedLen, sizeof(int));
	*pack_size += sizeof(int);
	memcpy(bytes_serial + *pack_size, infoReduccion->tempRed, infoReduccion->tempRedLen);
	*pack_size += infoReduccion->tempRedLen;


	memcpy(bytes_serial + *pack_size, &infoReduccion->listaSize, sizeof(int));
	*pack_size += sizeof(int);

	for(i=0;i<sizeLista;i++){
		TreduccionLista * aux = list_get(infoReduccion->listaTemporalesTransformacion,i);
		memcpy(bytes_serial + *pack_size, &aux->nombreTemporalLen, sizeof(int));
		*pack_size += sizeof(int);
		memcpy(bytes_serial + *pack_size, aux->nombreTemporal, aux->nombreTemporalLen);
		*pack_size += aux->nombreTemporalLen;
	}


	memcpy(bytes_serial + HEAD_SIZE, pack_size, sizeof(int));

	return bytes_serial;
}

TreduccionLocal *deserializeInfoReduccionLocal(char *bytes_serial){

	int off;
	TreduccionLocal *infoReduccion;

	if ((infoReduccion = malloc(sizeof *infoReduccion)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}

	off = 0;

	memcpy(&infoReduccion->job, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	memcpy(&infoReduccion->idTarea, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	memcpy(&infoReduccion->nombreNodoLen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((infoReduccion->nombreNodo = malloc(infoReduccion->nombreNodoLen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", infoReduccion->nombreNodoLen);
		return NULL;
	}

	memcpy(infoReduccion->nombreNodo, bytes_serial + off, infoReduccion->nombreNodoLen);
	off += infoReduccion->nombreNodoLen;



	memcpy(&infoReduccion->ipLen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((infoReduccion->ipNodo = malloc(infoReduccion->ipLen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", infoReduccion->ipLen);
		return NULL;
	}

	memcpy(infoReduccion->ipNodo, bytes_serial + off, infoReduccion->ipLen);
	off += infoReduccion->ipLen;





	memcpy(&infoReduccion->puertoLen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((infoReduccion->puertoNodo = malloc(infoReduccion->puertoLen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", infoReduccion->puertoLen);
		return NULL;
	}

	memcpy(infoReduccion->puertoNodo, bytes_serial + off, infoReduccion->puertoLen);
	off += infoReduccion->puertoLen;




	memcpy(&infoReduccion->tempRedLen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((infoReduccion->tempRed = malloc(infoReduccion->tempRedLen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", infoReduccion->tempRedLen);
		return NULL;
	}

	memcpy(infoReduccion->tempRed, bytes_serial + off, infoReduccion->tempRedLen);
	off += infoReduccion->tempRedLen;


	memcpy(&infoReduccion->listaSize, bytes_serial + off, sizeof (int));
	off += sizeof (int);


	int i;
	t_list *listaTemporales = list_create();
	for(i=0;i<infoReduccion->listaSize;i++){

		TreduccionLista *aux = malloc(sizeof aux);
		memcpy(&aux->nombreTemporalLen, bytes_serial + off, sizeof (int));
		off += sizeof (int);

		if ((aux->nombreTemporal = malloc(aux->nombreTemporalLen)) == NULL){
			printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", aux->nombreTemporalLen);
			return NULL;
		}

		memcpy(aux->nombreTemporal, bytes_serial + off, aux->nombreTemporalLen);
		off += aux->nombreTemporalLen;
		list_add(listaTemporales,aux);
	}

	infoReduccion->listaTemporalesTransformacion=listaTemporales;


	return infoReduccion;
}


char * serializarListaNombresTemporales(Theader head,t_list * listaNombres,int *pack_size){
	char *bytes_serial;
	int i;
	int espacioPackSize = sizeof(int);
	int espacioListSize = sizeof(int);

	int sizeLista = list_size(listaNombres);
	int espacioEnteros = sizeof(int) * sizeLista;
	int espaciosVariables=0;
	for(i=0;i< sizeLista;i++){
		TreduccionLista * aux = list_get(listaNombres,i);
		espaciosVariables += aux->nombreTemporalLen;
	}
	int espacioAMallocar = HEAD_SIZE + espacioPackSize+espacioListSize+espacioEnteros+espaciosVariables;

	if ((bytes_serial = malloc(espacioAMallocar)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}

	*pack_size = 0;
	memcpy(bytes_serial + *pack_size, &head, HEAD_SIZE);
	*pack_size += HEAD_SIZE;

	// hacemos lugar para el payload_size
	*pack_size += sizeof(int);

	// hacemos lugar para el lista_size
	*pack_size += sizeof(int);

	for(i=0;i<sizeLista;i++){
		TreduccionLista * aux = list_get(listaNombres,i);
		memcpy(bytes_serial + *pack_size, &aux->nombreTemporalLen, sizeof(int));
		*pack_size += sizeof(int);
		memcpy(bytes_serial + *pack_size, aux->nombreTemporal, aux->nombreTemporalLen);
		*pack_size += aux->nombreTemporalLen;
	}

	memcpy(bytes_serial + HEAD_SIZE, pack_size, sizeof(int));
	memcpy(bytes_serial + HEAD_SIZE+espacioPackSize, &sizeLista, sizeof(int));

	return bytes_serial;
}

t_list * deserializarListaNombresTemporales(char * bytes_serial){
	t_list * listaRet=list_create();

	int off;
	off = 0;
	int listaSize;

	memcpy(&listaSize, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	int i;
	for(i=0;i<listaSize;i++){

		TreduccionLista *aux = malloc(sizeof aux);
		memcpy(&aux->nombreTemporalLen, bytes_serial + off, sizeof (int));
		off += sizeof (int);


		if ((aux->nombreTemporal = malloc(aux->nombreTemporalLen)) == NULL){
			printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", aux->nombreTemporalLen);
			return NULL;
		}

		memcpy(aux->nombreTemporal, bytes_serial + off, aux->nombreTemporalLen);
		off += aux->nombreTemporalLen;
		list_add(listaRet,aux);
	}



	return listaRet;
}




char *serializarInfoReduccionLocalMasterWorker(Theader head,int nombreTemporalReduccionLen,char * nombreTemporalReduccion,t_list * listaTemporales, int *pack_size){

	char *bytes_serial;

	int i;
	int espacioPackSize = sizeof(int);
	int espacioListSize = sizeof(int);
	int espacioNombreTemporalLen = sizeof(int);
	int sizeLista = list_size(listaTemporales);

	int espaciosVariables=0;
	for(i=0;i< sizeLista;i++){
		TreduccionLista * aux = list_get(listaTemporales,i);
		espaciosVariables += aux->nombreTemporalLen;
	}
	espaciosVariables += sizeof(int)*sizeLista;
	int espacioAMallocar = HEAD_SIZE + espacioPackSize+espacioListSize+espacioNombreTemporalLen+nombreTemporalReduccionLen+espaciosVariables;
	printf("Espacio a mallocar: %d\n",espacioAMallocar);

	if ((bytes_serial = malloc(espacioAMallocar)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete de bytes\n");
		return NULL;
	}


	*pack_size = 0;
	memcpy(bytes_serial + *pack_size, &head, HEAD_SIZE);
	*pack_size += HEAD_SIZE;

	// hacemos lugar para el payload_size
	*pack_size += sizeof(int);

	memcpy(bytes_serial + *pack_size, &nombreTemporalReduccionLen, sizeof (int));
	*pack_size += sizeof (int);
	memcpy(bytes_serial + *pack_size, nombreTemporalReduccion, nombreTemporalReduccionLen);
	*pack_size += nombreTemporalReduccionLen;


	memcpy(bytes_serial + *pack_size, &sizeLista, sizeof (int));
	*pack_size += sizeof (int);

	printf("EN COMPARTIDAS; SIZE LISTA: %d\n",sizeLista);
	for(i=0;i<sizeLista;i++){
		TreduccionLista * aux = list_get(listaTemporales,i);
		memcpy(bytes_serial + *pack_size, &aux->nombreTemporalLen, sizeof(int));
		*pack_size += sizeof(int);
		memcpy(bytes_serial + *pack_size, aux->nombreTemporal, aux->nombreTemporalLen);
		*pack_size += aux->nombreTemporalLen;
	}


	memcpy(bytes_serial + HEAD_SIZE, pack_size, sizeof(int));

	printf("Pack size: %d\n",*pack_size);
	return bytes_serial;
}


TinfoReduccionLocalMasterWorker *deserializarInfoReduccionLocalMasterWorker(char *bytes_serial){

	int off;
	TinfoReduccionLocalMasterWorker *datosReduccion;

	if ((datosReduccion = malloc(sizeof *datosReduccion)) == NULL){
		fprintf(stderr, "No se pudo mallocar espacio para paquete datos reduccion\n");
		return NULL;
	}

	off = 0;
	memcpy(&datosReduccion->nombreTempReduccionLen, bytes_serial + off, sizeof (int));
	off += sizeof (int);

	if ((datosReduccion->nombreTempReduccion = malloc(datosReduccion->nombreTempReduccionLen)) == NULL){
		printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", datosReduccion->nombreTempReduccionLen);
		return NULL;
	}

	memcpy(datosReduccion->nombreTempReduccion, bytes_serial + off, datosReduccion->nombreTempReduccionLen);
	off += datosReduccion->nombreTempReduccionLen;



	memcpy(&datosReduccion->listaSize, bytes_serial + off, sizeof (int));
	off += sizeof (int);


	int i;
	t_list * listaRet = list_create();
	for(i=0;i<datosReduccion->listaSize;i++){

		TreduccionLista *aux = malloc(sizeof aux);
		memcpy(&aux->nombreTemporalLen, bytes_serial + off, sizeof (int));
		off += sizeof (int);


		if ((aux->nombreTemporal = malloc(aux->nombreTemporalLen)) == NULL){
			printf("No se pudieron mallocar %d bytes al Paquete De Bytes\n", aux->nombreTemporalLen);
			return NULL;
		}

		memcpy(aux->nombreTemporal, bytes_serial + off, aux->nombreTemporalLen);
		off += aux->nombreTemporalLen;
		list_add(listaRet,aux);
	}

	datosReduccion->listaTemporales=listaRet;



	return datosReduccion;
}

Tbuffer * empaquetarBytesMasInt(Theader* head, char * bytes, int numero) {
	Tbuffer * buffer = malloc(sizeof(Tbuffer));
	int tamanio = strlen(bytes) + 1;
	buffer->tamanio = (HEAD_SIZE + sizeof(int) + sizeof(int) + tamanio);
	buffer->buffer = malloc(buffer->tamanio);

	char * p = buffer->buffer;
	memcpy(p, head, sizeof(*head));
	p += sizeof(*head);
	memcpy(p, &numero, sizeof(int));
	p += sizeof(int);
	memcpy(p, &buffer->tamanio, sizeof(int));
	p += sizeof(int);
	memcpy(p, bytes, buffer->tamanio);
	p += buffer->tamanio;

	return buffer;
}

Tbuffer *empaquetarBytes(Theader * head, char * bytes){

	Tbuffer * buffer = malloc(sizeof(Tbuffer));
	int tamanio = strlen(bytes) +1;
	buffer->tamanio = (HEAD_SIZE + sizeof(int) + tamanio);
	buffer->buffer = malloc(buffer->tamanio);

	char * p = buffer->buffer;
	memcpy(p, head, sizeof(*head));
	p += sizeof(*head);
	memcpy(p, &buffer->tamanio, sizeof(int));
	p += sizeof(int);
	memcpy(p, bytes, buffer->tamanio);
	p += buffer->tamanio;

	return buffer;
}

Tbuffer * empaquetarInt(Theader * head, int numero){

	Tbuffer * buffer = malloc(sizeof(Tbuffer));

	buffer->tamanio = (HEAD_SIZE + sizeof(int));
	buffer->buffer = malloc(buffer->tamanio);

	char * p = buffer->buffer;
	memcpy(p, head, sizeof(*head));
	p += sizeof(*head);
	memcpy(p, &numero, sizeof(int));
	p += sizeof(int);

	return buffer;
}
