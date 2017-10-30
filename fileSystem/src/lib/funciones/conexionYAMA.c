#include "../funcionesFS.h"

char * recvRutaArchivo(int socket){
	int tamanio;
	char * ruta;
	char * rutaArchivo;

	if (recv(socket, &tamanio, sizeof(int), 0) == -1) {
		logAndExit("Error al recibir el tamanio de la ruta");
	}
	ruta = malloc(tamanio);
	rutaArchivo = malloc(tamanio);

	if (recv(socket, rutaArchivo, tamanio, 0) == -1) {
		logAndExit("Error al recibir la ruta");
	}
	memcpy(rutaArchivo, ruta, tamanio);
	free(ruta);
	return rutaArchivo;
}

//despues se puede pasar a compartidas aunque en realidad solo la usa FS
Tbuffer * empaquetarInfoArchivo(Theader* head, Tarchivo * archivo){

	Tbuffer * buffer = malloc(sizeof(Tbuffer));
	int tamanioFijo, tamanioNombreNodo, cantBloques, nroBloque = 0;
	cantBloques = cantidadDeBloquesDeUnArchivo(archivo->tamanioTotal);

	//La estructura que envio es head + cantBloquesArchivo + [ nroBloqueArchivo + tamanioNombreNodo +
	//copiacero->NombreNodo + copiacero->bloqueDatabin + tamanioNombreNodo + copiauno->NombreNodo +
	//copiauno->bloqueDatabin ]

	tamanioFijo = sizeof(int) * 5 * cantBloques;
	buffer->tamanio = (HEAD_SIZE + sizeof(int) + tamanioFijo);
	buffer->buffer = malloc(buffer->tamanio);

	char * p = buffer->buffer;
	memcpy(p, head, sizeof(*head));
	p += sizeof(*head);
	memcpy(p, &cantBloques, sizeof(int));
	p += sizeof(int);

	while(nroBloque != cantBloques){
		memcpy(p, &nroBloque, sizeof(int));
		p += sizeof(int);

		//copia cero
		tamanioNombreNodo = strlen(archivo->bloques[nroBloque].copiaCero.nombreDeNodo) +1;
		memcpy(p,&tamanioNombreNodo,sizeof(int));
		p += sizeof(int);

		buffer->tamanio += tamanioNombreNodo;
		buffer->buffer = realloc(buffer->buffer, tamanioNombreNodo);

		memcpy(p,archivo->bloques[nroBloque].copiaCero.nombreDeNodo,tamanioNombreNodo);
		p += tamanioNombreNodo;
		memcpy(p,&archivo->bloques[nroBloque].copiaCero.numeroBloqueDeNodo,sizeof(int));
		p += sizeof(int);

		//copia uno
		tamanioNombreNodo = strlen(archivo->bloques[nroBloque].copiaUno.nombreDeNodo) +1;
		memcpy(p,&tamanioNombreNodo,sizeof(int));
		p += sizeof(int);

		buffer->tamanio += tamanioNombreNodo;
		buffer->buffer = realloc(buffer->buffer, tamanioNombreNodo);

		memcpy(p,archivo->bloques[nroBloque].copiaUno.nombreDeNodo,tamanioNombreNodo);
		p += tamanioNombreNodo;
		memcpy(p,&archivo->bloques[nroBloque].copiaCero.numeroBloqueDeNodo,sizeof(int));
		p += sizeof(int);

		nroBloque++;
	}

	return buffer;
}

