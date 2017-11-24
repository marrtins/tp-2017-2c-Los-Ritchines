#ifndef LIB_FUNCIONESYM_H_
#define LIB_FUNCIONESYM_H_

#include "estructurasYM.h"

Tyama *obtenerConfiguracionYama(char* ruta);
void mostrarConfiguracion(Tyama *yama);
char * getAlgoritmoBalanceo(int algoritmo);

void sigusr1Handler(void);
void recargarConfiguracion(void);
void setRetardoPlanificacion();
int conectarAFS(Tyama *yama);
void masterHandler(void *client_sock);
int divideYRedondea(int x,int y);
int responderSolicTransf(int sockMaster,t_list * listaBloques,TjobMaster *job);
int conectarNuevoCliente( int fileDescriptor, fd_set * bolsaDeFileDescriptors);
void clearAndClose(int fileDescriptor, fd_set* masterFD);

void generarListaBloquesHardcode(t_list *listaBloques);
void generarListaComposicionArchivoHardcode(t_list * listaInfo);
void generarListaInfoNodos();


TpackInfoBloqueDN * recvInfoNodoYAMA(int socketFS);

//Funciones Planificacion
int posicionarClock(t_list * listaWorkers);
TpackInfoBloque * asignarBloque(TpackageUbicacionBloques *bloqueAux,t_list *listaWorkersPlanificacion,TjobMaster *job);
Tplanificacion *  getNodoApuntado(t_list * listaWorkersPlanificacion);
void mergeBloque(TpackInfoBloque *bloqueRet,Tplanificacion *nodoApuntado,TpackageUbicacionBloques *bloqueAux,int idMaster);
void avanzarClock(t_list *listaWorkersPlanificacion);
int desempatarClock(int disponibilidadMasAlta,t_list * listaWorkers);
int getHistorico(Tplanificacion *infoWorker);
Tplanificacion * getSiguienteNodoDisponible(t_list * listaWorkersPlanificacion,char * nombreNodo1,char* nombreNodo2);
Tplanificacion * getSiguienteConDisponibilidadPositivaPosible(t_list * listaWorkersPlanificacion, int indice,char * nombre1,char * nombre2);
void actualizarCargaWorkerEn(char * nombreNodo, int cantidadAAumentar);
t_list * planificar(TjobMaster *job);
void aumentarHistoricoEn(char * nombreNodo,int cantidadAAumentar);
void sumarDisponibilidadBaseATodos(t_list * listaWorkersPlanificacion);
int getCargaWorker(char * nombreWorker);
int getMayorCargaAllWorkers();
void asignarNodoElegido(t_list * listaReduccionGlobal);

int moverAListaFinalizadosOK(int idTareaFinalizada);
int moverFinalizadaAListaError(int idTarea);
int moverAListaError(int idTareaFinalizada);
char  * getNombreEtapa(int etapaEnum);
void mostrarTablaDeEstados();

char * generarNombreTemporal(int idMaster);
char *  generarNombreReductorTemporal(char * nombreNodo,int idMaster);
char *  generarNombreReduccionGlobalTemporal(int idMaster);
TpackTablaEstados * getTareaPorId(int idTarea);
bool sePuedeReplanificar(int idTarea,t_list * listaComposicionArchivo);
int replanificar(int idTarea,int fdMaster,t_list * listaComposicionArchivo);

char * getIpNodo(char * nombreNodo,TjobMaster *job);
char * getPuertoNodo(char * nombreNodo,TjobMaster *job);
bool sePuedeComenzarReduccionLocal(int idTareaFinalizada);
int comenzarReduccionLocal(int idTareaFinalizada,int sockMaster);

void agregarReduccionLocalAListaEnProceso(TreduccionLocal * infoReduccion,char * bloquesReducidos,TjobMaster *job);
void agregarTransformacionAListaEnProceso(TjobMaster *job, TpackInfoBloque *bloque,bool mostrarTabla);
void agregarReduccionGlobalAListaEnProceso(TreduccionGlobal *infoReduccion,char * bloquesReducidos,TjobMaster *job);
void agregarAlmacenadoFinalAListaEnProceso(TinfoAlmacenadoFinal *infoAlmacenado,char * nombreNodo,char * bloquesReducidos,TjobMaster *job);

bool sePuedeComenzarReduccionGlobal(int idTareaFinalizada);
int comenzarReduccionGlobal(int idTareaFinalizada,int sockMaster);
bool esNodoEncargado(char * nombreNodo);
bool yaFueFinalizadoPorErrorDeReplanificacion(int job);

int comenzarAlmacenadoFinal(int idTareaFinalizada,int sockMaster);

char * getNodoElegido(t_list * listaNodos);
void liberarCargaNodos(int idTareaFinalizada);
bool yaFueAgregadoAlistaJobFinalizados(int idTareaFinalizada);
void liberarCargaEn(char * nombreNodo,int cantidad);
int getCargaReduccionGlobal(int job);


void mostrarTablaHistorica();
void mostrarTablaCargas();

char * recibirPathArchivo(int sockMaster);
int responderTransformacion(TjobMaster *job);
void iniciarNuevoJob(int sockMaster,int socketFS);
void manejarFinTransformacionOK(int sockMaster);
void manejarFinTransformacionFailDesconexion(int sockMaster);

void liberarBloquesPlanificados(void * info);
void liberarWorkersPlanificacion(void * info);


void manejarFinTransformacionFail(int sockMaster);
void manejarFinReduccionLocalOK(int sockMaster);
void manejarFinReduccionLocalFail(int sockMaster);
void manejarFinReduccionGlobalOK(int sockMaster);
void manejarFinReduccionGlobalFail(int sockMaster);
void manejarFinAlmacenamientoOK(int sockMaster);
void manejarFinAlmacenamientoFail(int sockMaster);

TjobMaster * getJobPorNroJob(int nroJob);
TjobMaster * getJobPorMasterID(int masterId);

#endif

