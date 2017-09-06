/*
 * definiciones.h
 *
 *  Created on: 1/9/2017
 *      Author: utnso
 */

#ifndef DEFINICIONES_H_
#define DEFINICIONES_H_

#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



#define HEAD_SIZE 8
#define BACKLOG 20


typedef enum{
	FALLO_GRAL=-21,
	FALLO_CONEXION=-22,
	FALLO_RECV=-23,
	FALLO_SEND=-24,
	FALLO_SELECT=-24,
	CONEX_INVAL=-25
}tErrores;

typedef enum {
	INICIO      = 1,
	NEW_DN=2,
	INICIOFS=3,
	INICIOYAMA=4
} tMensaje;


typedef enum {
	YAMA= 1,
	FILESYSTEM= 2,
	DATANODE= 3,
	WORKER= 4,
	MASTER= 5
} tProceso;


typedef struct {

	tProceso tipo_de_proceso;
	tMensaje tipo_de_mensaje;
} tHeader; // este tipo de struct no necesita serialazion

#endif /* DEFINICIONES_H_ */
