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
void liberarTablaDeArchivo(Tarchivo * tablaDeArchivos);
void guardarTablaDeArchivo(Tarchivo * archivoAAlmacenar, char * rutaDestino);
void agregarNodoATablaDeNodos(Tnodo * nuevoNodo);
char * agregarNodoAArrayDeNodos(char ** nodos, char * nombreNodo);
void inicializarTablaDeNodos();
void eliminarNodoDeTablaDeNodos(Tnodo * nuevoNodo);

//Listas
bool ordenarListaPorMayor(void * directorio1, void * directorio2);

//Bitmap
t_bitarray* crearBitmap(int tamanioBitmap);
void mostrarBitmap(t_bitarray* bitmap);
void ocuparProximoBloqueBitmap(Tnodo * nodo);

//Directorios
int existeDirectorio(char * directorio);
int obtenerIndexDeUnaRuta(char * rutaDestino);
int buscarIndexPorNombreDeDirectorio(char * directorio);
void* buscarPorNombreDeDirectorio(char * directorio);
int buscarIndexMayor();
int directorioNoExistente(char ** carpetas);
int crearDirectorio(char * ruta);

//Almacenar Archivo
void almacenarArchivo(char **palabras);
void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar);
void almacenarEstructuraArchivoEnUnArchivo(Tarchivo * archivoAAlmacenar, char * rutaArchivo);
char * generarStringDeBloqueNCopiaN(int numeroDeBloque, int numeroDeCopia);
char * generarStringDeBloqueNBytes(int numeroDeBloque);
bool ordenarSegunBloquesDisponibles(void* nodo1, void* nodo2);
void almacenarBloquesEnEstructuraArchivo(Tarchivo * archivoAAlmacenar, Tnodo * nodo1, Tnodo * nodo2, TbloqueAEnviar);

//Nodos
TpackInfoBloqueDN * recvInfoNodo(int socketFS);
Tnodo * inicializarNodo(TpackInfoBloqueDN * infoBloqueRecibido, int fileDescriptor);
void borrarNodoPorFD(int fd);
void* buscarNodoPorFD(int fd);
void borrarNodoDesconectadoPorFD(int fd);
int verificarDisponibilidadDeEspacioEnNodos(unsigned long long tamanio);

//md5
int getMD5(char**palabras);

//Archivos
int procesarArchivoSegunExtension(Tarchivo * archivoAAlmacenar, char * nombreArchivo);
void procesarArchivoCsv(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar * infoBloque);
void procesarArchivoBinario(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar * infoBloque);

//Liberar
void liberarEstructuraBuffer(Tbuffer * buffer);

#endif
