#ifndef LIB_FUNCIONESMS_H_
#define LIB_FUNCIONESMS_H_

#include "estructurasMS.h"

Tmaster * obtenerConfiguracionMaster(char * ruta);
void mostrarConfiguracion(Tmaster *master);



TpackInfoBloque *recibirInfoBloque(int sockYama);
int enviarScript(char * rutaScript,int sockDestino);

TreduccionLocal *recibirInfoReduccionLocal(int sockYama);


//-----Funciones Hilo Worker Transf//
int conectarseAWorkersTransformacion(t_list * bloquesTransformacion,int sockYama);
int conectarseAWorkerParaReplanificarTransformacion(TpackInfoBloque *infoBloque,int sockYama);
void hiloWorkerTransformacion(void *info);

int conectarseAWorkerParaReduccionLocal(TreduccionLocal *infoReduccion,int sockYama);
int conectarseAWorkerParaReduccionGlobal(TreduccionGlobal *infoReduccion,int sockYama);

TreduccionGlobal *recibirInfoReduccionGlobal(int sockYama);

#endif
