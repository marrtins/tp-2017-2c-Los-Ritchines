#ifndef LIB_FUNCIONESFS_H_
#define LIB_FUNCIONESFS_H_

#include "estructurasFS.h"

//serializacion
void empaquetarBloqueAEliminar(Tbuffer * buffer, Theader * head, int numeroDeBloque);
void desempaquetarArchivoFinal(int fileDescriptor, TarchivoFinal * archivoFinal);

//Operaciones
long sumaDeDosNumerosLong(long valor1, long valor2);
long restaDeDosNumerosLong(long valor1, long valor2);
int sumaDeDosNumerosInt(int valor1, int valor2);
int restaDeDosNumerosInt(int valor1, int valor2);
void setearAtributoDeArchivoConfigConInts(t_config * archivoConf, char * key, int value, int(*funcion)(int valor1, int valor2));
int punteroDeStringsContieneString(char ** punteroDeString ,char * string);

//Configuracion
TfileSystem *obtenerConfiguracionFS(char* ruta);
void mostrarConfiguracion(TfileSystem *fileSystem);



//Consola
void procesarCpblock(char ** palabras);
void procesarInput(char * linea);
void consolaFS(void);
void consolaFormat(char**palabras, int cantidadParametros);
void consolaCpto(char** palabras, int cantidad);
void consolaRemove(char** palabras, int cantidadParametros);
void consolaRename(char**palabras, int cantidadParametros);
void consolaCat(char**palabras, int cantidadParametros);
void consolaMkdir(char**palabras, int cantidadParametros);
void consolaCpfrom(char**palabras, int cantidadParametros);
void consolaMd5(char**palabras, int cantidadParametros);
void consolaLs(char**palabras, int cantidadParametros);
void consolaMove(char**palabras, int cantidadParametros);
void consolaInfo(char**palabras, int cantidadParametros);
void consolaCpblock(char ** palabras, int cantidad);


//Conexiones
void conectarNuevoNodo(TpackInfoBloqueDN * infoNodo, int fileDescriptor);
void verificarSiEsEstable();
int sigueEstableParaArchivo(char *rutaArchivoYamafs);
int conectarNuevoCliente( int fileDescriptor, fd_set * masterFD);
void clearAndClose(int fileDescriptor, fd_set* masterFD);
void conexionesDatanode(void * estructura);
void formatearNodos();


int todosLosArchivosSePuedenLevantar();
int sePuedeLevantaElArchivo(char * rutaArchivoYamafs);


//Tablas
void persistirTablaDeDirectorios();
void levantarTablasDirectorios();
void levantarTablaArchivo(Tarchivo * tablaArchivos, char * ruta);
void levantarTablaNodos(Tnodos * tablaNodos);
void liberarTablaDeArchivo(Tarchivo * tablaDeArchivos);
void mostrarTablaArchivo(Tarchivo* tablaArchivo);
void guardarTablaDeArchivo(Tarchivo * archivoAAlmacenar, char * rutaDestino);
void agregarNodoATablaDeNodos(Tnodo * nuevoNodo);
void altaDeNodoEnTablaDeNodos(Tnodo * nodo);
void bajaDeNodoEnTablaDeNodos(Tnodo * nodo);
void agregarElementoAArrayArchivoConfig(t_config * tablaDeNodos, char * key, char * nombreElemento);
void eliminarElementoDeArrayArchivosConfig(t_config * archivoConfig, char * key, char * nombreElemento);
void inicializarTablaDeNodos();
void inicializarTablaDirectorios();
void eliminarNodoDeTablaDeNodos(Tnodo * nuevoNodo);
void ocuparBloqueEnTablaNodos(char * nombreNodo);
void generarArrayParaArchivoConfig(t_config * archivoConfig, char * key, char * dato1, char * dato2);
int cantidadDeBloquesDeUnArchivo(unsigned long long tamanio);
void renombrarArchivo(char * rutaYamafs, char * nombre);
void renombrarDirectorio(char * rutaYamafs, char * nombre);
void formatearTablaDeNodos();
void formatearFS();
void leerArchivoComoTextoPlano(char * rutaLocal);
char * obtenerNombreDeArchivoDeUnaRuta(char * rutaLocal);
void mostrarCsv(char * rutaLocal);
void mostrarBinario(char * rutaLocal);
char * generarStringBloqueNCopias(int nroBloque);
int esRutaYamafs(char * ruta);
void eliminarBloqueDeUnArchivo(char * rutaLocal, int numeroDeBloque, int numeroDeCopia);
int eliminarBloqueDeNodo(Tnodo * nodo, int numeroDeBloque);
void eliminarBloqueDeTablaDeArchivos(t_config * archivo, int numeroDeBloque, int numeroDeCopia);
void agregarCopiaAtablaArchivo(char * rutaLocalArchivo,char * nodo, int bloqueDatabin, int nroBloque);
char * generarStringNodoNLibre(char * nombre);
char * generarStringNodoNTotal(char * nombre);
void levantarEstadoAnteriorDeLaTablaDeNodos(t_list * desconectados);
void removerArchivos(char * ruta);
int eliminarKeyDeArchivo(char * rutaArchivo, char * key);
char * obtenerExtensionDeArchivoDeUnaRuta(char * rutaLocal);
void pasarInfoDeUnArchivoAOtro(char * archivoAMoverMapeado, char * archivoMapeado, unsigned long long tamanio);
char ** obtenerNodosDeUnArchivo(Tarchivo * archivo);
void mostrarPadreEHijosDeUnDirectorio(Tdirectorio * directorio, int nivelDelArbol);
char * generarTabs(int cantidadDeTabs);
void mostrarArbolDeDirectorios(char * rutaADirectorio);
void mostrarDistribucionDeBloquesEnNodos();
void generarDistribucionDeBloquesEnNodos(struct dirent * directorio, t_list * tablaDeArchivosGlobal);
void actualizarEnTablaDeArchivosGlobal(t_list * tablaDeArchivosGlobal, Tarchivo * estructuraArchivo, char * nombreDirectorioPadre);

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
TinfoNodo * buscarInfoNodoPorNombre(t_list * lista, char * nombre);
void liberarNodosDeLista(void * nodo);
void vaciarLista();
void borrarNodoPorNombre(t_list * lista, char * nombre);
Tnodo * buscarNodoDiponibleParaEnviar(t_list * listaDeNodos, int* indice);
Tdirectorio * buscarDirectorioPorIndice(int indice);
t_list * buscarHijosDeUnDirectorio(Tdirectorio * padre);
TelementoDeTablaArchivoGlobal * siNoExisteElNodoAgregar(char * nombreNodo, t_list * tablaDeArchivosGlobal);
TarchivoDeTablaArchivoGlobal * siNoExisteElArchivoAgregar(char * nombreArchivo, t_list * listaDeArchivos);
void ordenarTablaDeArchivosGlobalPorNombre(t_list * tablaDeArchivosGlobal);
void ordenarArchivosDeTablaDeArchivosGlobalPorNombre(t_list * archivos);

//Directorios
void crearRoot();
int existeDirectorio(char * directorio);
int obtenerIndexDeUnaRuta(char * rutaDestino);
int buscarIndexPorNombreDeDirectorio(char * directorio);
Tdirectorio* buscarPorNombreDeDirectorio(char * directorio);
int buscarIndexMayor();
int directorioNoExistente(char ** carpetas);
int crearDirectorio(char * ruta);
int esDirectorio(char * ruta);
int esDirectorioRaiz(char*ruta);
int esDirectorioPadre(char * ruta);
int esDirectorioVacio(char* ruta);
int esArchivo(char* ruta);
char** buscarDirectorios(char * ruta);
char** buscarArchivos(char * ruta);
void listarArchivos(char * ruta);
void moverArchivo(char*rutaArchivoOrigen, char*rutaDirectorioDestino);
void moverDirectorio(char*rutaDirectorioOrigen, char*rutaDirectorioFinal);
void removerDirectorios(char * ruta);
int verificarRutaArchivo(char * rutaYamafs);
int verificarRutaArchivoSinPuts(char * rutaYamafs);
int existeArchivo(int indiceDirectorio , char * rutaYamafs);
char * obtenerRutaSinArchivo(char * ruta);
char * obtenerRutaLocalDeArchivo(char * rutaYamafs);
int esRutaDeUnArchivo(char * ruta);
int removerArchivo(char* ruta);
void removerDirectorio(char*ruta);
void removerDirectorioDeTabla(char* directorio);
int archivoRepetidoEnDirectorio(char* rutaLocalArchivo, char* rutaDestinoYamafs);
void crearDirectorioTemporal();


//Bitmap
t_bitarray* crearBitmap(unsigned int tamanioBitmap);
void mostrarBitmap(t_bitarray* bitmap);
void ocuparBloque(Tnodo * nodo, int bloqueAOcupar);
void inicializarBitmap(Tnodo* bitmap);
void almacenarTodosLosBitmaps(t_list * lista);
void almacenarBitmap(Tnodo * nodo);
void levantarBitmapDeUnNodo(Tnodo * nodo);
void inicializarBitmaps();
void desocuparBloque(Tnodo * nodo, int numeroDeBloque);
void desocuparBloqueEnTablaDeNodos(char * nombreNodo);
void setearACeroMalloc(char * memoria, int tamanioMemoria);

//Almacenar Archivo
int almacenarArchivo(char **palabras);
void enviarBloque(TbloqueAEnviar* bloque, Tarchivo * estructuraArchivoAAlmacenar);
void almacenarEstructuraArchivoEnUnArchivo(Tarchivo * archivoAAlmacenar, char * rutaArchivo);
char * generarStringDeBloqueNCopiaN(int numeroDeBloque, int numeroDeCopia);
char * generarStringDeBloqueNBytes(int numeroDeBloque);
bool ordenarSegunBloquesDisponibles(void* nodo1, void* nodo2);
void almacenarBloquesEnEstructuraArchivo(Tarchivo * archivoAAlmacenar, Tnodo * nodo1, Tnodo * nodo2, TbloqueAEnviar * bloque);

//Obtener Archivo
int levantarArchivo(Tarchivo * archivo, char * ruta);
int copiarArchivo(char ** palabras);

//Nodos
TpackInfoBloqueDN * recvInfoNodo(int socketFS);
TinfoNodo * inicializarInfoNodo(TpackInfoBloqueDN * infoBloqueRecibido);
Tnodo * inicializarNodo(TpackInfoBloqueDN * infoBloqueRecibido, int fileDescriptor, Tnodo * nuevoNodo);
void borrarNodoPorFD(int fd);
void* buscarNodoPorFD(t_list * lista, int fd);
int verificarDisponibilidadDeEspacioEnNodos(unsigned long long tamanio);
int bloquesOcupadosDeNodo(Tnodo * nodo);
int nodosDisponiblesParaBloqueDeArchivo(Tarchivo* tablaArchivo,int nroBloque);
void setearDisponibilidadDeEnvioDeNodos(t_list * listaDeNodos, int valor);
int losNodosConectadosSonLosQueEstabanAntes();


//Archivos
int procesarArchivoSegunExtension(Tarchivo * archivoAAlmacenar, char * nombreArchivo);
void procesarArchivoCsv(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar * infoBloque);
void procesarArchivoBinario(Tarchivo * archivoAAlmacenar, char * archivoMapeado, TbloqueAEnviar * infoBloque);
int pedirBloque(Tarchivo* tablaArchivo,int nroBloque);
int copiarBloque(Tbuffer* buffer, Tbuffer * destino);
int enviarBloqueA(TbloqueAEnviar* bloque, char* nombreNodo);
int getMD5(char* ruta);



//Liberar
void liberarEstructuraBuffer(Tbuffer * buffer);
void liberarEstructuraBloquesAEnviar(TbloqueAEnviar * infoBloque);
void liberarTPackInfoBloqueDN(TpackInfoBloqueDN * bloque);
void liberarTablaDirectorios(void* directorio);
void liberarCopia(void * copia);
void liberarEstructuraArchivoFinal(TarchivoFinal * archivoFinal);
void liberarTPackageInfoNodo (void * infoNodo);
void liberarTpackageUbicacionBloques(void * bloque);
void liberarBloques(void * bloque);
void liberarTInfoArchivoFSYama(TinfoArchivoFSYama * infoArchivo);
void liberarTablaDeArchivosGlobal(t_list * tablaDeArchivosGlobal);
void liberarNodoDeTablaDeArchivosGlobal(TelementoDeTablaArchivoGlobal * nodo);
void liberarArchivosDeTablaDeArchivosGlobal(TarchivoDeTablaArchivoGlobal * archivo);

//YAMA
char * recvRutaArchivo(int socket);
TinfoArchivoFSYama * crearListaTablaArchivoParaYama(Tarchivo * archivo);
void enviarInfoNodoAYama(int socketYama, Tarchivo * archivo);
t_list * generarListaInfoNodos(char ** nodos);

#endif
