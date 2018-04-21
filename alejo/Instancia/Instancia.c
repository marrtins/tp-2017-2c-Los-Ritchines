	#include "Instancia.h"
	#include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <sys/socket.h>


    int main(int argc, char *argv[])
    {


    	//configuracion etc etc

    	int socketCoordinador;
    	int estado;
    	socketCoordinador = conectarAServidor(instancia->ip_coord,instancia->puertoCoord);

    	tHeader * head = malloc(sizeof(tHeader));
    	head->tipoDeProceso=INSTANCIA;
    	head->tipoDeMensaje=INICIO;
    	estado = enviarHeader(socketCoordinador,head)
		if(estado<0){
			puts(";error");
		}

    	while (1) {

    			if ((estado = recv(socketCoordinador, head, sizeof(Theader), 0)) == -1) {
    				log_error(logError,"Error al recibir informacion");
    				break;

    			} else if (estado == 0) {
    				sprintf(mensaje, "Se desconecto el socket de fd: %d\n", socketFS);
    				log_error(logError, mensaje);
    				break;
    			}
    			printf("Recibi el head %d bytes\n", estado);

    			if (head->tipo_de_proceso == COORDINADOR) {
    				switch (head->tipo_de_mensaje) {
    					case ALMACENAR_BLOQUE:
    						log_info(logInfo,"Es FileSystem y quiere almacenar un bloque");

    						bloque = recvBloque(socketFS);
    						log_info(logInfo,"Voy a almacenar el bloque.");

    						setBloque(bloque->nroBloque, bloque);

    						log_info(logInfo,"Bloque almacenado");

    						free(bloque->contenido);
    						free(bloque);

    						break;
    					case OBTENER_BLOQUE:
    						log_info(logInfo,"Es FileSystem y quiere obtener el bloque");
    						int nroBloque_;
    						unsigned long long int tamanioBloque_;
    						if (recv(socketFS, &nroBloque_, sizeof(int), 0) == -1) {
    							log_info(logInfo,"Error al recibir el numero do bloque");
    						}
    						if (recv(socketFS, &tamanioBloque_, sizeof(unsigned long long int),0) == -1){
    							log_info(logInfo,"Error al recibir el tamanio del bloque");
    						}
    						enviarBloque(nroBloque_ , tamanioBloque_ ,socketFS);
    						log_info(logInfo,"Se envio el bloque %d a FILESYSTEM",nroBloque_);
    						break;
    					default:
    						log_error(logError,"Se recibio un mensaje que no esta incluido en el protocolo");
    						break;
    					}
    				}
    		}


    }

    }

    int enviarHeader(int socketDestino,tHeader * head){

    	int estado;

    	if ((estado = send(socketDestino, head, sizeof(Theader), 0)) == -1){
    		logErrorAndExit("Fallo al enviar el header");
    	}

    	return estado;
    }
