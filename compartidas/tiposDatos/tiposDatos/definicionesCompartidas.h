#ifndef COMPARTIDAS_DEFINICIONESCOMPARTIDAS_H_
#define COMPARTIDAS_DEFINICIONESCOMPARTIDAS_H_

#include "includesCompartidos.h"

#define MAXIMO(A,B) ((A) > (B) ? (A) : (B))
#define MAXIMA_LONGITUD_PUERTO 6
#define MAXIMA_LONGITUD_IP 16
#define BACKLOG 20
#define HEAD_SIZE 8
#define TAMANIO_EXTENSION_ARCHIVO 10
#define TAMANIO_NOMBRE_NODO 7
#define TAMANIO_BLOQUE_N_COPIA_N 15
#define TAMANIO_NOMBRE_TEMPORAL 25
#define BLOQUE_SIZE 1024*1024



#ifndef MUX_LOCK_M
#define MUX_LOCK(M) (pthread_mutex_lock(M))
#endif
#ifndef MUX_UNLOCK_M
#define MUX_UNLOCK(M) (pthread_mutex_unlock(M))
#endif



#endif
