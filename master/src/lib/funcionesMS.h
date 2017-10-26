#ifndef LIB_FUNCIONESMS_H_
#define LIB_FUNCIONESMS_H_

#include "estructurasMS.h"

Tmaster * obtenerConfiguracionMaster(char * ruta);
void mostrarConfiguracion(Tmaster *master);



TpackInfoBloque *recibirInfoBloque(int sockYama);
int enviarScript(char * rutaScript,int sockDestino);



//-----Funciones Hilo Worker Transf//
int conectarseAWorkersTransformacion(t_list * bloquesTransformacion,int sockYama);
int conectarseAWorkerParaReplanificarTransformacion(TpackInfoBloque *infoBloque,int sockYama);
void hiloWorkerTransformacion(void *info);



#endif
