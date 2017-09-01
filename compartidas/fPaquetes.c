/*
 * fPaquetes.c
 *
 *  Created on: 1/9/2017
 *      Author: utnso
 */

#include "definiciones.h"
#include "fCompartidas.h"
#include "fPaquetes.h"
int main(){	return 0;}

int recibirInfoProcSimple(int sock, tPackHeader h_esp, int *var){

	int stat;
	char *info_serial;
	tPackHeader head;

	if ((stat = recv(sock, &head, HEAD_SIZE, 0)) == -1){
		perror("Fallo recepcion de info de Proceso. error");
		return FALLO_RECV;
	}

	if (head.tipo_de_proceso != h_esp.tipo_de_proceso || head.tipo_de_mensaje != h_esp.tipo_de_mensaje){
		printf("El paquete recibido no era el esperado! Proceso: %d, Mensaje: %d\n",
				head.tipo_de_proceso, head.tipo_de_mensaje);
		return FALLO_GRAL;
	}

	if ((info_serial = recvGeneric(sock)) == NULL){
		puts("Fallo recepcion generica");
		return FALLO_GRAL;
	}

	memcpy(var, info_serial, sizeof(int));
	free(info_serial);
	return 0;

}
char *recvGenericWFlags(int sock_in, int flags){
	//printf("Se recibe el paquete serializado, usando flags %x\n", flags);

	int stat, pack_size;
	char *p_serial;

	if ((stat = recv(sock_in, &pack_size, sizeof(int), flags)) == -1){
		perror("Fallo de recv. error");
		return NULL;

	} else if (stat == 0){
		printf("El proceso del socket %d se desconecto. No se pudo completar recvGenerico\n", sock_in);
		return NULL;
	}

	pack_size -= (HEAD_SIZE + sizeof(int)); // ya se recibieron estas dos cantidades
	//printf("Paquete de size: %d\n", pack_size);

	if ((p_serial = malloc(pack_size)) == NULL){
		printf("No se pudieron mallocar %d bytes para paquete generico\n", pack_size);
		return NULL;
	}

	if ((stat = recv(sock_in, p_serial, pack_size, flags)) == -1){
		perror("Fallo de recv. error");
		return NULL;

	} else if (stat == 0){
		printf("El proceso del socket %d se desconecto. No se pudo completar recvGenerico\n", sock_in);
		return NULL;
	}

	return p_serial;
}

char *recvGeneric(int sock_in){
	return recvGenericWFlags(sock_in, 0);
}
