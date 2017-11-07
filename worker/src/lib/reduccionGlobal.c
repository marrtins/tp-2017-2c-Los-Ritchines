/*
 * reduccionGlobal.c
 *
 *  Created on: 2/11/2017
 *      Author: utnso
 */


#include "funcionesWK.h"

extern Tworker *worker;

extern int cont;
int realizarReduccionGlobal(client_sock){
	int stat;
	char * buffer;
	char * nombreScriptReductor;
	char * rutaScriptReductor;

	TreduccionGlobal *infoReduccionGlobal;
	if ((buffer = recvGenericWFlags(client_sock,MSG_WAITALL)) == NULL){
		puts("Fallo recepcion de INFOBLOQUE");
		return FALLO_RECV;
	}

	if ((infoReduccionGlobal = deserializeInfoReduccionGlobal(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del deserializar info reduccion local");
		return FALLO_GRAL;
	}
	printf("llego la info apra la reduccion global\n");
	printf("job %d\n id %d\n tempred %s\n",infoReduccionGlobal->job,infoReduccionGlobal->idTarea,infoReduccionGlobal->tempRedGlobal);

	printf("list size %d\n",infoReduccionGlobal->listaNodosSize);

	int i;
	for(i=0;i<list_size(infoReduccionGlobal->listaNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(infoReduccionGlobal->listaNodos,i);
		printf(" nombre nodo: %s \n",infoNodo->nombreNodo);
		printf(" ip nodo: %s \n",infoNodo->ipNodo);
		printf(" peurto: %s \n",infoNodo->puertoNodo);
		printf(" temp red loc: %s \n",infoNodo->temporalReduccion);
		printf(" encargado: %d \n",infoNodo->nodoEncargado);
	}


	puts("Ahora recibo el script reductor");


	nombreScriptReductor=string_new();
	rutaScriptReductor  = string_new();
	string_append(&rutaScriptReductor,"/home/utnso/");
	string_append(&nombreScriptReductor,"reductorGlobal");
	cont++;
	string_append(&nombreScriptReductor,string_itoa(cont));
	string_append(&nombreScriptReductor,worker->nombre_nodo);
	string_append(&nombreScriptReductor,".sh");
	string_append(&rutaScriptReductor,nombreScriptReductor);


	stat = recibirYAlmacenarScript(client_sock,rutaScriptReductor);
	if(stat<0){
		puts("error");
	}

	//le pido a todos los workers que me pasen sus reducciones locales.

	char * rutaApareoFinal=string_new();
	string_append(&rutaApareoFinal,"/home/utnso/apareoGlobalFinal-");
	cont++;
	string_append(&rutaApareoFinal,string_itoa(cont));
	stat = realizarApareoGlobal(infoReduccionGlobal->listaNodos,rutaApareoFinal);
	if(stat<0){
			puts("error");
		}

	puts("hago reduccion global\n");







	puts("Forkeo");
	char * lineaDeEjecucionReduccionGlobal;
	char * rutaResultadoReduccionGlobal;
	pid_t pidRedGl;
	if ( (pidRedGl=fork()) == 0 )
	{ /* hijo */
		//	printf("Soy el hijo (%d, hijo de %d)\n", getpid(),getppid());
		//	printf("%d\n",cont);


		lineaDeEjecucionReduccionGlobal = string_new();
		rutaResultadoReduccionGlobal=string_new();




		string_append(&lineaDeEjecucionReduccionGlobal,"cat ");
		string_append(&lineaDeEjecucionReduccionGlobal,rutaApareoFinal);
		string_append(&lineaDeEjecucionReduccionGlobal," | ./");
		string_append(&lineaDeEjecucionReduccionGlobal,nombreScriptReductor);
		string_append(&lineaDeEjecucionReduccionGlobal, " > /home/utnso");
		string_append(&rutaResultadoReduccionGlobal,infoReduccionGlobal->tempRedGlobal);
		string_append(&lineaDeEjecucionReduccionGlobal,rutaResultadoReduccionGlobal);

		printf("linea de eecucion red global %s\n",lineaDeEjecucionReduccionGlobal);
		printf("Ruta resutlado reduccion %s\n",rutaResultadoReduccionGlobal);



		stat = system(lineaDeEjecucionReduccionGlobal);
		printf("Stat lineaDeEjecucion :%d \n",stat);
		Theader *head = malloc(sizeof (Theader));
		head->tipo_de_proceso = WORKER;
		head->tipo_de_mensaje = FIN_REDUCCIONGLOBALOK;

		puts("Envio header. fin reduccion global ok");
		enviarHeader(client_sock,head);
		//close(client_sock);
		exit(0);

	}
	else
	{ /* padre */
		//	printf("Soy el padre (%d, hijo de %d)\n", getpid(),	getppid());
		//	printf("%d\n",cont);
		//waitpid(pid,pidStat,0);
	}
	return 0;

}







int realizarApareoGlobal(t_list * listaInfoNodos,char * rutaApareoGlobal){
	int i,stat;

	int packSize2;
	Theader headEnvio;
	headEnvio.tipo_de_proceso=WORKER;
	headEnvio.tipo_de_mensaje=GIVE_TMPREDUCCIONLOCAL;
	char * buffer2;
	int cantArch = list_size(listaInfoNodos);

	char * rutaMiTemporal = string_new();
	string_append(&rutaMiTemporal,"/home/utnso/");
	t_list * listaFds = list_create();
	for(i=0;i<list_size(listaInfoNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(listaInfoNodos,i);
		if(infoNodo->nodoEncargado!=1){//no soy yo.(soy el unico encargado(==1)
			int fdWorker = conectarAServidor(infoNodo->ipNodo,infoNodo->puertoNodo);
			buffer2=serializeBytes(headEnvio,infoNodo->temporalReduccion,infoNodo->temporalReduccionLen,&packSize2);
			if ((stat = send(fdWorker, buffer2, packSize2, 0)) == -1){
				puts("no se pudo enviar path del archivo temporal que necesitamos. ");
				return  FALLO_SEND;
			}
			TinfoApareoGlobal *infoWorker = malloc(sizeof (TinfoApareoGlobal));
			infoWorker->fdWorker=fdWorker;
			infoWorker->eofTemporal=false;
			infoWorker->encargado=false;
			list_add(listaFds,infoWorker);
		}else{


			string_append(&rutaMiTemporal,infoNodo->temporalReduccion);
			TinfoApareoGlobal *infoWorker = malloc(sizeof (TinfoApareoGlobal));
			infoWorker->fdWorker=-1;
			infoWorker->eofTemporal=false;
			infoWorker->encargado=true;
			list_add(listaFds,infoWorker);

		}
	}

	FILE * fdTempFilePropio;
	fdTempFilePropio = fopen(rutaMiTemporal,"r");

	int eofTotal = list_size(listaFds);
	int cantEofRecibidos =0;

	Theader *head = malloc(sizeof (Theader));
	head->tipo_de_proceso=WORKER;
	head->tipo_de_mensaje=GIVE_NEXTLINE;
	TpackBytes *siguienteLinea;

	char * lineaAux=malloc(MAXSIZELINEA);

	char lineas[cantArch][MAXSIZELINEA];
	char * buffer ;

	//pido la primer linea a cada worker y leo la mia:
	for(i=0;i<list_size(listaFds);i++){
		TinfoApareoGlobal *infoWorker = list_get(listaFds,i);
		int fdWorker = infoWorker->fdWorker;
		if(!infoWorker->encargado){

			head->tipo_de_mensaje=GIVE_NEXTLINE;
			enviarHeader(fdWorker,head);

			if ((stat = recv(fdWorker, head, sizeof(Theader), 0)) < 0){
				log_trace(logger,"Error en la recepcion del header.");
			}
			if(head->tipo_de_mensaje==TAKE_NEXTLINE){
				if ((buffer = recvGeneric(fdWorker)) == NULL){
					puts("Fallo recepcion de nextline");
					return FALLO_RECV;
				}

				if ((siguienteLinea =  deserializeBytes(buffer)) == NULL){
					puts("Fallo deserializacion de Bytes de nextline");
					return FALLO_GRAL;
				}

				printf("Linea Recibida: %s\n",siguienteLinea->bytes);
				strcpy(lineas[i],siguienteLinea->bytes);
			}
		}else{
			fscanf (fdTempFilePropio, "%s", lineaAux);
			strcpy(lineas[i],lineaAux);
		}
	}
	puts("tengo estas lineas x el momento");
	for(i=0;i<list_size(listaInfoNodos);i++){
		printf("%s\n",lineas[i]);
	}

	int mayorIndice;

	FILE * fdArchivoResultado=fopen(rutaApareoGlobal,"w");

	while(cantEofRecibidos<eofTotal){
			mayorIndice = compararLineas(cantArch,lineas);
			lineaAux=lineas[mayorIndice];
			fprintf(fdArchivoResultado,"%s\n",lineaAux);
			//lineaAux=pedirSiguienteLineaA(mayorIndice);

			//pido siguiente linea al mayor indice (el punteor q avanzo)

			TinfoApareoGlobal *infoWorker = list_get(listaFds,mayorIndice);

			if(!infoWorker->encargado){
				head->tipo_de_proceso=YAMA;
				head->tipo_de_mensaje=GIVE_NEXTLINE;

				enviarHeader(infoWorker->fdWorker,head);

				if ((stat = recv(infoWorker->fdWorker, head, sizeof(Theader), 0)) < 0){
					log_trace(logger,"Error en la recepcion del header.");
				}
				if(head->tipo_de_mensaje==TAKE_NEXTLINE){
					if ((buffer = recvGeneric(infoWorker->fdWorker)) == NULL){
						puts("Fallo recepcion de nextline");
						return FALLO_RECV;
					}

					if ((siguienteLinea =  deserializeBytes(buffer)) == NULL){
						puts("Fallo deserializacion de Bytes de nextline");
						return FALLO_GRAL;
					}

				//	printf("Linea Recibida: %s\n",siguienteLinea->bytes);
					strcpy(lineas[mayorIndice],siguienteLinea->bytes);


				}else if(head->tipo_de_mensaje==EOF_TEMPORAL){
					strcpy(lineas[mayorIndice],"@@EOF@NULL!@@");//hrdc
					cantEofRecibidos++;
				}
			}else{
				if((stat=fscanf (fdTempFilePropio, "%s", lineaAux))!=-1){
					strcpy(lineas[mayorIndice],lineaAux);
				}else{
					strcpy(lineas[mayorIndice],"@@EOF@NULL!@@");//hrdc
					cantEofRecibidos++;
				}

			}
	}
	puts("fin apareo global");
	fclose(fdTempFilePropio);
	fclose(fdArchivoResultado);
	for(i=0;i<list_size(listaFds);i++){
		TinfoApareoGlobal *infoWorker = list_get(listaFds,i);
		if(infoWorker->encargado!=1){
			close(infoWorker->fdWorker);
		}
	}
	//free(head);
	//free(lineaAux);
	return 0;
}


int compararLineas(int cantArchivos,char arrayLineas[cantArchivos][MAXSIZELINEA]){
	int i,ret;
	int indiceMenor=0;
	char *lineaMenor;
	int contadorEOF=0;

	for(i=0;i<cantArchivos;i++){
		if(!string_equals_ignore_case(arrayLineas[i],"@@EOF@NULL!@@")){
			lineaMenor=arrayLineas[0];
			i=cantArchivos;
		}else{
			contadorEOF++;
		}
	}

	if(contadorEOF==cantArchivos){
		return -1;
	}


	for(i=0;i<cantArchivos;i++){
		char *lineaAux = arrayLineas[i];
		if(!string_equals_ignore_case(lineaAux,"@@EOF@NULL!@@")){
			//printf("linea mayor: %s \n linea aux: %s\n",lineaMayor,lineaAux);
			ret = strcmp(lineaMenor,lineaAux);

			if(ret > 0) {
				//printf("lineaMenor is less than lineaActual\n");
				lineaMenor=arrayLineas[i];
				indiceMenor=i;
			} else {
				//printf("lineaActual <= lineaMayor\n");
			}
		}
	}
	//printf("linea menor: %s \n",lineaMenor);
	//free(lineaMayor);
	return indiceMenor;

}



