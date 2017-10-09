#ifndef LIB_FUNCIONESDN_H_
#define LIB_FUNCIONESDN_H_

#include "estructurasDN.h"

TdataNode *obtenerConfiguracionDN(char* ruta);
void mostrarConfiguracion(TdataNode *dn);
void setBloque(int posicion, char* bloque);
char * getBloque(int posicion);

#endif
