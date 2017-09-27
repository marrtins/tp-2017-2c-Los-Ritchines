#ifndef LIB_FUNCIONES_H_
#define LIB_FUNCIONES_H_

#include "estructuras.h"

void logAndExit(char * mensaje);
Tyama *obtenerConfiguracion(char* ruta);
void mostrarConfiguracion(Tyama *yama);
int conectarAFS(int* socketFS, Tyama *yama);
char *recvGenericWFlags(int sock_in, int flags);
char *recvGeneric(int sock_in);
TpackBytes *deserializeBytes(char *bytes_serial);
void freeAndNULL(void **ptr);
void masterHandler(void *client_sock);

#endif
