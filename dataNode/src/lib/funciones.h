#ifndef LIB_FUNCIONES_H_
#define LIB_FUNCIONES_H_

#include "estructuras.h"

TdataNode *obtenerConfiguracion(char* ruta);
void mostrarConfiguracion(TdataNode *dn);
void logAndExit(char * mensaje);

#endif
