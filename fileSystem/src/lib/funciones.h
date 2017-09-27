#ifndef LIB_FUNCIONES_H_
#define LIB_FUNCIONES_H_

#include "estructuras.h"

void logAndExit(char * mensaje);
TfileSystem *obtenerConfiguracion(char* ruta);
void mostrarConfiguracion(TfileSystem *fileSystem);
void liberarPunteroDePunterosAChar(char** palabras);
void procesarInput(char * palabras);
void consolaFS(void);
int conectarNuevoCliente( int fileDescriptor, fd_set * masterFD);
void clearAndClose(int fileDescriptor, fd_set* masterFD);
void levantarTablasDirectorios(Tdirectorios * tablaDirectorios);
void levantarTablasArchivos(Tarchivos * tablaArchivos);
void levantarTablasNodos();
void levantarTablas(Tdirectorios * tablaDirectorios, t_list * listaBitMaps);

#endif
