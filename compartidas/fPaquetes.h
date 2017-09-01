/*
 * fPaquetes.h
 *
 *  Created on: 1/9/2017
 *      Author: utnso
 */

#ifndef FPAQUETES_H_
#define FPAQUETES_H_

#include "definiciones.h"

char *recvGeneric(int sock_in);
int recibirInfoProcSimple(int sock, tPackHeader h_esp, int *var);
char *recvGenericWFlags(int sock_in, int flags);

#endif /* FPAQUETES_H_ */
