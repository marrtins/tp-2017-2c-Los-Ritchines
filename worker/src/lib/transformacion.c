/*
 * transformacion.c
 *
 *  Created on: 2/11/2017
 *      Author: utnso
 */


#include "funcionesWK.h"

extern Tworker *worker;

extern int cont;




int realizarTransformacion(int client_sock){
	TpackDatosTransformacion *datosTransf;
	char * buff;
	char * lineaDeEjecucionTransformacion;
	char * rutaResultadoTransformacion;
	char * nombreScriptTransformador;
	char * rutaScriptTransformador ;
	Theader *headEnvio  = malloc(sizeof (Theader));
	int status;
	pid_t pid;

	puts("llego solicitud para nueva transformacion. recibimos bloque cant bytes y nombre temporal..");
	log_info(logInfo,"llego sol para neuva transf. recibimos daots");
	if ((buff = recvGenericWFlags(client_sock,0)) == NULL){
		puts("Fallo recepcion de datos de la transformacion");
		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_LOCALTRANSFFAIL;
		enviarHeader(client_sock,headEnvio);
		return FALLO_RECV;
	}

	if ((datosTransf = deserializarInfoTransformacionMasterWorker(buff)) == NULL){
		puts("Fallo deserializacion de Bytes de los datos de la transformacion");
		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_LOCALTRANSFFAIL;
		enviarHeader(client_sock,headEnvio);
		return FALLO_GRAL;
	}

	log_info(logInfo,"Se nos pide operar sobre el bloque %d, que ocupa %d bytes y guardarlo en el temporal %s \n",datosTransf->nroBloque,
			datosTransf->bytesOcupadosBloque,datosTransf->nombreTemporal);
	printf("Se nos pide operar sobre el bloque %d, que ocupa %d bytes y guardarlo en el temporal %s \n",datosTransf->nroBloque,
				datosTransf->bytesOcupadosBloque,datosTransf->nombreTemporal);

	//Recibimos el script


	nombreScriptTransformador=string_new();
	rutaScriptTransformador  = string_new();
	string_append(&rutaScriptTransformador,"/home/utnso/");
	string_append(&nombreScriptTransformador,"transformador");
	cont++;
	string_append(&nombreScriptTransformador,string_itoa(cont));
	string_append(&nombreScriptTransformador,worker->nombre_nodo);
	string_append(&nombreScriptTransformador,".sh");
	string_append(&rutaScriptTransformador,nombreScriptTransformador);

	log_info(logInfo,"recibimos el script");
	status = recibirYAlmacenarScript(client_sock,rutaScriptTransformador);
	if(status < 0){
		puts("fallo recibir y almac script");
		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_LOCALTRANSFFAIL;
		enviarHeader(client_sock,headEnvio);
		return FALLO_GRAL;

	}

	char *input1 = getBloqueWorker(datosTransf->nroBloque);
	char * input2=malloc(BLOQUE_SIZE);
	memcpy(input2,input1,datosTransf->bytesOcupadosBloque);



	//log_info(logInfo,"linea  %s \n",input2);

	FILE *bloqueSTD;
	char * rutaBloque = string_new();
	string_append(&rutaBloque,"/home/utnso/tmp/tmpbl-");
	cont++;
	string_append(&rutaBloque,string_itoa(cont));
	string_append(&rutaBloque,"-");
	string_append(&rutaBloque,worker->nombre_nodo);
	bloqueSTD = fopen(rutaBloque, "w");

	fwrite(input2, sizeof(char), datosTransf->bytesOcupadosBloque, bloqueSTD);

	fclose(bloqueSTD);


	if ( (pid=fork()) == 0 )
	{ /* hijo */

		lineaDeEjecucionTransformacion = string_new();
		rutaResultadoTransformacion=string_new();





		string_append(&lineaDeEjecucionTransformacion,"cat ");
		string_append(&lineaDeEjecucionTransformacion,rutaBloque);
		string_append(&lineaDeEjecucionTransformacion," | ./");
		string_append(&lineaDeEjecucionTransformacion,nombreScriptTransformador);
		string_append(&lineaDeEjecucionTransformacion, " | sort  > ");
		string_append(&rutaResultadoTransformacion,"/home/utnso/");
		string_append(&rutaResultadoTransformacion,datosTransf->nombreTemporal);
		string_append(&lineaDeEjecucionTransformacion,rutaResultadoTransformacion);

		log_info(logInfo,"linea de eecucion %s\n",lineaDeEjecucionTransformacion);
		log_info(logInfo,"Ruta resutlado Transformador %s\n",rutaResultadoTransformacion);



		status = system(lineaDeEjecucionTransformacion);
		log_info(logInfo,"Stat lineaDeEjecucion :%d \n",status);
		if(status!=0){
			puts("fallo linea de ejecucion transformacion");
			headEnvio->tipo_de_proceso = WORKER;
			headEnvio->tipo_de_mensaje = FIN_LOCALTRANSFFAIL;
			enviarHeader(client_sock,headEnvio);

		}else{
			headEnvio->tipo_de_proceso = WORKER;
			headEnvio->tipo_de_mensaje = FIN_LOCALTRANSF;


			enviarHeader(client_sock,headEnvio);
			log_info(logInfo,"Envio header. fin transfo ok");
			puts("fin transf ok");
		}
		remove(rutaBloque);
		remove(rutaScriptTransformador);

		exit(0);

	}
	else
	{ /* padre */
		//	printf("Soy el padre (%d, hijo de %d)\n", getpid(),	getppid());
		//	printf("%d\n",cont);
		//waitpid(pid,pidStat,0);
	}
	//printf("Cierro fd %d\n",client_sock);
	//close(client_sock);
	free(input1);
	free(headEnvio);
	return 0;
}


