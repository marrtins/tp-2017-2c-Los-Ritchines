/*
 * directorios.c
 *
 *  Created on: 21/9/2017
 *      Author: utnso
 */

#include "directorios.h"

void eliminar_directorio(t_directorio* directorio){
	free(directorio);
}

t_directorio* buscar_directorio_nombre(t_list* listaDirectorios, char* nombre, int padre){
	bool buscar_directorio_nombre(t_directorio* directorio){
		return string_equals_ignore_case(directorio->nombre, nombre) && directorio->padre == padre;
	}
	return list_find(listaDirectorios, (void*)buscar_directorio_nombre);
}

void setear_nombre_directorio(t_directorio* directorio, char* nombre) {
	strcpy(directorio->nombre, nombre);
}

t_directorio* buscar_directorio_id(t_list* listaDirectorios, int id){
	bool buscar_directorio_id(t_directorio* dir){
		return dir->index == id;
	}
	return list_find(listaDirectorios, (void*)buscar_directorio_id);
}


