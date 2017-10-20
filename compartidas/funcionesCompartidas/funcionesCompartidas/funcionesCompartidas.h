#ifndef COMPARTIDAS_FUNCIONESCOMPARTIDAS_H_
#define COMPARTIDAS_FUNCIONESCOMPARTIDAS_H_

#include <tiposDatos/estructurasCompartidas.h>

void logAndExit(char * mensaje);
void setupHints(struct addrinfo *hints, int familiaDeProtocolos, int tipoDeProtocolo, int flags);
int crearSocketDeEscucha(char*);
void crearHilo(pthread_t * nombreHilo, void * nombreFuncion, void * parametros);
int aceptarCliente(int fileDescriptor);
int conectarAServidor(char *ipDestino, char *puertoDestino);
int enviarHeader(int socketDestino,Theader * head);
char *recvGenericWFlags(int sock_in, int flags);
char *recvGeneric(int sock_in);
int tamanioArchivo(FILE *archivo);
void freeAndNULL(void **ptr);
int contarPunteroDePunteros(char ** puntero);
void liberarPunteroDePunterosAChar(char** palabras);
int cantidadParametros(char **palabras);
char * obtenerUltimoElementoDeUnSplit(char ** palabras);
char* deUnsignedLongLongAString(int number);
char * obtenerExtensionDeUnArchivo(char * nombreArchivoConExtension);
char * obtenerNombreDeArchivoSinExtension(char * nombreDeArchivoConExtension);

#endif
