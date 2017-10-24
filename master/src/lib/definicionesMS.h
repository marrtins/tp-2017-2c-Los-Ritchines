#ifndef LIB_DEFINICIONESMS_H_
#define LIB_DEFINICIONESMS_H_

#include "includesMS.h"
int conectarseAWorkersTransformacion(t_list * bloquesTransformacion,int sockYama);
void workerHandler(void *info);

unsigned long fsize2(char* ruta);
#endif
