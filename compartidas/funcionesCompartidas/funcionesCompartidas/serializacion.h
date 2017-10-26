#ifndef FUNCIONESCOMPARTIDAS_FUNCIONESCOMPARTIDAS_SERIALIZACION_H_
#define FUNCIONESCOMPARTIDAS_FUNCIONESCOMPARTIDAS_SERIALIZACION_H_

#include "funcionesCompartidas.h"

char * empaquetarRutasYamafs(Theader *head,char *rutaArchivoAReducir,char* rutaResultado);
void desempaquetarRutasYamafs(TpackageRutas * estructuraDeRutas, int socket);

char *serializeInfoBloque(Theader head, TpackInfoBloque * infoBloque, int *pack_size);
TpackInfoBloque *deserializeInfoBloque(char *bytes_serial);

char *serializeBytes(Theader head, char* buffer, int buffer_size, int *pack_size);
TpackBytes *deserializeBytes(char *bytes_serial);

TpackSrcCode *readFileIntoPack(Tproceso sender, char* ruta);
unsigned long fsize(FILE* f);

Tbuffer *empaquetarBloque(Theader * head, TbloqueAEnviar* bloque, Tnodo* nodo);
Tbuffer * empaquetarInfoBloqueDNaFS(TpackInfoBloqueDN * infoBloque);
TpackInfoBloqueDN * desempaquetarInfoNodo(TpackInfoBloqueDN * infoBloque, char * nombreNodo, char * ipNodo, char * puertoNodo);

char *serializarInfoTransformacionMasterWorker(Theader head,int nroBloque, int bytesOcupadosBloque,int nombreTemporalLen ,char* nombreTemporal, int *pack_size);
TpackDatosTransformacion *deserializarInfoTransformacionMasterWorker(char *bytes_serial);

int enviarHeaderYValor(Theader head, int valorAEnviar,int socketDestino);
int recibirValor(int fd);

#endif
