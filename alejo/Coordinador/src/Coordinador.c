/*
 * Coordinador.c
 *
 *  Created on: 4 abr. 2018
 *      Author: utnso
 */

#include "Coordinador.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <commons/config.h>

tCoordinador *coordinador;

#define HEADSIZE 8
#define ESI 2
#define INICIO 3
#define PLANIFICADOR 4
#define INSTANCIA 5
int main(int argc, char* argv[]){
	int estado;

	if(argc!=1){
		printf("Error en la cantidad de parametros\n");
		return EXIT_FAILURE;
	}


	coordinador=obtenerConfiguracionCoordinador("/home/utnso/tp-2018-1c-Sistemas-Operactivos/config_coordinador");
	mostrarConfiguracion(coordinador);


	tHead * head = malloc(sizeof(tHead));

	fd_set readFD, masterFD;
	int socketDeEscuchaESI,sockEscuchaInstancia,sockEscuchaPlani,
	fileDescriptorMax = -1,
	cantModificados = 0,
	nuevoFileDescriptor,
	fileDescriptor;

	FD_ZERO(&masterFD);
	FD_ZERO(&readFD);

	//yama cliente
	//aca
	socketDeEscuchaESI = crearSocketDeEscucha(coordinador->puertoPropio);

	fileDescriptorMax = MAXIMO(socketDeEscuchaESI, fileDescriptorMax);

	puts("antes del whle")

	while (listen(socketDeEscuchaESI, BACKLOG) == -1){

		puts("Reintentamos...");
	}


	FD_SET(socketDeEscuchaESI, &masterFD);

	pritnf("el fdmax es %d\n",fileDescriptorMax);
	//aca
	while(1){

		readFD = masterFD;

		if((cantModificados = select(fileDescriptorMax + 1, &readFD, NULL, NULL, NULL)) == -1){
			puts("fallo el select");

		}

		for(fileDescriptor = 3; fileDescriptor <= fileDescriptorMax; fileDescriptor++){

			if(FD_ISSET(fileDescriptor, &readFD)){

				printf("hay un fd listo id %d\n",fileDescriptor);

				if(fileDescriptor == socketDeEscuchaESI){
					nuevoFileDescriptor = conectarNuevoCliente(fileDescriptor, &masterFD);
					printf("Nuevo cliente conectado: %d\n", nuevoFileDescriptor);
					fileDescriptorMax = MAXIMO(nuevoFileDescriptor, fileDescriptorMax);

					break;
				}
				puts("recibiendo");


				if ((estado = recv(fileDescriptor, head, HEADSIZE, 0)) == -1){
					puts("error en recv");

					break;

				} else if (estado == 0){
					printf("Se desconecto el socket %d\nLo sacamos del set listen...\n", fileDescriptor);
					clearAndClose(fileDescriptor, &masterFD);
					break;
				}

				if(head->tipoDeProceso==ESI){
					switch(head->tipoDeMensaje){
					case INICIO:
						puts("se conecto esi. mando msj de inicio");
						//crear hilo p gestionar esi
						break;

//					case PATH_RES_FILE:
//
//						iniciarNuevoJob(fileDescriptor,sockFS);
//						break;
//					case FINTRANSFORMACIONOK:
//						manejarFinTransformacionOK(fileDescriptor);
//						break;
//
//					case FINTRANSFORMACIONFAIL:
//						manejarFinTransformacionFail(fileDescriptor);
//						break;
//
//					case FINTRANSFORMACIONFAILDESCONEXION:
//						manejarFinTransformacionFailDesconexion(fileDescriptor);
//						break;
//					case FINTRANSFORMACIONFAILDESCONEXIONBIS:
//						manejarFinTransformacionFailDesconexionBIS(fileDescriptor);
//						break;
//
//					case FIN_REDUCCIONLOCALOK:
//						manejarFinReduccionLocalOK(fileDescriptor);
//						break;
//
//					case FIN_REDUCCIONLOCALFAIL:
//						manejarFinReduccionLocalFail(fileDescriptor);
//						break;
//
//					case FIN_REDUCCIONGLOBALOK:
//						manejarFinReduccionGlobalOK(fileDescriptor);
//						break;
//
//					case FIN_REDUCCIONGLOBALFAIL:
//						manejarFinReduccionGlobalFail(fileDescriptor);
//						break;
//					case FIN_ALMACENAMIENTOFINALOK:
//						manejarFinAlmacenamientoOK(fileDescriptor);
//						break;
//					case FIN_ALMACENAMIENTOFINALFAIL:
//						manejarFinAlmacenamientoFail(fileDescriptor);
//						break;

					default:
						puts("Tipo de Mensaje no encontrado en el protocolo");

						break;
					}


					break;

				}
				else if(head->tipoDeProceso == PLANIFICADOR){
					switch(head->tipoDeMensaje){
					case INICIO:
						puts("se conecto planificador. mando msj de inicio");

						break;
					default:
						//log_trace(logError, "Tipo de mensaje no encontrado en el protocolo.");

						break;
					}
				}
				else if(head->tipoDeProceso == INSTANCIA){
					switch(head->tipoDeMensaje){
					case INICIO:
						puts("se conecto instancia. mando msj de inicio");

						break;
					default:
						//log_trace(logError, "Tipo de mensaje no encontrado en el protocolo.");
						break;
					}
				}
				else{
					printf("se quiso conectar el proceso: %d\n",head->tipoDeProceso);
					puts("Hacker detected");

					clearAndClose(fileDescriptor, &masterFD);
				}

			}




		}

	//log_error(logError, "Fallo el accept de master.");



	}

free(head);
return 0;
}


tCoordinador *obtenerConfiguracionCoordinador(char* ruta){

	#define MAXIMA_LONGITUD_PUERTO 10
#define COORD 1

	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tCoordinador *coordinador = malloc(sizeof(tCoordinador));


	coordinador->puertoPropio = malloc(MAXIMA_LONGITUD_PUERTO);


	t_config *coordConfig = config_create(ruta);


	strcpy(coordinador->puertoPropio, config_get_string_value(coordConfig, "PUERTOPROPIO"));


	coordinador->algoritmo = config_get_int_value(coordConfig, "ALGORITMO");
	coordinador->entradas =          config_get_int_value(coordConfig, "ENTRADAS");
	coordinador->tamanio =          config_get_int_value(coordConfig, "TAMANIO");
	coordinador->retardo =          config_get_int_value(coordConfig, "RETARDO");
	coordinador->tipoDeProceso = COORD;




	config_destroy(coordConfig);
	return coordinador;
}

void mostrarConfiguracion(tCoordinador *coordinador){

	printf("Puerto Entrada: %s\n",  coordinador->puertoPropio);
//	printf("Algoritmo: %d\n",   coordinador->retardo_planificacion);
//	printf("Algoritmo Balanceo: %s\n",getAlgoritmoBalanceo(yama->algoritmo_balanceo));
//	printf("Retardo: %d\n",yama->retardo_planificacion);
//	printf("Disponibilidad: %d\n",yama->disponibilidadBase);
//	printf("Tipo de proceso: %d\n", yama->tipo_de_proceso);
}
