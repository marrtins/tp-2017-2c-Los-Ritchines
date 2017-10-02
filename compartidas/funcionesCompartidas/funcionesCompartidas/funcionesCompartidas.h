#ifndef COMPARTIDAS_FUNCIONESCOMPARTIDAS_H_
#define COMPARTIDAS_FUNCIONESCOMPARTIDAS_H_

#include "serializacion.h"

void logAndExit(char * mensaje);
void setupHints(struct addrinfo *hints, int familiaDeProtocolos, int tipoDeProtocolo, int flags);
int crearSocketDeEscucha(char*);
void crearHilo(pthread_t * nombreHilo, void * nombreFuncion, void * parametros);
int aceptarCliente(int fileDescriptor);
int conectarAServidor(char *ipDestino, char *puertoDestino);
int enviarHeader(int socketDestino,Theader * head);


#endif
