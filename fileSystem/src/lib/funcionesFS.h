#ifndef LIB_FUNCIONESFS_H_
#define LIB_FUNCIONESFS_H_

#include "estructurasFS.h"

//Operaciones
long sumaDeDosNumerosLong(long valor1, long valor2);
long restaDeDosNumerosLong(long valor1, long valor2);
int sumaDeDosNumerosInt(int valor1, int valor2);
int restaDeDosNumerosInt(int valor1, int valor2);
void setearAtributoDeArchivoConfigConInts(t_config * archivoConf, char * key, int value, int(*funcion)(int valor1, int valor2));

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
void persistirTablaDeDirectorios();
void levantarTablasDirectorios();
void levantarTablaArchivo(Tarchivo * tablaArchivos, char * ruta);
void levantarTablaNodos(Tnodos * tablaNodos);
void liberarTablaDeArchivo(Tarchivo * tablaDeArchivos);
void mostrarTablaArchivo(Tarchivo* tablaArchivo);
void guardarTablaDeArchivo(Tarchivo * archivoAAlmacenar, char * rutaDestino);
void agregarNodoATablaDeNodos(Tnodo * nuevoNodo);
void agregarElementoAArrayArchivoConfig(t_config * tablaDeNodos, char * key, char * nombreElemento);
void eliminarElementoDeArrayArchivosConfig(t_config * archivoConfig, char * key, char * nombreElemento);
void inicializarTablaDeNodos();
void inicializarTablaDirectorios();
void eliminarNodoDeTablaDeNodos(Tnodo * nuevoNodo);
void ocuparBloqueEnTablaArchivos(char * nombreNodo);
void generarArrayParaArchivoConfig(t_config * archivoConfig, char * key, char * dato1, char * dato2);
int cantidadDeBloquesDeUnArchivo(unsigned long long tamanio);


//Listas
bool ordenarListaPorMayor(void * directorio1, void * directorio2);
int sumarListasPorTamanioDatabin();
void* buscarNodoDesconectadoPorFD(int fd);
bool ordenarSegunBloquesDisponibles(void * nodo1, void * nodo2);
Tnodo * obtenerNodoPorTamanioMaximo();
int sumarBloquesLibresDeNodoSinElMaximo(Tnodo * maximo);
void mostrarListaDeNodos(t_list * lista);
void buscarLosDosNodosConMasDisponibilidad(t_list * lista, Tnodo * nodo1, Tnodo * nodo2);
Tnodo * buscarNodoPorNombre(t_list * lista, char * nombre);

//Bitmap
t_bitarray* crearBitmap(int tamanioBitmap);
void mostrarBitmap(t_bitarray* bitmap);
void ocuparProximoBloque(Tnodo * nodo);

//Directorios
int existeDirectorio(char * directorio);
int obtenerIndexDeUnaRuta(char * rutaDestino);
int buscarIndexPorNombreDeDirectorio(char * directorio);
void* buscarPorNombreDeDirectorio(char * directorio);
int buscarIndexMayor();
int directorioNoExistente(char ** carpetas);
int crearDirectorio(char * ruta);
int esDirectorio(char * ruta);
int esArchivo(char* ruta);
char** buscarDirectorios(char * ruta);
char** buscarArchivos(char * ruta);
void listarArchivos(char * ruta);
void removerDirectorios(char * ruta);
int verificarRutaArchivo(char * rutaYamafs);
int existeArchivo(int indiceDirectorio , char * rutaYamafs);
char * obtenerRutaSinArchivo(char * ruta);
char * obtenerRutaLocalDeArchivo(char * rutaYamafs);
int validarQueLaRutaTengaElNombreDelArchivo(char * ruta);

//Almacenar Archivo
void almacenarArchivo(char **palabras);
void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar);
void almacenarEstructuraArchivoEnUnArchivo(Tarchivo * archivoAAlmacenar, char * rutaArchivo);
char * generarStringDeBloqueNCopiaN(int numeroDeBloque, int numeroDeCopia);
char * generarStringDeBloqueNBytes(int numeroDeBloque);
bool ordenarSegunBloquesDisponibles(void* nodo1, void* nodo2);
void almacenarBloquesEnEstructuraArchivo(Tarchivo * archivoAAlmacenar, Tnodo * nodo1, Tnodo * nodo2, TbloqueAEnviar * bloque);

//Obtener Archivo
void pedirBloques(Tarchivo * archivo);
void copiarArchivo(char ** palabras);

//Nodos
TpackInfoBloqueDN * recvInfoNodo(int socketFS);
Tnodo * inicializarNodo(TpackInfoBloqueDN * infoBloqueRecibido, int fileDescriptor, Tnodo * nuevoNodo);
void borrarNodoPorFD(int fd);
void* buscarNodoPorFD(int fd);
void borrarNodoDesconectadoPorFD(int fd);
int verificarDisponibilidadDeEspacioEnNodos(unsigned long long tamanio);
int bloquesOcupadosDeNodo(Tnodo * nodo);

//md5
int getMD5(char**palabras);

//Archivos
int procesarArchivoSegunExtension(Tarchivo * archivoAAlmacenar, char * nombreArchivo);
void procesarArchivoCsv(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar * infoBloque);
void procesarArchivoBinario(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar * infoBloque);

//Liberar
void liberarEstructuraBuffer(Tbuffer * buffer);
void liberarEstructuraBloquesAEnviar(TbloqueAEnviar * infoBloque);
void liberarTPackInfoBloqueDN(TpackInfoBloqueDN * bloque);

//YAMA
char * recvRutaArchivo(int socket);
Tbuffer * empaquetarInfoArchivo(Theader* head, Tarchivo * archivo);

#endif
