#ifndef LIB_FUNCIONESDN_H_
#define LIB_FUNCIONESDN_H_

#include "estructurasDN.h"

//Configuracion
TdataNode *obtenerConfiguracionDN(char* ruta);
void mostrarConfiguracion(TdataNode *dn);

//Bloque
void setBloque(int posicion, Tbloque* bloque);
char * getBloque(int posicion);

//Envios
int enviarInfoNodo(int socketFS, TdataNode * dataNode);

//Recv
Tbloque * recvBloque(int socketFS);
#endif
