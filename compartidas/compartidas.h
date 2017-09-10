/*
 * compartidas.h
 *
 *  Created on: 10/9/2017
 *      Author: utnso
 */

#ifndef COMPARTIDAS_H_
#define COMPARTIDAS_H_

int establecerConexion(char *ip_dest, char *port_dest);
void setupHints(struct addrinfo *hints, int address_family, int socket_type, int flags);
int makeListenSock(char *port_listen);
int makeCommSock(int socket_in);
int handleNewListened(int sock_listen, fd_set *setFD);
void clearAndClose(int *fd, fd_set *setFD);
int validarRespuesta(int sock, tHeader h_esp, tHeader *h_obt);
int handshakeCon(int sock_dest, int id_sender);
int enviarHeader(int sock_dest,tHeader head);

#endif /* COMPARTIDAS_H_ */
