#ifndef LIB_FUNCIONESYM_H_
#define LIB_FUNCIONESYM_H_

#include "estructurasYM.h"

Tyama *obtenerConfiguracionYama(char* ruta);
void mostrarConfiguracion(Tyama *yama);
void conectarAFS(int* socketFS, Tyama *yama);
char *recvGenericWFlags(int sock_in, int flags);
char *recvGeneric(int sock_in);
TpackBytes *deserializeBytes(char *bytes_serial);
void freeAndNULL(void **ptr);
void masterHandler(void *client_sock);
int responderSolicTransf(int sockMaster,t_list * listaBloques);
char *  generarNombreTemporal(int idMaster);

void generarListaBloquesHardcode(t_list *listaBloques);
void generarListaComposicionArchivoHardcode(t_list * listaInfo);
void generarListaInfoNodos(t_list * listaNodos);
void setearGlobales();


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
t_list * planificar(t_list * listaInfoBloques,t_list * listaInfoNodos);
void aumentarHistoricoEn(char * nombreNodo,int cantidadAAumentar);
void sumarDisponibilidadBaseATodos(t_list * listaWorkersPlanificacion);
#endif

