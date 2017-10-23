#ifndef LIB_FUNCIONESFS_H_
#define LIB_FUNCIONESFS_H_

#include "estructurasFS.h"

//Configuracion
TfileSystem *obtenerConfiguracionFS(char* ruta);
void mostrarConfiguracion(TfileSystem *fileSystem);

//Consola
void procesarInput(char * palabras);
void consolaFS(void);

//Conexiones
int conectarNuevoCliente( int fileDescriptor, fd_set * masterFD);
void clearAndClose(int fileDescriptor, fd_set* masterFD);
void conexionesDatanode(void * estructura);

//Tablas
void levantarTablasDirectorios();
void levantarTablaArchivo(Tarchivo * tablaArchivos);
void levantarTablaNodos(Tnodos * tablaNodos);
void levantarTablas(Tnodos * tablaNodos);
void liberarTablaDeArchivo(Tarchivo * tablaDeArchivos);
void guardarTablaDeArchivo(Tarchivo * archivoAAlmacenar, char * rutaDestino);

//Bitmap
t_bitarray* crearBitmap(int tamanioBitmap);
void mostrarBitmap(t_bitarray* bitmap);
void ocuparProximoBloqueBitmap(Tnodo * nodo);

//Directorios
int existeDirectorio(char * directorio);
int obtenerIndexDeUnaRuta(char * rutaDestino);
int buscarIndexPorNombreDeDirectorio(char * directorio);
void* buscarPorNombreDeDirectorio(char * directorio);
void crearDirectorio(char * ruta);

//Almacenar Archivo
void almacenarArchivo(char **palabras);
void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar);
void almacenarEstructuraArchivoEnUnArchivo(Tarchivo * archivoAAlmacenar, char * rutaArchivo);
char * generarStringDeBloqueNCopiaN(int numeroDeBloque, int numeroDeCopia);
char * generarStringDeBloqueNBytes(int numeroDeBloque);
bool ordenarSegunBloquesDisponibles(void* nodo1, void* nodo2);

//Nodos
TpackInfoBloqueDN * recvInfoNodo(int socketFS);
Tnodo * inicializarNodo(TpackInfoBloqueDN * infoBloqueRecibido, int fileDescriptor);
void borrarNodoPorFD(int fd);
void* buscarNodoPorFD(int fd);
void borrarNodoDesconectadoPorFD(int fd);

//md5
int getMD5(char**palabras);

//Archivoa
void procesarArchivoSegunExtension(Tarchivo * archivoAAlmacenar, char * archivoMapeado);
void procesarArchivoCsv(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar * infoBloque);
void procesarArchivoBinario(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar * infoBloque);

//Liberar
void liberarEstructuraBuffer(Tbuffer * buffer);

#endif
