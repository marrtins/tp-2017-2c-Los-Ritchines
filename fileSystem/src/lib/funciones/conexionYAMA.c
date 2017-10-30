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

	//La estructura que envio es head + cantBloquesArchivo + [ nroBloqueArchivo + copiacero->NombreNodo +
	//copiacero->bloqueDatabin + copiauno->NombreNodo + copiauno->bloqueDatabin ]



	return buffer;
}

