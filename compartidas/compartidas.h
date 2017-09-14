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

/* Dado un archivo, lo lee e inserta en un paquete de codigo fuente
 */
tPackSrcCode *readFileIntoPack(tProceso sender, char* ruta);

unsigned long fsize(FILE* f);

/* Medida de seguridad. No solo hace free(pointer) sino que reasigna el pointer a NULL,
 * de esta manera, si se usare accidentalmente a futuro, es mas seguro que no toque nada critico
 */
void freeAndNULL(void **pointer);


/* Para mandar solamente un Header. Util para informar fallos */
char *serializeHeader(tHeader head, int *pack_size);

/* Serializa un buffer de bytes para que respete el protocolo de HEADER
 * int *pack_size se usa para almacenar el size del paquete serializado, asi se lo puede send'ear
 */
char *serializeBytes(tHeader head, char* buffer, int buffer_size, int *pack_size);

/* Deserializa un buffer en un Paquete de Bytes
 */
tPackBytes *deserializeBytes(char *bytes_serial);


/* Funcion generica de recepcion. Toda serializacion deberia dar un paquete
 * que responda al formato |HEAD(8)|PAYLOAD_SIZE(int)|PAYLOAD(char*)|
 * Luego cada funcion de deserializacion se debe encargar de interpretar el PAYLOAD
 *  !!!!!! Esta funcion comprende que ya se recibio el HEAD de 8 bytes !!!!!!
 */
char *recvGeneric(int sock_in);
char *recvGenericWFlags(int sock_in, int flags);


#endif /* COMPARTIDAS_H_ */
