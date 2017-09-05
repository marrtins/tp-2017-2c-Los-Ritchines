/*
 * filesystem.c
 *
 *  Created on: 31/8/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include "filesystem.h"
#include <commons/config.h>
#include <commons/collections/list.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include <netinet/in.h>

#include "../compartidas/definiciones.h"



#define BACKLOG 20
tFS *fileSystem;
t_list *listaNodos;

int main(int argc, char* argv[]){

	if(argc!=2){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}

	bool estable=false;

	list_create(listaNodos);

	fileSystem=getConfigFilesystem(argv[1]);
	mostrarConfiguracion(fileSystem);


	int stat, ready_fds;
	int fd, new_fd;
	int fd_max = -1;

	int sock_lis_datanode,sock_yama;


	// Creamos e inicializamos los conjuntos que retendran sockets para el select()
	fd_set read_fd, master_fd;
	FD_ZERO(&read_fd);
	FD_ZERO(&master_fd);



	// Creamos sockets para hacer listen() de CPUs
	if ((sock_lis_datanode = makeListenSock(fileSystem->puerto_datanode)) < 0){
		fprintf(stderr, "No se pudo crear socket para escuchar! sock_lis_cpu: %d\n", sock_lis_datanode);
		return FALLO_CONEXION;
	}

	fd_max = MAX(sock_lis_datanode, fd_max);



	// Se agregan memoria, fs, listen_cpu, listen_consola y stdin al set master
	FD_SET(sock_lis_datanode, &master_fd);
	FD_SET(0, &master_fd);

	while ((stat = listen(sock_lis_datanode, BACKLOG)) == -1){
		perror("Fallo listen a socket datanodes. error");
		puts("Reintentamos...\n");
	}


	tHeader *header_tmp = malloc(HEAD_SIZE); // para almacenar cada recv
	while (1){

		read_fd = master_fd;

		ready_fds = select(fd_max + 1, &read_fd, NULL, NULL, NULL);
		if(ready_fds == -1){
			perror("Fallo el select(). error");
			return FALLO_SELECT;
		}

		for (fd = 0; fd <= fd_max; ++fd){
			if (FD_ISSET(fd, &read_fd)){

				printf("Hay un socket listo! El fd es: %d\n", fd);

				// Controlamos el listen de CPU o de Consola
				if (fd == sock_lis_datanode){
					new_fd = handleNewListened(fd, &master_fd);
					if (new_fd < 0){
						perror("Fallo en manejar un listen. error");
						return FALLO_CONEXION;
					}

					fd_max = MAX(new_fd, fd_max);


					break;

				}
				// Como no es un listen, recibimos el header de lo que llego
				if ((stat = recv(fd, header_tmp, HEAD_SIZE, 0)) == -1){
					perror("Error en recv() de algun socket. error");
					break;

				} else if (stat == 0){
					printf("Se desconecto el socket %d\nLo sacamos del set listen...\n", fd);
					//clearAndClose(&fd, &master_fd);
					break;
				}

				// Se recibio un header sin conflictos, procedemos con el flujo..
				if (header_tmp->tipo_de_mensaje == NEW_DN){
					puts("Nuevo Datanode ingresa al sistema");

					break;
				}

				if (fd == sock_yama){
					printf("Llego algo desde YAMA!\n\tTipo de mensaje: %d\n", header_tmp->tipo_de_mensaje);

					/*if (header_tmp->tipo_de_mensaje != )
						break;
					 */


					break;


					if (header_tmp->tipo_de_proceso == DATANODE){
						printf("Llego algo desde DATANODE!\n\tTipo de mensaje: %d\n", header_tmp->tipo_de_mensaje);
						break;
					}



					puts("Si esta linea se imprime, es porque el header_tmp tiene algun valor rarito...");
					printf("El valor de header_tmp es: proceso %d \t mensaje: %d", header_tmp->tipo_de_proceso, header_tmp->tipo_de_mensaje);

				}} // aca terminan el for() y el if(FD_ISSET)
		}


	}
	return 0;
}





tFS *getConfigFilesystem(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tFS *fileSystem = malloc(sizeof(tFS));

	fileSystem->puerto_entrada = malloc(MAX_PORT_LEN);
	fileSystem->puerto_datanode = malloc(MAX_PORT_LEN);
	fileSystem->puerto_yama = malloc(MAX_PORT_LEN);

	t_config *fsConfig = config_create(ruta);


	strcpy(fileSystem->puerto_entrada, config_get_string_value(fsConfig, "PUERTO_FILESYSTEM"));
	strcpy(fileSystem->puerto_datanode, config_get_string_value(fsConfig, "PUERTO_DATANODE"));
	strcpy(fileSystem->puerto_yama, config_get_string_value(fsConfig, "PUERTO_YAMA"));



	//fileSystem->tipo_de_proceso = FILESYSTEM;

	config_destroy(fsConfig);
	return fileSystem;
}
void mostrarConfiguracion(tFS *fileSystem){

	printf("Puerto Entrada: %s\n",  fileSystem->puerto_entrada);
	printf("Puerto Datanode: %s\n",       fileSystem->puerto_datanode);
	printf("Puerto Yama: %s\n", fileSystem->puerto_yama);
	printf("Tipo de proceso: %d\n", fileSystem->tipo_de_proceso);
}


int establecerConexion(char *ip_dest, char *port_dest){

	int stat;
	int sock_dest; // file descriptor para el socket del destino a conectar
	struct addrinfo hints, *destInfo;

	setupHints(&hints, AF_INET, SOCK_STREAM, 0);

	if ((stat = getaddrinfo(ip_dest, port_dest, &hints, &destInfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(stat));
		return FALLO_GRAL;
	}

	if ((sock_dest = socket(destInfo->ai_family, destInfo->ai_socktype, destInfo->ai_protocol)) == -1){
		perror("No se pudo crear socket. error.");
		return FALLO_GRAL;
	}

	if ((stat = connect(sock_dest, destInfo->ai_addr, destInfo->ai_addrlen)) == -1){
		perror("No se pudo establecer conexion, fallo connect(). error");
		printf("Fallo conexion con destino IP: %s PORT: %s\n", ip_dest, port_dest);
		return FALLO_CONEXION;
	}

	freeaddrinfo(destInfo);

	if (sock_dest < 0){
		printf("Error al tratar de conectar con Kernel!\n");
		return FALLO_CONEXION;
	}

	return sock_dest;
}

void setupHints(struct addrinfo *hints, int address_family, int socket_type, int flags){
	memset(hints, 0, sizeof *hints);
	hints->ai_family = address_family;
	hints->ai_socktype = socket_type;
	hints->ai_flags = flags;
}

int makeListenSock(char *port_listen){

	int stat, sock_listen;
	struct addrinfo hints, *serverInfo;

	setupHints(&hints, AF_INET, SOCK_STREAM, AI_PASSIVE);

	if ((stat = getaddrinfo(NULL, port_listen, &hints, &serverInfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(stat));
		return FALLO_GRAL;
	}

	if ((sock_listen = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1){
		perror("No se pudo crear socket. error.");
		return FALLO_GRAL;
	}
	if ((bind(sock_listen, serverInfo->ai_addr, serverInfo->ai_addrlen)) == -1){
		perror("Fallo binding con socket. error");
		printf("Fallo bind() en PORT: %s\n", port_listen);
		return FALLO_CONEXION;
	}

	freeaddrinfo(serverInfo);
	return sock_listen;
}

int makeCommSock(int socket_in){

	int sock_comm;
	struct sockaddr_in clientAddr;
	socklen_t clientSize = sizeof(clientAddr);

	if ((sock_comm = accept(socket_in, (struct sockaddr*) &clientAddr, &clientSize)) == -1){
		perror("Fallo accept del socket entrada. error");
		printf("Fallo accept() en socket_in: %d\n", socket_in);
		return FALLO_CONEXION;
	}

	return sock_comm;
}

int handleNewListened(int sock_listen, fd_set *setFD){

	int stat;
	int new_fd = makeCommSock(sock_listen);
	FD_SET(new_fd, setFD);

	if ((stat = listen(sock_listen, BACKLOG)) == -1){
		perror("Fallo listen al socket. error");
		printf("Fallo listen() en el sock_listen: %d", sock_listen);
		return FALLO_GRAL;
	}

	return new_fd;
}
