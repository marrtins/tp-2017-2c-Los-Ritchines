#ifndef LIB_FUNCIONESWK_H_
#define LIB_FUNCIONESWK_H_

#include "estructurasWK.h"

Tworker *obtenerConfiguracionWorker(char* ruta);
void liberarConfiguracionWorker(Tworker *worker);
void mostrarConfiguracion(Tworker *worker);
char * getBloqueWorker(int nroBloque);
int manejarConexionMaster(Theader *header,int client_sock);
int manejarConexionWorker(Theader *head, int client_sock);

void clearAndClose(int fileDescriptor, fd_set* masterFD);
int conectarNuevoCliente( int fileDescriptor, fd_set * bolsaDeFileDescriptors);
void desempaquetarScript(int fileDescriptor, Tscript * script);
int recibirYAlmacenarScript(int client_sock,char * rutaAAlmacenar);
int realizarReduccionLocal(int client_sock);
int realizarTransformacion(int client_sock);
int realizarReduccionGlobal(int client_sock);
int realizarApareoGlobal(t_list * listaInfoNodos,char * rutaApareoGlobal);
int recibirYAlmacenarArchivo(int client_sock,char * rutaAAlmacenar);
int enviarArchivo(char * rutaArchivo,int sockDestino);
int realizarAlmacenamientoFinal(int client_sock);
int compararLineas(int cantArchivos,char arrayLineas[cantArchivos][MAXSIZELINEA]);
Tbuffer * empaquetarArchivoFinal(Theader * header, char * rutaArchivo, char * contenidoArchivo, unsigned long long tamanioArchivoFinal);
void liberarInfoApareoGlobal(void * info);
void liberarInfoNodos(void * info);
void liberarListaTemporales(void * info);

int recibirYAlmacenarScript2(int client_sock,char *rutaAAlmacenar);

FILE * getFDTemporal(int fdWorker);
void removerFDWorker(int fdWorker);
#endif
