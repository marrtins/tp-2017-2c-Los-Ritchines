#ifndef LIB_FUNCIONESYM_H_
#define LIB_FUNCIONESYM_H_

#include "estructurasYM.h"

void logAndExit(char * mensaje);
Tyama *obtenerConfiguracionYama(char* ruta);
void mostrarConfiguracion(Tyama *yama);
void conectarAFS(int* socketFS, Tyama *yama);
char *recvGenericWFlags(int sock_in, int flags);
char *recvGeneric(int sock_in);
TpackBytes *deserializeBytes(char *bytes_serial);
void freeAndNULL(void **ptr);
void masterHandler(void *client_sock);

#endif
