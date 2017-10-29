#include "../funcionesFS.h"

void generarArrayParaArchivoConfig(t_config * archivoConf, char * key, char * dato1, char * dato2){
	char * concatenacionLoca = string_new();
	string_append(&concatenacionLoca, "[");
	string_append(&concatenacionLoca, dato1);
	string_append(&concatenacionLoca, ",");
	string_append(&concatenacionLoca, dato2);
	string_append(&concatenacionLoca, "]");
	printf("Soy lo que rompe: %s", concatenacionLoca);
	config_set_value(archivoConf, key, concatenacionLoca);
	free(concatenacionLoca);
	while(1);
}

void eliminarElementoDeArrayArchivosConfig(t_config * archivoConfig, char * key, char * nombreElemento){
	char ** elementos = config_get_array_value(archivoConfig, key);
	int i = 0;
	char * definitivo = string_new();
	char * nuevoString = string_new();
	string_append(&nuevoString, "[");
	int verificacion = 0;
	while(elementos[i] != NULL){
		if(strcmp(elementos[i], nombreElemento)){
			string_append(&nuevoString, elementos[i]);
			string_append(&nuevoString, ",");
			verificacion++;
		}
		i++;
	}

	liberarPunteroDePunterosAChar(elementos);
	free(elementos);

	if(verificacion > 0){
		definitivo = string_substring(nuevoString, 0, strlen(nuevoString)-1);
		string_append(&definitivo, "]");
		config_set_value(archivoConfig, key, definitivo);
		free(definitivo);
		free(nuevoString);
		return;
	}
	string_append(&nuevoString, "]");
	config_set_value(archivoConfig, key, nuevoString);
	free(definitivo);
	free(nuevoString);
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

