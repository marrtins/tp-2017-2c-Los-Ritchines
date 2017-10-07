#ifndef FUNCIONESCOMPARTIDAS_FUNCIONESCOMPARTIDAS_SERIALIZACION_H_
#define FUNCIONESCOMPARTIDAS_FUNCIONESCOMPARTIDAS_SERIALIZACION_H_

#include <tiposDatos/estructurasCompartidas.h>

char * empaquetarRutasYamafs(Theader *head,char *rutaArchivoAReducir,char* rutaResultado);
void desempaquetarRutasYamafs(TpackageRutas * estructuraDeRutas, int socket);

#endif
