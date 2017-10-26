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

int sumaDeDosNumerosInt(int valor1, int valor2){
	return valor1 + valor2;
}

long sumaDeDosNumerosLong(long valor1, long valor2){
	return valor1 + valor2;
}

int restaDeDosNumerosInt(int valor1, int valor2){
	return valor1 - valor2;
}

long restaDeDosNumerosLong(long valor1, long valor2){
	return valor1 - valor2;
}

void setearAtributoDeArchivoConfigConInts(t_config * archivoConf, char * key, int value, int(*funcion)(int valor1, int valor2)){
	int tamanio = config_get_int_value(archivoConf, key);
	tamanio = funcion(tamanio, value);
	char * tamanioString = string_itoa(tamanio);
	config_set_value(archivoConf, key, tamanioString);
	free(tamanioString);
}

//problemas con los long, tener cuidado
void setearAtributoDeArchivoConfigConLongs(t_config * archivoConf, char * key, long value, long(*funcion)(long valor1, long valor2)){
	long tamanio = config_get_long_value(archivoConf, key);
	tamanio = funcion(tamanio, value);
	char * tamanioString = string_itoa(tamanio);
	config_set_value(archivoConf, key, tamanioString);
	free(tamanioString);
}

