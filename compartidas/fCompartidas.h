/*
 * fCompartidas.h
 *
 *  Created on: 1/9/2017
 *      Author: utnso
 */

#ifndef FCOMPARTIDAS_H_
#define FCOMPARTIDAS_H_

#include "definiciones.h"

int makeListenSock(char *port_listen);
int validarRespuesta(int sock, tPackHeader h_esp, tPackHeader *h_obt);
int handshakeCon(int sock_dest, int id_sender);
void setupHints(struct addrinfo *hints, int address_family, int socket_type, int flags);
int establecerConexion(char *ip_dest, char *port_dest);
int makeCommSock(int socket_in);
int handleNewListened(int sock_listen, fd_set *setFD);

#endif /* FCOMPARTIDAS_H_ */
