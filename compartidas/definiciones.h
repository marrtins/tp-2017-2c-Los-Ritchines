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
	INICIOYAMA=4,
	SRC_CODE_RED=5,
	SRC_CODE_TRANSF=6,
	SRC_CODE=7,
	PATH_FILE_TOREDUCE=8,
	PATH_RES_FILE=9,
	INFO_WORKER=10
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

typedef struct {

	tHeader head;
	int bytelen;
	char *bytes;
} tPackSrcCode,tPackBytes;

typedef struct {

	tHeader head;
	int bytelen1;
	char *bytes1;
	int bytelen2;
	char *bytes2;
}__attribute__((packed)) tPack2Bytes;



#endif /* DEFINICIONES_H_ */
