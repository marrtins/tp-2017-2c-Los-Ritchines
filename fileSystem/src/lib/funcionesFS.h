#ifndef LIB_FUNCIONESFS_H_
#define LIB_FUNCIONESFS_H_

#include "estructurasFS.h"

TfileSystem *obtenerConfiguracionFS(char* ruta);
void mostrarConfiguracion(TfileSystem *fileSystem);
void liberarPunteroDePunterosAChar(char** palabras);
void procesarInput(char * palabras);
void consolaFS(void);
int conectarNuevoCliente( int fileDescriptor, fd_set * masterFD);
void clearAndClose(int fileDescriptor, fd_set* masterFD);
void freeEstructuraAnidada(Tarchivos * tabla, int cantBloques);
void levantarTablasDirectorios(Tdirectorios * tablaDirectorios);
void levantarTablaArchivos(Tarchivos * tablaArchivos);
void levantarTablaNodos(Tnodos * tablaNodos);
void levantarTablas(Tdirectorios * tablaDirectorios, Tnodos * tablaNodos);
t_bitarray* crearBitmap(int tamanioBitmap);
void mostrarBitmap(t_bitarray* bitmap);
void conexionesDatanode(void * estructura);
void liberarTablaDeArchivos(Tarchivos * tablaDeArchivos);

#endif
