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


	if ((bufferReduccion = recvGenericWFlags(client_sock,0)) == NULL){
		puts("Fallo recepcion de datos de la reduccion local");
		return FALLO_RECV;
	}

	if ((infoReduccion = deserializarInfoReduccionLocalMasterWorker(bufferReduccion)) == NULL){
		puts("Fallo deserializacion de Bytes de los datos de la reduccion local");
		return FALLO_GRAL;
	}


	printf("\n\n\n esta es la info q  me llego");
	printf("Nombre temporal de la reduccion: %s\n",infoReduccion->nombreTempReduccion);

	int i;

	printf("LIST SIZE %d\n",infoReduccion->listaSize);
	lineaDeEjecucionApareo=string_new();
	string_append(&lineaDeEjecucionApareo,"sort -m");

	for(i=0;i<infoReduccion->listaSize;i++){
		TreduccionLista *infoAux = list_get(infoReduccion->listaTemporales,i);
		printf("Nombre del archivo %d a reducir: %s\n",i,infoAux->nombreTemporal);
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

	printf("linea de ejec apareo %s \n",lineaDeEjecucionApareo);
	system(lineaDeEjecucionApareo);
	puts("Ahora recibo el script reductor");


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








	puts("Forkeo");

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

		printf("linea de eecucion %s\n",lineaDeEjecucionReduccion);
		printf("Ruta resutlado reduccion %s\n",rutaResultadoReduccion);



		stat = system(lineaDeEjecucionReduccion);
		printf("Stat lineaDeEjecucion :%d \n",stat);

		headEnvio->tipo_de_proceso = WORKER;
		headEnvio->tipo_de_mensaje = FIN_REDUCCIONLOCALOK;

		puts("Envio header. fin reduccion ok");
		enviarHeader(client_sock,headEnvio);
		//close(client_sock);
		exit(0);

	}
	else
	{ /* padre */

	}

	//free(headEnvio);
	return 0;
}
