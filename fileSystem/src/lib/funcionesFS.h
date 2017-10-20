#ifndef LIB_FUNCIONESFS_H_
#define LIB_FUNCIONESFS_H_

#include "estructurasFS.h"

TfileSystem *obtenerConfiguracionFS(char* ruta);
void mostrarConfiguracion(TfileSystem *fileSystem);
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
void almacenarArchivo(char **palabras);
void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar);
int existeDirectorio(char * directorio);
void ocuparProximoBloqueBitmap(Tnodo * nodo);
void guardarTablaDeArchivo(Tarchivo * archivoAAlmacenar, char * rutaDestino);
int obtenerIndexDeUnaRuta(char * rutaDestino);
int buscarIndexPorNombreDeDirectorio(char * directorio);
void almacenarEstructuraArchivoEnUnArchivo(Tarchivo * archivoAAlmacenar, char * rutaArchivo);
char * generarStringDeBloqueNCopiaN(int numeroDeBloque, int numeroDeCopia);
char * generarStringDeBloqueNBytes(int numeroDeBloque);
bool ordenarSegunBloquesDisponibles(void* nodo1, void* nodo2);
void* buscarPorNombreDeDirectorio(char * directorio);
TpackInfoBloqueDN * recvInfoNodo(int socketFS);
void liberarEstructuraBuffer(Tbuffer * buffer);
void crearDirectorio(char * ruta);
int getMD5(char**palabras);
Tnodo * inicializarNodo(TpackInfoBloqueDN * infoBloqueRecibido, int fileDescriptor);
void borrarNodoPorFD(int fd);
void* buscarNodoPorFD(int fd);
void borrarNodoDesconectadoPorFD(int fd);


#endif
