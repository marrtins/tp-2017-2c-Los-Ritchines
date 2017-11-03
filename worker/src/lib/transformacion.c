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
	Theader *headEnvio  = malloc(sizeof headEnvio);
	int stat;
	pid_t pid;

	puts("llego solicitud para nueva transformacion. recibimos bloque cant bytes y nombre temporal..");

	if ((buff = recvGenericWFlags(client_sock,0)) == NULL){
		puts("Fallo recepcion de datos de la transformacion");
		return FALLO_RECV;
	}

	if ((datosTransf = deserializarInfoTransformacionMasterWorker(buff)) == NULL){
		puts("Fallo deserializacion de Bytes de los datos de la transformacion");
		return FALLO_GRAL;
	}

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


	stat = recibirYAlmacenarScript(client_sock,rutaScriptTransformador);




	if ( (pid=fork()) == 0 )
	{ /* hijo */

		lineaDeEjecucionTransformacion = string_new();
		rutaResultadoTransformacion=string_new();

		//todo: reemplazar cat wban.csv por el bloque de codigo que nos interesa trasnformar.
		string_append(&lineaDeEjecucionTransformacion,"cat WBAN.csv | ./");
		string_append(&lineaDeEjecucionTransformacion,nombreScriptTransformador);
		string_append(&lineaDeEjecucionTransformacion, " | sort  > ");
		string_append(&rutaResultadoTransformacion,"/home/utnso/");
		string_append(&rutaResultadoTransformacion,datosTransf->nombreTemporal);
		string_append(&lineaDeEjecucionTransformacion,rutaResultadoTransformacion);

		printf("linea de eecucion %s\n",lineaDeEjecucionTransformacion);
		//			printf("Ruta resutlado Transformador %s\n",rutaResultadoTransformacion);



		stat = system(lineaDeEjecucionTransformacion);
		printf("Stat lineaDeEjecucion :%d \n",stat);

		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_LOCALTRANSF;

		puts("Envio header. fin transfo ok");
		enviarHeader(client_sock,headEnvio);

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

	free(headEnvio);
	return 0;
}
