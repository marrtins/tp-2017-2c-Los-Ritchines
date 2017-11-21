/*
 * reduccionLocal.c
 *
 *  Created on: 2/11/2017
 *      Author: utnso
 */


#include "funcionesWK.h"

extern Tworker *worker;
extern int cantApareosGlobal;
extern int cont;


int realizarReduccionLocal(int client_sock){
	TinfoReduccionLocalMasterWorker *infoReduccion;
	char * bufferReduccion;
	char * nombreScriptReductor;
	char * rutaScriptReductor;
	char  *rutaResultadoReduccion;
	char  *rutaTemporalesApareados;
	char * lineaDeEjecucionReduccion;
	char * lineaDeEjecucionApareo;

	int stat;
	pid_t pidRed;
	Theader *headEnvio  = malloc(sizeof (Theader));
	puts("Llego solicitud de inicio para reduccion local");
	log_info(logInfo,"llego sol d inic para RL");

	if ((bufferReduccion = recvGenericWFlags(client_sock,0)) == NULL){
		puts("Fallo recepcion de datos de la reduccion local");
		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_REDUCCIONLOCALFAIL;
		enviarHeader(client_sock,headEnvio);
		return FALLO_RECV;
	}

	if ((infoReduccion = deserializarInfoReduccionLocalMasterWorker(bufferReduccion)) == NULL){
		puts("Fallo deserializacion de Bytes de los datos de la reduccion local");
		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_REDUCCIONLOCALFAIL;
		enviarHeader(client_sock,headEnvio);
		return FALLO_GRAL;
	}


	log_info(logInfo,"\n\n\n esta es la info q  me llego");
	log_info(logInfo,"Nombre temporal de la reduccion: %s\n",infoReduccion->nombreTempReduccion);

	int i;

	log_info(logInfo,"LIST SIZE %d\n",infoReduccion->listaSize);
	lineaDeEjecucionApareo=string_new();
	string_append(&lineaDeEjecucionApareo,"sort -m");

	for(i=0;i<infoReduccion->listaSize;i++){
		TreduccionLista *infoAux = list_get(infoReduccion->listaTemporales,i);
		log_info(logInfo,"Nombre del archivo %d a reducir: %s\n",i,infoAux->nombreTemporal);
		string_append(&lineaDeEjecucionApareo," /home/utnso/");
		string_append(&lineaDeEjecucionApareo,infoAux->nombreTemporal);

	}
	rutaTemporalesApareados=string_new();

	string_append(&rutaTemporalesApareados,"/home/utnso/tmp/apareoLocal");
	string_append(&rutaTemporalesApareados,worker->nombre_nodo);
	string_append(&rutaTemporalesApareados,"nro");
	string_append(&rutaTemporalesApareados,string_itoa(cantApareosGlobal++));

	string_append(&lineaDeEjecucionApareo," > ");
	string_append(&lineaDeEjecucionApareo,rutaTemporalesApareados);

	log_info(logInfo,"linea de ejec apareo %s \n",lineaDeEjecucionApareo);
	stat = system(lineaDeEjecucionApareo);
	if(stat != 0){
		puts("fallo apareo local ");
		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_REDUCCIONLOCALFAIL;
		enviarHeader(client_sock,headEnvio);
		log_info(logInfo,"stat apareo local: %d",stat);
		return FALLO_GRAL;
	}
	log_info(logInfo,"Ahora recibo el script reductor");


	nombreScriptReductor=string_new();
	rutaScriptReductor  = string_new();
	string_append(&rutaScriptReductor,"/home/utnso/");
	string_append(&nombreScriptReductor,"reductorLocal");
	cont++;
	string_append(&nombreScriptReductor,string_itoa(cont));
	string_append(&nombreScriptReductor,worker->nombre_nodo);
	string_append(&nombreScriptReductor,".sh");
	string_append(&rutaScriptReductor,nombreScriptReductor);


	stat = recibirYAlmacenarScript(client_sock,rutaScriptReductor);
	if(stat < 0){
		puts("fallo recibir script");
		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_REDUCCIONLOCALFAIL;
		enviarHeader(client_sock,headEnvio);
		return FALLO_GRAL;
	}







	//puts("Forkeo");

	if ( (pidRed=fork()) == 0 )
	{ /* hijo */
		//	printf("Soy el hijo (%d, hijo de %d)\n", getpid(),getppid());
		//	printf("%d\n",cont);


		lineaDeEjecucionReduccion = string_new();
		rutaResultadoReduccion=string_new();




		string_append(&lineaDeEjecucionReduccion,"cat ");
		string_append(&lineaDeEjecucionReduccion,rutaTemporalesApareados);
		string_append(&lineaDeEjecucionReduccion," | ./");
		string_append(&lineaDeEjecucionReduccion,nombreScriptReductor);
		string_append(&lineaDeEjecucionReduccion, " > /home/utnso/");
		string_append(&rutaResultadoReduccion,infoReduccion->nombreTempReduccion);
		string_append(&lineaDeEjecucionReduccion,rutaResultadoReduccion);

		log_info(logInfo,"linea de eecucion %s\n",lineaDeEjecucionReduccion);
		log_info(logInfo,"Ruta resutlado reduccion %s\n",rutaResultadoReduccion);



		stat = system(lineaDeEjecucionReduccion);
		log_info(logInfo,"Stat lineaDeEjecucion :%d \n",stat);
		if(stat !=0){
			puts("fallo linea de ejecucion de la reduccion");
			headEnvio->tipo_de_proceso = WORKER;
			headEnvio->tipo_de_mensaje = FIN_REDUCCIONLOCALFAIL;
			enviarHeader(client_sock,headEnvio);
		}else{
			headEnvio->tipo_de_proceso = WORKER;
			headEnvio->tipo_de_mensaje = FIN_REDUCCIONLOCALOK;
			enviarHeader(client_sock,headEnvio);
			log_info(logInfo,"Envio header. fin reduccion ok");
			puts("fin rl ok");
		}
		remove(rutaScriptReductor);
		//close(client_sock);
		exit(0);

	}
	else
	{ /* padre */

	}

	//free(headEnvio);
	return 0;
}
