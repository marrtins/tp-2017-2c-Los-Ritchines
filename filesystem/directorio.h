/*
 * directorios.h
 *
 *  Created on: 23/9/2017
 *      Author: utnso
 */

#include <stdbool.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include "filesystem.h"

#define DIR_TAMANIO_MAX_NOMBRE 128
#define DIR_CANT_MAX 99

//char FILE_DIRECTORIO[99] = "/home/utnso/tp-201-2c-Los-Ritchines/filesystem/directorios.dat";
char FILE_DIRECTORIO[99];


void eliminar_directorio(t_directory* directorio);
t_directory* buscar_directorio_nombre(t_list* listaDirectorios, char* nombre, int padre);

void setear_nombre_directorio(t_directory* directorio, char* nombre);

t_directory* buscar_directorio_id(t_list* listaDirectorios, int id);

