#ifndef LIB_FUNCIONESWK_H_
#define LIB_FUNCIONESWK_H_

#include "estructurasWK.h"

Tworker *obtenerConfiguracionWorker(char* ruta);
void mostrarConfiguracion(Tworker *worker);

void manejarConexionMaster(Theader *header,int client_sock);

#endif
