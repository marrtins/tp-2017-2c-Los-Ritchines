#ifndef LIB_FUNCIONESWK_H_
#define LIB_FUNCIONESWK_H_

#include "estructurasWK.h"

Tworker *obtenerConfiguracionWorker(char* ruta);
void mostrarConfiguracion(Tworker *worker);

void manejarConexionMaster(Theader *header,int client_sock);

int recibirYAlmacenarScript(int client_sock,char * rutaAAlmacenar);
int realizarReduccionLocal(int client_sock);
int realizarTransformacion(int client_sock);
int realizarReduccionGlobal(int client_sock);
#endif
