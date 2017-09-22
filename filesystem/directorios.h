/*
 * directorios.h
 *
 *  Created on: 21/9/2017
 *      Author: utnso
 */

#ifndef DIRECTORIOS_H_
#define DIRECTORIOS_H_

#include <stdbool.h>
#include <stdint.h>
#include <commons/collections/list.h>


#define DIR_TAMANIO_MAX_NOMBRE 128
#define DIR_CANT_MAX 99

//char FILE_DIRECTORIO[99] = "/home/utnso/tp-201-2c-Los-Ritchines/filesystem/directorios.dat";
char FILE_DIRECTORIO[99];


typedef struct {
	int index;
	char nombre[255];
	int padre;
} t_directorio;

void eliminar_directorio(t_directorio* directorio);
t_directorio* buscar_directorio_nombre(t_list* listaDirectorios, char* nombre, int padre);

void setear_nombre_directorio(t_directorio* directorio, char* nombre);

t_directorio* buscar_directorio_id(t_list* listaDirectorios, int id);
