/*
 * filesystem.h
 *
 *  Created on: 1/9/2017
 *      Author: utnso
 */



#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
#include <stdbool.h>
#define MAX_PORT_LEN 6
#define MAX_IP_LEN 16

#ifndef MAX_AB
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif


#include "../compartidas/definiciones.h"

typedef struct{

	char* puerto_entrada;
	char* puerto_datanode;
	char* puerto_yama;
	char* ip_yama;
	int tipo_de_proceso;
}tFS;

typedef struct {
	int sock_lis_datanode,
		fd_max;
	fd_set master;
} fs_socks;

typedef struct{
	int fd_dn;
	int id_dn;
	char* nombreNodo;
	char* ipWorker;
	char* puertoWorker;
}t_infoNodo;

typedef struct {
	int index;
	char nombre[255];
	int padre;
}t_directory;

tFS *getConfigFilesystem(char* ruta);

void mostrarConfiguracion(tFS *fileSystem);
bool sistemaEstable();
void consolaFS(void);

int establecerConexion(char *ip_dest, char *port_dest);
int validarRespuesta(int sock, tHeader h_esp, tHeader *h_o);

int yamaHandler(tMensaje msjRecibido);
int datanodeHandler(tMensaje msjRecibido,int fd_dn);
int crearListenYama();


//int conectarConFS(tFS *fileSystem);

#endif /* FILESYSTEM_H_ */
