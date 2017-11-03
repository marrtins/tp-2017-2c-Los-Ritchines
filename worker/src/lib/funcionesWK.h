#ifndef LIB_FUNCIONESWK_H_
#define LIB_FUNCIONESWK_H_

#include "estructurasWK.h"

Tworker *obtenerConfiguracionWorker(char* ruta);
void mostrarConfiguracion(Tworker *worker);

int manejarConexionMaster(Theader *header,int client_sock);
int manejarConexionWorker(Theader *head, int client_sock);

int recibirYAlmacenarScript(int client_sock,char * rutaAAlmacenar);
int realizarReduccionLocal(int client_sock);
int realizarTransformacion(int client_sock);
int realizarReduccionGlobal(int client_sock);
int realizarApareoGlobal(t_list * listaInfoNodos,char * rutaApareoGlobal);
int recibirYAlmacenarArchivo(int client_sock,char * rutaAAlmacenar);
int enviarArchivo(char * rutaArchivo,int sockDestino);
int realizarAlmacenamientoFinal(int client_sock);
int compararLineas(int cantArchivos,char arrayLineas[cantArchivos][MAXSIZELINEA]);
#endif
