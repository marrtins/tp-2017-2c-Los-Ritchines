/*
 * directorios.c
 *
 *  Created on: 21/9/2017
 *      Author: utnso
 */

#include "directorios.h"

void eliminar_directorio(t_directory* directorio){
	free(directorio);
	directorio = NULL;
}

t_directory* buscar_directorio_nombre(t_list* listaDirectorios, char* nombre, int padre){
	bool buscar_directorio_nombre(t_directory* directorio){
		return string_equals_ignore_case(directorio->nombre, nombre) && directorio->padre == padre;
	}
	return list_find(listaDirectorios, (void*)buscar_directorio_nombre);
}

void setear_nombre_directorio(t_directory* directorio, char* nombre) {
	strcpy(directorio->nombre, nombre);
}

t_directory* buscar_directorio_id(t_list* listaDirectorios, int id){
	bool buscar_directorio_id(t_directory* dir){
		return dir->index == id;
	}
	return list_find(listaDirectorios, (void*)buscar_directorio_id);
}

int existe_directorio_de_nombre(char* name, t_list* listaDirectorios){
	int _eq_name(t_directory* direc){
		return string_equals_ignore_case(direc->nombre,name);
	}
	return list_any_satisfy(listaDirectorios, (void*) _eq_name);
}

