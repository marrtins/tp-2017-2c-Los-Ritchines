#include "../funcionesFS.h"

char * generarArrayParaArchivoConfig(char * dato1, char * dato2){
	char * concatenacionLoca = malloc(1 + TAMANIO_NOMBRE_NODO + 1 + 3);
	string_append(&concatenacionLoca, "[");
	string_append(&concatenacionLoca, dato1);
	string_append(&concatenacionLoca, ",");
	string_append(&concatenacionLoca, dato2);
	string_append(&concatenacionLoca, "]");
	return concatenacionLoca;
}
