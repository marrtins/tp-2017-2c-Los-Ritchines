#ifndef LIB_FUNCIONESFS_H_
#define LIB_FUNCIONESFS_H_

#include "estructurasFS.h"

void logAndExit(char * mensaje);
TfileSystem *obtenerConfiguracion(char* ruta);
void mostrarConfiguracion(TfileSystem *fileSystem);
void liberarPunteroDePunterosAChar(char** palabras);
void procesarInput(char * palabras);
void consolaFS(void);
int conectarNuevoCliente( int fileDescriptor, fd_set * masterFD);
void clearAndClose(int fileDescriptor, fd_set* masterFD);
void freeEstructuraAnidada(Tarchivos * tabla, int cantBloques);
void levantarTablasDirectorios(Tdirectorios * tablaDirectorios);
void levantarTablaArchivos(Tarchivos * tablaArchivos);
void levantarTablasNodos();
void levantarTablas(Tdirectorios * tablaDirectorios);
t_bitarray* crearBitmap(int tamanioBitmap);
void conexionesDatanode(void * estructura);

#endif
