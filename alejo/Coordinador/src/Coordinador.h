/*
 * Coordinador.h
 *
 *  Created on: 4 abr. 2018
 *      Author: utnso
 */

#define MYPORT 3490    // Puerto al que conectarán los usuarios

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola

typedef struct{

	char* puertoPropio;
	int algoritmo;
	int entradas;
	int tamanio;
	int retardo;
	int tipoDeProceso;
}tCoordinador;





struct parametrosConexion{
	//int sockfd; --> no se requiere para la conexion
	int new_fd;
};


void sigchld_handler(int s);
int main(void);

void *gestionarConexion(struct parametrosConexion *parametros);
void *conexionESI(int *new_fd);
void *conexionPlanificador(int *new_fd);
void *conexionInstancia(int *new_fd);
