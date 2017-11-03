#ifndef LIB_FUNCIONESYM_H_
#define LIB_FUNCIONESYM_H_

#include "estructurasYM.h"

Tyama *obtenerConfiguracionYama(char* ruta);
void mostrarConfiguracion(Tyama *yama);
void conectarAFS(Tyama *yama);
void masterHandler(void *client_sock);
int responderSolicTransf(int sockMaster,t_list * listaBloques);


void generarListaBloquesHardcode(t_list *listaBloques);
void generarListaComposicionArchivoHardcode(t_list * listaInfo);
void generarListaInfoNodos();




//Funciones Planificacion
int posicionarClock(t_list * listaWorkers);
TpackInfoBloque * asignarBloque(TpackageUbicacionBloques *bloqueAux,t_list *listaWorkersPlanificacion);
Tplanificacion *  getNodoApuntado(t_list * listaWorkersPlanificacion);
void mergeBloque(TpackInfoBloque *bloqueRet,Tplanificacion *nodoApuntado,TpackageUbicacionBloques *bloqueAux);
void avanzarClock(t_list *listaWorkersPlanificacion);
int desempatarClock(int disponibilidadMasAlta,t_list * listaWorkers);
int getHistorico(Tplanificacion *infoWorker);
Tplanificacion * getSiguienteNodoDisponible(t_list * listaWorkersPlanificacion,char * nombreNodo1,char* nombreNodo2);
Tplanificacion * getSiguienteConDisponibilidadPositivaPosible(t_list * listaWorkersPlanificacion, int indice,char * nombre1,char * nombre2);
void actualizarCargaWorkerEn(char * nombreNodo, int cantidadAAumentar);
t_list * planificar(t_list * listaInfoBloques);
void aumentarHistoricoEn(char * nombreNodo,int cantidadAAumentar);
void sumarDisponibilidadBaseATodos(t_list * listaWorkersPlanificacion);

int moverAListaFinalizadosOK(int idTareaFinalizada);
int moverAListaError(int idTareaFinalizada);
char * getNombreEtapa(int etapaEnum);
void mostrarTablaDeEstados();

char * generarNombreTemporal();
char *  generarNombreReductorTemporal(char * nombreNodo);
char *  generarNombreReduccionGlobalTemporal();
TpackTablaEstados * getTareaPorId(int idTarea);
bool sePuedeReplanificar(int idTarea,t_list * listaComposicionArchivo);
int replanificar(int idTarea,int fdMaster,t_list * listaComposicionArchivo);

char * getIpNodo(char * nombreNodo);
char * getPuertoNodo(char * nombreNodo);
bool sePuedeComenzarReduccionLocal(int idTareaFinalizada);
int comenzarReduccionLocal(int idTareaFinalizada,int sockMaster);

void agregarReduccionLocalAListaEnProceso(TreduccionLocal * infoReduccion,char * bloquesReducidos);
void agregarTransformacionAListaEnProceso(int jobActual, int idTarea, TpackInfoBloque *bloque);
void agregarReduccionGlobalAListaEnProceso(TreduccionGlobal *infoReduccion);
void agregarAlmacenadoFinalAListaEnProceso(TinfoAlmacenadoFinal *infoAlmacenado,char * nombreNodo);

bool sePuedeComenzarReduccionGlobal(int idTareaFinalizada);
int comenzarReduccionGlobal(int idTareaFinalizada,int sockMaster);
bool esNodoEncargado(char * nombreNodo);


int comenzarAlmacenadoFinal(int idTareaFinalizada,int sockMaster);

char * getNodoElegido(t_list * listaNodos);


#endif

