#ifndef LIB_ESTRUCTURASYM_H_
#define LIB_ESTRUCTURASYM_H_

#include "definicionesYM.h"

typedef struct{

	char* ip_filesystem;
	char* puerto_entrada;
	char* puerto_master;
	char* puerto_filesystem;
	int   algoritmo_balanceo,
	   	  retardo_planificacion,
		  disponibilidadBase,
		  tipo_de_proceso;
}Tyama;


/*typedef struct {

	Theader head;
	int bytelen;
	char *bytes;
} TpackSrcCode,TpackBytes;
*/
typedef struct {

	Theader head;
	int bytelen1;
	char *bytes1;
	int bytelen2;
	char *bytes2;
}__attribute__((packed)) Tpack2Bytes;

typedef struct {
	char * nombreNodo;
	int tareasRealizadas;
}ThistorialTareas;
typedef struct {
	char * nombreNodo;
	int cargaGlobal;
}TcargaGlobal;


typedef struct {
	TpackageInfoNodo infoNodo;
	int availability;
	int disponibilidadBase;
	int pwl;
	bool clock;

}Tplanificacion;

typedef struct {
	int idTarea;
	int job;
	int master;
	char * nodo;
	int bloqueDelArchivo;
	int etapa;
	char * nombreArchTemporal;
	//t_list * listaTemporales;
	bool fueReplanificada;
	char  * bloquesReducidos;
}TpackTablaEstados;

typedef struct{
	int fdMaster;
}TatributosHiloMaster;

typedef struct {
	int nroJob;
	bool finCorrecto;
}TjobFinalizado;
typedef struct{
	int nroJob;
	int fdMaster;
	int masterId;
	char * pathResultado;
	t_list * listaComposicionArchivo;
	t_list * listaNodosArchivo;
}TjobMaster;


t_log* logger;



#endif
