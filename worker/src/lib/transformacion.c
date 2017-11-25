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
	char * nombreScriptTransformador;
	char * rutaScriptTransformador ;
	Theader *headEnvio  = malloc(sizeof (Theader));
	int status;
	pid_t pid;

	//puts("llego solicitud para nueva transformacion. recibimos bloque cant bytes y nombre temporal..");
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

	free(buff);

	log_info(logInfo,"Se nos pide operar sobre el bloque %d, que ocupa %d bytes y guardarlo en el temporal %s \n",datosTransf->nroBloque,
			datosTransf->bytesOcupadosBloque,datosTransf->nombreTemporal);
	/*printf("Se nos pide operar sobre el bloque %d, que ocupa %d bytes y guardarlo en el temporal %s \n",datosTransf->nroBloque,
				datosTransf->bytesOcupadosBloque,datosTransf->nombreTemporal);
	*/
	printf("Nueva Transformacion bloque %d\n",datosTransf->nroBloque);
	//Recibimos el script


	nombreScriptTransformador=string_new();
	rutaScriptTransformador  = string_new();
	string_append(&rutaScriptTransformador,"/home/utnso/");
	string_append(&nombreScriptTransformador,"transformador");
	cont++;
	//string_append_with_format(&nombreScriptTransformador,"%d",cont);
	string_append(&nombreScriptTransformador,string_itoa(cont));
	string_append(&nombreScriptTransformador,worker->nombre_nodo);
	string_append(&nombreScriptTransformador,".py");
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

	log_info(logInfo,"asd1");
	free(input1);
	log_info(logInfo,"2");

	//log_info(logInfo,"linea  %s \n",input2);

	FILE *bloqueSTD;
	char * rutaBloque = string_new();
	string_append(&rutaBloque,"/home/utnso/tmp/tmpbl-");
	cont++;
	//string_append_with_format(&rutaBloque,"%d",cont);
	string_append(&rutaBloque,string_itoa(cont));
	string_append(&rutaBloque,"-");
	string_append(&rutaBloque,worker->nombre_nodo);
	bloqueSTD = fopen(rutaBloque, "w");

	fwrite(input2, sizeof(char), datosTransf->bytesOcupadosBloque, bloqueSTD);
	log_info(logInfo,"2.1");
	free(input2);
	log_info(logInfo,"3");
	fclose(bloqueSTD);


	if ( (pid=fork()) == 0 )
	{ /* hijo */
		char * lineaDeEjecucionTransformacion;
		char * rutaResultadoTransformacion;

		lineaDeEjecucionTransformacion = string_new();
		rutaResultadoTransformacion=string_new();
		//int asd=system("export LC_ALL=C");
		//printf("Hice LC_ALL. stat: %d\n",asd);

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

			printf("Fin Transformacion %d OK\n",datosTransf->nroBloque);
			enviarHeader(client_sock,headEnvio);
			log_info(logInfo,"Envio header. fin transfo oky");

		}
		remove(rutaBloque);
		remove(rutaScriptTransformador);
		free(lineaDeEjecucionTransformacion);
		free(rutaResultadoTransformacion);
		/*free(nombreScriptTransformador);
		free(rutaScriptTransformador);*/
		log_info(logInfo,"fin fork t");

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


	//log_info(logInfo,"4");
	free(headEnvio);
	//log_info(logInfo,"5");
	free(datosTransf->nombreTemporal);
	//log_info(logInfo,"6");
	free(datosTransf);
	//log_info(logInfo,"7");
	//free(lineaDeEjecucionTransformacion);
	//log_info(logInfo,"8");
	//free(rutaResultadoTransformacion);
	//log_info(logInfo,"9");
	free(nombreScriptTransformador);
	//log_info(logInfo,"10");
	free(rutaScriptTransformador);
	//log_info(logInfo,"11");
	free(rutaBloque);
	//log_info(logInfo,"12");
	return 0;
}


