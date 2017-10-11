#ifndef LIB_DEFINICIONESYM_H_
#define LIB_DEFINICIONESYM_H_

#include "includesYM.h"


#define MAXSIZETEMPNAME 35
#define YESCLOCK 87
#define NOCLOCK 88


typedef struct {
	TpackageInfoNodo infoNodo;
	int availability;
	int disponibilidadBase;
	int pwl;
	bool clock;


}Tplanificacion;

typedef struct {
	int nombreLen;
	char * nombreNodo;
	int tareasRealizadas;
}ThistorialTareas;


#endif
