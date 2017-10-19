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
void freeEstructuraAnidada(Tarchivo * tabla, int cantBloques);
void levantarTablasDirectorios();
void levantarTablaArchivo(Tarchivo * tablaArchivos);
void levantarTablaNodos(Tnodos * tablaNodos);
void levantarTablas(Tnodos * tablaNodos);
t_bitarray* crearBitmap(int tamanioBitmap);
void mostrarBitmap(t_bitarray* bitmap);
void conexionesDatanode(void * estructura);
void liberarTablaDeArchivo(Tarchivo * tablaDeArchivos);
int cantidadParametros(char **palabras);
void almacenarArchivo(char **palabras);
void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar);
int existeDirectorio(char * directorio);
void ocuparProximoBloqueBitmap(Tnodo * nodo);
void guardarTablaDeArchivo(Tarchivo * archivoAAlmacenar, char * rutaDestino);
int obtenerIndexDeUnaRuta(char * rutaDestino);
char * obtenerUltimoElementoDeUnSplit(char ** palabras);
int buscarIndexPorNombreDeDirectorio(char * directorio);
void almacenarEstructuraArchivoEnArchivoAbierto(Tarchivo * archivoAAlmacenar, char * rutaArchivo);
char * generarStringDeBloqueNCopiaN(int numeroDeBloque, int numeroDeCopia);
char * generarStringDeBloqueNBytes(int numeroDeBloque);
bool ordenarSegunBloquesDisponibles(void* nodo1, void* nodo2);

#endif
