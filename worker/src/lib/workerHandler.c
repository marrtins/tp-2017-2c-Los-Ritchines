/*
 * workerHandler.c
 *
 *  Created on: 2/11/2017
 *      Author: utnso
 */

#include "funcionesWK.h"





int manejarConexionWorker(Theader *head, int client_sock){

	char * buffer;
	TpackBytes *pathArchivoTemporal;
	char * lineaAux = malloc(MAXSIZELINEA);//todo revisar


	if(head->tipo_de_mensaje==GIVE_TMPREDUCCIONLOCAL){
		puts("Se conecto nodo encargado para hacer el apareo global");
		log_info(logInfo,"Nos llega el path del archivo temporal que precisa");


		if ((buffer = recvGeneric(client_sock)) == NULL){
			puts("Fallo recepcion del path del archivo temporal");
			return FALLO_RECV;
		}

		if ((pathArchivoTemporal =  deserializeBytes(buffer)) == NULL){
			puts("Fallo deserializacion de Bytes del path arch a reducir");
			return FALLO_GRAL;
		}
		free(buffer);

		log_info(logInfo,"Path archivo que vamos a enviarle: %s\n",pathArchivoTemporal->bytes);
		FILE * fdTempFilePropio;
		fdTempFilePropio = fopen((pathArchivoTemporal->bytes),"r");


		int packSize=0;
		Theader headEnvio;

		int stat;
		Theader headRcv = {.tipo_de_proceso = WORKER, .tipo_de_mensaje = 0};
		while ((stat=recv(client_sock, &headRcv, HEAD_SIZE, 0)) > 0) {

			switch (headRcv.tipo_de_mensaje) {

			case(GIVE_NEXTLINE):
				//log_info(logInfo,"give next");
						headEnvio.tipo_de_proceso=WORKER;
						headEnvio.tipo_de_mensaje=TAKE_NEXTLINE;
						if(fgets(lineaAux, 1024*1024,fdTempFilePropio) !=NULL){
							//log_info(logInfo,"Envio: %s\n",lineaAux);
							printf("Envio: %s\n",lineaAux);
							buffer=serializeBytes(headEnvio,lineaAux,strlen(lineaAux)+1,&packSize);
							if ((stat = send(client_sock, buffer, packSize, 0)) == -1){
								puts("no se pudo enviar path del archivo temporal que necesitamos. ");
								return FALLO_SEND ;
							}
							//log_info(logInfo,"41");
							free(buffer);
							//log_info(logInfo,"42");
							//free(lineaAux);
							//log_info(logInfo,"43");
						}else{
							head->tipo_de_mensaje=EOF_TEMPORAL;
							head->tipo_de_proceso=WORKER;
							puts("le mando eof");
							enviarHeader(client_sock,head);
							fclose(fdTempFilePropio);
						}

			break;
			default:
				printf("mensaje no reconocido proceso: %d msj: %d\n",head->tipo_de_proceso, head->tipo_de_mensaje);
				break;
			}
		}
	}
	puts("fin conexion con worker encargado");
	log_info(logInfo,"free linea axu");
	free(lineaAux);
	log_info(logInfo,"pase free linea aux wh");
	free(pathArchivoTemporal->bytes);
	free(pathArchivoTemporal);
	return 0;
}
