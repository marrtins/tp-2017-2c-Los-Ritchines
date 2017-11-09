#ifndef FUNCIONESCOMPARTIDAS_FUNCIONESCOMPARTIDAS_SERIALIZACION_H_
#define FUNCIONESCOMPARTIDAS_FUNCIONESCOMPARTIDAS_SERIALIZACION_H_

#include "funcionesCompartidas.h"

char *serializeInfoBloque(Theader head, TpackInfoBloque * infoBloque, int *pack_size);
TpackInfoBloque *deserializeInfoBloque(char *bytes_serial);

char *serializeBytes(Theader head, char* buffer, int buffer_size, int *pack_size);
TpackBytes *deserializeBytes(char *bytes_serial);

TpackSrcCode *readFileIntoPack(Tproceso sender, char* ruta);
unsigned long fsize(FILE* f);

Tbuffer *empaquetarBloque(Theader * head, TbloqueAEnviar* bloque, Tnodo* nodo);
Tbuffer * empaquetarInfoNodo(TpackInfoBloqueDN * infoBloque);
TpackInfoBloqueDN * desempaquetarInfoNodo(TpackInfoBloqueDN * infoBloque, char * nombreNodo, char * ipNodo, char * puertoNodo);

Tbuffer * empaquetarBytesMasInt(Theader* head, char * bytes, int numero);
Tbuffer *empaquetarBytes(Theader * head, char * bytes);
Tbuffer * empaquetarInt(Theader * head, int numero);

Tbuffer * empaquetarPeticionBloque(Theader* head, int nroBloque, unsigned long long tamanioBloque);

char *serializarInfoTransformacionMasterWorker(Theader head,int nroBloque, int bytesOcupadosBloque,int nombreTemporalLen ,char* nombreTemporal, int *pack_size);
TpackDatosTransformacion *deserializarInfoTransformacionMasterWorker(char *bytes_serial);

int enviarHeaderYValor(Theader head, int valorAEnviar,int socketDestino);
int recibirValor(int fd);

char *serializeInfoReduccionLocal(Theader head, TreduccionLocal * infoReduccion, int *pack_size);
TreduccionLocal *deserializeInfoReduccionLocal(char *bytes_serial);

char *serializarInfoReduccionLocalMasterWorker(Theader head,int nombreTemporalReduccionLen,char * nombreTemporalReduccion,t_list * listaTemporales, int *pack_size);
TinfoReduccionLocalMasterWorker *deserializarInfoReduccionLocalMasterWorker(char *bytes_serial);

char *serializeInfoReduccionGlobal(Theader head, TreduccionGlobal * infoReduccionGlobal, int *pack_size);
TreduccionGlobal *deserializeInfoReduccionGlobal(char *bytes_serial);

char *serializeInfoAlmacenadoFinal(Theader head, TinfoAlmacenadoFinal * infoAlmacenado, int *pack_size);
TinfoAlmacenadoFinal *deserializeInfoAlmacenadoFinal(char *bytes_serial);

char *serializeInfoAlmacenadoFinalMasterWorker(Theader head, TinfoAlmacenadoMasterWorker * infoAlmacenado, int *pack_size);
TinfoAlmacenadoMasterWorker *deserializeInfoAlmacenadoMasterWorker(char *bytes_serial);

Tbuffer * serializarInfoArchivoYamaFS(Theader *head,TinfoArchivoFSYama *infoArchivo);
TinfoArchivoFSYama *deserializarInfoArchivoYamaFS(char * buffer);

#endif
