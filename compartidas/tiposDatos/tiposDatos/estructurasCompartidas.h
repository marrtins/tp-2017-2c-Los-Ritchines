#ifndef COMPARTIDAS_ESTRUCTURASCOMPARTIDAS_H_
#define COMPARTIDAS_ESTRUCTURASCOMPARTIDAS_H_

#include "definicionesCompartidas.h"

typedef enum{
	TRANSFORMACION = 80,
	REDUCCIONLOCAL=81,
	REDUCCIONGLOBAL=82,
	ALMACENAMIENTOFINAL=83
}Etapa;
typedef enum{
	CLOCK=121,
	WCLOCK=122
}AlgoritmoBalanceo;

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
	FIN_LOCALTRANSF=13,
	PREG_FILEINFO=14,
	RTA_FILEINFO=15,
	INFOBLOQUE=16,
	INFOULTIMOBLOQUE=17,
	INFOCONEXIONWORKER=18,
	TRANSFORMADORLEN=19,
	ALMACENAR_BLOQUE=20,
	NUEVATRANSFORMACION=21,
	FINTRANSFORMACIONLOCALOK=22,
	FINTRANSFORMACIONLOCALFAIL=23,
	INFOBLOQUEREPLANIFICADO=24,
	FINJOB_ERRORREPLANIFICACION=25,
	INFOREDUCCIONLOCAL=26,
	INICIARREDUCCIONLOCAL=27,
	FIN_REDUCCIONLOCAL=28,
	FIN_REDUCCIONLOCALOK=29,
	FIN_REDUCCIONLOCALFAIL=30,
	INICIARREDUCCIONGLOBAL=31,
	FIN_REDUCCIONGLOBALOK=32,
	FIN_REDUCCIONGLOBALFAIL=33,
	OBTENER_BLOQUE_Y_NRO=34,
	INICIO_YAMA=35,
	INFO_ARCHIVO= 36,
	INFO_BLOQUE = 37,
	FORMATEAR_NODO = 38,
	OBTENER_BLOQUE = 39,
	INFOREDUCCIONGLOBAL=40,
	GIVE_TMPREDUCCIONLOCAL=41,
	TAKE_TMPREDUCCIONLOCAL=42,
	GIVE_NEXTLINE=43,
	TAKE_NEXTLINE=44,
	EOF_TEMPORAL=45,
	INFOALMACENADOFINAL=46,
	INICIARALMACENAMIENTOFINAL=47,
	FIN_ALMACENAMIENTOFINALOK=48,
	FIN_ALMACENAMIENTOFINALFAIL=49,
	ELIMINAR_BLOQUE=50,
	ARCH_NO_VALIDO=51

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

typedef struct {
	Theader head;
	int bytelen;
	char *bytes;
} TpackSrcCode,TpackBytes;

typedef struct {
	Theader head;
	uint32_t tamanioRutaOrigen;
	char * rutaOrigen;
	uint32_t tamanioRutaResultado;
	char * rutaResultado;
}TpackageRutas;

typedef struct {
	Theader head;
	uint32_t tamanioReductor;
	char * scriptReductor;
	uint32_t tamanioTransformador;
	char * scriptTransformador;
}TpackageScripts;


typedef struct {
	int bloque;
	int nombreNodoC1Len;
	char *nombreNodoC1;
	int bloqueC1;
	int nombreNodoC2Len;
	char *nombreNodoC2;
	int bloqueC2;
	unsigned long long finBloque;
}TpackageUbicacionBloques;

typedef struct{
	int listaSize;
	t_list * listaBloques;
}TinfoArchivoFSYama;

typedef struct {
	int tamanioNombre;
	char *nombreNodo;
	int tamanioIp;
	char *ipNodo;
	int tamanioPuerto;
	char *puertoWorker;
}TpackageInfoNodo;


typedef struct {

	Theader head;
	int idTarea;
	int tamanioNombre;
	char *nombreNodo;
	int bloqueDelArchivo;
	int bloqueDelDatabin;
	int bytesOcupados;
	int nombreTemporalLen;
	char *nombreTemporal;
	int tamanioIp;
	char * ipWorker;
	int tamanioPuerto;
	char * puertoWorker;
}__attribute__((packed)) TpackInfoBloque;

typedef struct {
	Theader head;
	int tamanioNombre;
	char *nombreNodo;
	int tamanioIp;
	char * ipNodo;
	int tamanioPuerto;
	char * puertoNodo;
	int databinEnMB;

}TpackInfoBloqueDN;

typedef struct {
	char * buffer;
	unsigned long long tamanio;
}Tbuffer;

typedef struct{
	int numeroDeBloque;
	char *contenido;
	unsigned long long tamanio;
}TbloqueAEnviar;

typedef struct{
	char* nombre;
	int fd;
	unsigned int cantidadBloquesTotal;
	unsigned int cantidadBloquesLibres;
	t_bitarray * bitmap;
}Tnodo;

typedef struct {
	Theader head;
	int nroBloque;
	int bytesOcupadosBloque;
	int nombreTemporalLen;
	char * nombreTemporal;
}TpackDatosTransformacion;


typedef struct{
	int job;
	int idTarea;
	int nombreNodoLen;
	char * nombreNodo;
	int ipLen;
	char * ipNodo;
	int puertoLen;
	char * puertoNodo;
	int tempRedLen;
	char * tempRed;
	int listaSize;
	t_list * listaTemporalesTransformacion;
}TreduccionLocal;





typedef struct {
	int nombreTemporalLen;
	char * nombreTemporal;
}TreduccionLista;


typedef struct{
	int job;
	int idTarea;
	int tempRedGlobalLen;
	char * tempRedGlobal;
	int listaNodosSize;
	t_list * listaNodos;
}TreduccionGlobal;



typedef struct{
	int nombreNodoLen;
	char * nombreNodo;
	int ipNodoLen;
	char * ipNodo;
	int puertoNodoLen;
	char * puertoNodo;
	int temporalReduccionLen;
	char * temporalReduccion;
	int nodoEncargado;
}TinfoNodoReduccionGlobal;


typedef struct {
	int nombreTempReduccionLen;
	char * nombreTempReduccion;
	int listaSize;
	t_list * listaTemporales;
}TinfoReduccionLocalMasterWorker;

typedef struct{
	int nombreTempReduccionLen;
	char * nombreTempReduccion;
	int listaNodosSize;
	t_list * listaNodos;
}TinfoReduccionGlobalMasterWorker;
typedef struct{
	int idTarea;
	int job;
	int ipNodoLen;
	char * ipNodo;
	int puertoNodoLen;
	char * puertoNodo;
	int nombreTempReduccionLen;
	char * nombreTempReduccion;
}TinfoAlmacenadoFinal;
typedef struct{
	int nombreTempReduccionLen;
	char * nombreTempReduccion;
	int nombreResultanteLen;
	char * nombreResultante;
}TinfoAlmacenadoMasterWorker;

typedef struct{

};

t_log* logger;

#endif
