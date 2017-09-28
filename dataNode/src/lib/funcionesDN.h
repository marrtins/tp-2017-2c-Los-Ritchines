#ifndef LIB_FUNCIONESDN_H_
#define LIB_FUNCIONESDN_H_

#include "estructurasDN.h"

TdataNode *obtenerConfiguracion(char* ruta);
void mostrarConfiguracion(TdataNode *dn);
void logAndExit(char * mensaje);

#endif
