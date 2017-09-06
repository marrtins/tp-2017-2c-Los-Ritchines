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
#include <commons/collections/list.h>
#include <commons/config.h>
#include <string.h>
#include <stdbool.h>
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
#include <commons/string.h>
//#include <readline/readline.h>

#include <netinet/in.h>

#include "../compartidas/definiciones.h"


#define MAX_LINEA 150
#define BACKLOG 20
tFS *fileSystem;
t_list *listaNodos;
int sock_yama;
int fd_max;
fd_set read_fd, master_fd;

int contadorHardCode;

int main(int argc, char* argv[]){

	if(argc!=2){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}

	contadorHardCode=0;
	list_create(listaNodos);

	fileSystem=getConfigFilesystem(argv[1]);
	mostrarConfiguracion(fileSystem);

	pthread_t consola_thread;

	if(pthread_create(&consola_thread, NULL, (void*) consolaFS, NULL) < 0){
			perror("no pudo crear hilo. error");
			return FALLO_GRAL;
		}


	int stat, ready_fds;
	int fd, new_fd;
	fd_max = -1;

	int sock_lis_datanode;


	// Creamos e inicializamos los conjuntos que retendran sockets para el select()

	FD_ZERO(&read_fd);
	FD_ZERO(&master_fd);



	// Creamos sockets para hacer listen() de datanodes
	if ((sock_lis_datanode = makeListenSock(fileSystem->puerto_datanode)) < 0){
		fprintf(stderr, "No se pudo crear socket para escuchar! sock_lis_cpu: %d\n", sock_lis_datanode);
		return FALLO_CONEXION;
	}

	fd_max = MAX(sock_lis_datanode, fd_max);


	// Se agrega list_datanode al master

	FD_SET(sock_lis_datanode, &master_fd);

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

				// Controlamos el listen de datanode
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
					clearAndClose(&fd, &master_fd);
					break;
				}

				// Se recibio un header sin conflictos, procedemos con el flujo..

				if (header_tmp->tipo_de_proceso == DATANODE){
					printf("Llego algo desde DATANODE!\n");

					if((stat=datanodeHandler(header_tmp->tipo_de_mensaje))<0){
						printf("Llego un mensaje no manejado desde datanode\n");
					}

					break;
				}
				if (fd == sock_yama){
					printf("Llego algo desde YAMA!\n");

					if((stat=yamaHandler(header_tmp->tipo_de_mensaje))<0){
						printf("Llego un mensaje no manejado desde yama\n");
					}

					break;


				}
				puts("Si esta linea se imprime, es porque el header_tmp tiene algun valor rarito...");
				printf("El valor de header_tmp es: proceso %d \t mensaje: %d", header_tmp->tipo_de_proceso, header_tmp->tipo_de_mensaje);

			}
		}


	}
	return 0;
}


void consolaFS(void){
	puts("funcion hilo");
			char * linea = malloc(MAX_LINEA);
			char ** palabras;
			while(1){
				linea = fgets(linea,MAX_LINEA,stdin);
				printf("pase\n");
				palabras = string_split(linea, " ");
				if(string_equals_ignore_case(*palabras,"format")){
					printf("ya pude formatear el fs\n");
				}
				else if(string_equals_ignore_case(*palabras,"rm")){
					printf("ya pude remover el archivo\n");
				}
				else if(string_equals_ignore_case(*palabras,"rename")){
					printf("ya pude renombrar el archivo\n");
				}
				else if(string_equals_ignore_case(*palabras,"mv")){
					printf("ya pude mover el archivo\n");
				}
				else if(string_equals_ignore_case(*palabras,"cat")){
					printf("ya pude leer el archivo\n");
				}
				else if(string_equals_ignore_case(*palabras,"mkdir")){
					printf("ya pude crear el directorio\n");
				}
				else if(string_equals_ignore_case(*palabras,"cpfrom")){
					printf("ya pude copiar el archivo local al file system siguiendo lineamientos\n");
				}
				else if(string_equals_ignore_case(*palabras,"cpto")){
					printf("ya pude copiar un archivo local al file system\n");
				}
				else if(string_equals_ignore_case(*palabras,"cpblock")){
					printf("ya pude crear una copia de un bloque del archivo en un nodo\n");
				}
				else if(string_equals_ignore_case(*palabras,"md5")){
					printf("ya pude solicitar el md5 de un archivo del file system\n");
				}
				else if(string_equals_ignore_case(*palabras,"ls")){
					printf("ya pude listar los archivos del directorio\n");
				}
				else if(string_equals_ignore_case(*palabras,"info")){
					printf("ya pude mostrar la informacion del archivo\n");
				}
				else{
					printf("No existe el comando\n");
				}
				 //esto libera la primera posicion del array
				// pero hay que liberar todas ok?
				free(*palabras);

			}
			free(linea);
}
int datanodeHandler(tMensaje msjRecibido){


	printf("Mensaje Recibido desde Datanode: %d \n",msjRecibido);

	switch(msjRecibido){

	case(NEW_DN):
		contadorHardCode++;

		puts("Nuevo Datanode ingresa al sistema");

		if(sistemaEstable()){
			recibirConexionYAMA();
			contadorHardCode=-200;
		}

		break;

	default:
		break;

	}

	return 0;
}
int yamaHandler(tMensaje msjRecibido){


	printf("Mensaje Recibido desde YAMA: %d \n",msjRecibido);

	contadorHardCode=-100;

	switch(msjRecibido){

	case(INICIOYAMA):

		puts("Conexion con YAMA exitosa");

		break;

	default:
		break;

	}

	return 0;
}






//Aca se va a hacer la verificacion entre la lista de nodos guardados q tiene el FS y los que se van conectando.
//Cuando se conecte el ultimo y haga estable el sistema, devolvera true.
bool sistemaEstable(){

	/*if...
	 *
	 */
	//hardcodeo por ahora
	return contadorHardCode>1;
}



int recibirConexionYAMA(void){

	int sock_lis_yama,new_fd;
	tHeader head, h_esp;
	if ((sock_lis_yama = makeListenSock(fileSystem->puerto_entrada)) < 0){
		printf("No se pudo crear socket listen en puerto: %s\n", fileSystem->puerto_entrada);

		return FALLO_GRAL;
	}

	if(listen(sock_lis_yama, BACKLOG) == -1){

		perror("Fallo de listen sobre socket filesystem. error");
		return FALLO_GRAL;
	}

	while (1){
		if((sock_yama = makeCommSock(sock_lis_yama)) < 0){

			puts("No se pudo acceptar conexion entrante de YAMA");
			return FALLO_GRAL;
		}
		printf("Se establecio conexion con YAMA. Socket %d\n", sock_yama);
		break;
	}

	fd_max = MAX(sock_yama, fd_max);
	FD_SET(sock_yama, &master_fd);

	return 0;
}

tFS *getConfigFilesystem(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tFS *fileSystem = malloc(sizeof(tFS));

	fileSystem->puerto_entrada = malloc(MAX_PORT_LEN);
	fileSystem->puerto_datanode = malloc(MAX_PORT_LEN);
	fileSystem->puerto_yama = malloc(MAX_PORT_LEN);
	fileSystem->ip_yama = malloc(MAX_IP_LEN);


	t_config *fsConfig = config_create(ruta);


	strcpy(fileSystem->puerto_entrada, config_get_string_value(fsConfig, "PUERTO_FILESYSTEM"));
	strcpy(fileSystem->puerto_datanode, config_get_string_value(fsConfig, "PUERTO_DATANODE"));
	strcpy(fileSystem->puerto_yama, config_get_string_value(fsConfig, "PUERTO_YAMA"));
	strcpy(fileSystem->ip_yama, config_get_string_value(fsConfig, "IP_YAMA"));


	fileSystem->tipo_de_proceso = FILESYSTEM;

	config_destroy(fsConfig);
	return fileSystem;
}
void mostrarConfiguracion(tFS *fileSystem){

	printf("Puerto Entrada: %s\n",  fileSystem->puerto_entrada);
	printf("Puerto Datanode: %s\n",       fileSystem->puerto_datanode);
	printf("Puerto Yama: %s\n", fileSystem->puerto_yama);
	printf("IP Yama: %s\n", fileSystem->ip_yama);
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

void clearAndClose(int *fd, fd_set *setFD){
	FD_CLR(*fd, setFD);
	close(*fd);
}

int validarRespuesta(int sock, tHeader h_esp, tHeader *h_obt){

	if ((recv(sock, h_obt, HEAD_SIZE, 0)) == -1){
		perror("Fallo recv de Header. error");
		return FALLO_RECV;
	}

	if (h_esp.tipo_de_proceso != h_obt->tipo_de_proceso){
		printf("Fallo de comunicacion. Se espera un mensaje de %d, se recibio de %d\n",
				h_esp.tipo_de_proceso, h_obt->tipo_de_proceso);
		return FALLO_GRAL;
	}

	if (h_esp.tipo_de_mensaje != h_obt->tipo_de_mensaje){
		printf("Fallo ejecucion de funcion con valor %d\n", h_obt->tipo_de_mensaje);
		return FALLO_GRAL;
	}

	return 0;
}


int handshakeCon(int sock_dest, int id_sender){

	int stat;
	char *package;
	tHeader head;
	head.tipo_de_proceso = id_sender;
	head.tipo_de_mensaje = INICIOFS;

	if ((package = malloc(HEAD_SIZE)) == NULL){
		fprintf(stderr, "No se pudo hacer malloc\n");
		return FALLO_GRAL;
	}
	memcpy(package, &head, HEAD_SIZE);

	if ((stat = send(sock_dest, package, HEAD_SIZE, 0)) == -1){
		perror("Fallo send de handshake. error");
		printf("Fallo send() al socket: %d\n", sock_dest);
		return FALLO_SEND;
	}

	free(package);
	return stat;
}
