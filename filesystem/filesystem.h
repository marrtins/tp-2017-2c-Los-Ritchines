/*
 * filesystem.h
 *
 *  Created on: 1/9/2017
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#define MAX_PORT_LEN 6
#define MAX_IP_LEN 16

#ifndef MAX_AB
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif

typedef struct{

	char* puerto_entrada;
	char* puerto_datanode;
	char* puerto_yama;
	int tipo_de_proceso;
}tFS;

typedef struct {
	int sock_lis_datanode,
		fd_max;
	fd_set master;
} fs_socks;

typedef struct{
	int fd_dn,id_dn;
}t_datanode;


tFS *getConfigFilesystem(char* ruta);

void mostrarConfiguracion(tFS *fileSystem);

//int conectarConFS(tFS *fileSystem);

#endif /* FILESYSTEM_H_ */
