#ifndef COMPARTIDAS_FUNCIONESCOMPARTIDAS_H_
#define COMPARTIDAS_FUNCIONESCOMPARTIDAS_H_

#include <tiposDatos/estructurasCompartidas.h>

void logErrorAndExit(char * mensaje);
void inicializarArchivoDeLogs(char * ruta);
void setupHints(struct addrinfo *hints, int familiaDeProtocolos, int tipoDeProtocolo, int flags);
int crearSocketDeEscucha(char*);
void crearHilo(pthread_t * nombreHilo, void * nombreFuncion, void * parametros);
int aceptarCliente(int fileDescriptor);
int conectarAServidor(char *ipDestino, char *puertoDestino);
int enviarHeader(int socketDestino,Theader * head);
char *recvGenericWFlags(int sock_in, int flags);
char *recvGeneric(int sock_in);


#endif
