#ifndef COMPARTIDAS_ESTRUCTURASCOMPARTIDAS_H_
#define COMPARTIDAS_ESTRUCTURASCOMPARTIDAS_H_

#include "definicionesCompartidas.h"

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
	INFO_NODO=10,
	INICIOMASTER=11,
	START_LOCALTRANSF=12,
	FIN_LOCALTRANSF=13
} Tmensaje;

typedef enum {
	YAMA= 1,
	FILESYSTEM= 2,
	DATANODE= 3,
	WORKER= 4,
	MASTER= 5
} Tproceso;

typedef struct {
	Tproceso tipo_de_proceso;
	Tmensaje tipo_de_mensaje;
} Theader;

typedef enum{
	FALLO_GRAL=-21,
	FALLO_CONEXION=-22,
	FALLO_RECV=-23,
	FALLO_SEND=-24,
	FALLO_SELECT=-24,
	FALLO_BIND=-25,
	CONEX_INVAL=-26
}Terrores;

#endif
