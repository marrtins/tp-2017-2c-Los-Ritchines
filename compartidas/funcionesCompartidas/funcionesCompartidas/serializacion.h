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

char *empaquetarBloque(Theader * head, int nroBloque, unsigned long long tamanio, char *contenido);
#endif
