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

	Theader *head = malloc(sizeof (Theader));

	TreduccionGlobal *infoReduccionGlobal;
	if ((buffer = recvGenericWFlags(client_sock,MSG_WAITALL)) == NULL){
		puts("Fallo recepcion de INFOBLOQUE");
		head->tipo_de_proceso = WORKER;
		head->tipo_de_mensaje = FIN_REDUCCIONGLOBALFAIL;
		enviarHeader(client_sock,head);
		return FALLO_RECV;
	}

	if ((infoReduccionGlobal = deserializeInfoReduccionGlobal(buffer)) == NULL){
		puts("Fallo deserializacion de Bytes del deserializar info reduccion local");
		head->tipo_de_proceso = WORKER;
		head->tipo_de_mensaje = FIN_REDUCCIONGLOBALFAIL;
		enviarHeader(client_sock,head);
		return FALLO_GRAL;
	}
	free(buffer);


	printf("Nueva RG %s\n",infoReduccionGlobal->tempRedGlobal);
	log_info(logInfo," llego info para la redu global job %d\n id %d\n tempred %s",infoReduccionGlobal->job,infoReduccionGlobal->idTarea,infoReduccionGlobal->tempRedGlobal);

	log_info(logInfo,"list size %d",infoReduccionGlobal->listaNodosSize);

	int i;
	for(i=0;i<list_size(infoReduccionGlobal->listaNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(infoReduccionGlobal->listaNodos,i);
		log_info(logInfo," nombre nodo: %s ",infoNodo->nombreNodo);
		log_info(logInfo," ip nodo: %s ",infoNodo->ipNodo);
		log_info(logInfo," peurto: %s ",infoNodo->puertoNodo);
		log_info(logInfo," temp red loc: %s",infoNodo->temporalReduccion);
		log_info(logInfo," encargado: %d ",infoNodo->nodoEncargado);
	}

	char * nombreScriptReductor;
	char * rutaScriptReductor;


	nombreScriptReductor=string_new();
	rutaScriptReductor  = string_new();
	string_append(&rutaScriptReductor,"/home/utnso/");
	string_append(&nombreScriptReductor,"reductorGlobal");
	cont++;
	string_append(&nombreScriptReductor,string_itoa(cont));
	string_append(&nombreScriptReductor,worker->nombre_nodo);
	string_append(&nombreScriptReductor,".py");
	string_append(&rutaScriptReductor,nombreScriptReductor);


	stat = recibirYAlmacenarScript(client_sock,rutaScriptReductor);
	stat=0;
	if(stat<0){
		puts("error recibir script");
		head->tipo_de_proceso = WORKER;
		head->tipo_de_mensaje = FIN_REDUCCIONGLOBALFAIL;
		enviarHeader(client_sock,head);
		return -1;
	}






	//puts("Forkeo");
	cont++;
	pid_t pidRedGl;
	if ( (pidRedGl=fork()) == 0 )
	{ /* hijo */

			char * lineaDeEjecucionReduccionGlobal;
				char * rutaResultadoReduccionGlobal;
				log_info(logInfo,"Ahora recibo el script reductor");



					log_info(logInfo,"recibi");
					//le pido a todos los workers que me pasen sus reducciones locales.
					puts("Soy el worker encargado. Inicio apareo global...");
					char * rutaApareoFinal=string_new();
					//puts("str new");
					string_append(&rutaApareoFinal,"/home/utnso/tmp/apareoGlobalFinal-");
					string_append(&rutaApareoFinal,worker->nombre_nodo);
					string_append(&rutaApareoFinal,string_itoa(cont));
					puts(rutaApareoFinal);
					log_info(logInfo,"realizar apaglobal");
		//	printf("Soy el hijo (%d, hijo de %d)\n", getpid(),getppid());
		//	printf("%d\n",cont);

		stat = realizarApareoGlobal(infoReduccionGlobal->listaNodos,rutaApareoFinal);
		stat=0;
		if(stat<0){
			puts("error apareo global");
			head->tipo_de_proceso = WORKER;
			head->tipo_de_mensaje = FIN_REDUCCIONGLOBALFAIL;
			enviarHeader(client_sock,head);
		}

		log_info(logInfo,"hago reduccion global");

		printf("Inicio RG %s\n",infoReduccionGlobal->tempRedGlobal);



		lineaDeEjecucionReduccionGlobal = string_new();
		rutaResultadoReduccionGlobal=string_new();

		//int asd=system("export LC_ALL=C");
		//printf("Hice LC_ALL. stat: %d\n",asd);


		string_append(&lineaDeEjecucionReduccionGlobal,"cat ");
		string_append(&lineaDeEjecucionReduccionGlobal,rutaApareoFinal);
		string_append(&lineaDeEjecucionReduccionGlobal," | ./");
		string_append(&lineaDeEjecucionReduccionGlobal,nombreScriptReductor);
		string_append(&lineaDeEjecucionReduccionGlobal, " > /home/utnso/");
		string_append(&rutaResultadoReduccionGlobal,infoReduccionGlobal->tempRedGlobal);
		string_append(&lineaDeEjecucionReduccionGlobal,rutaResultadoReduccionGlobal);

		log_info(logInfo,"linea de eecucion red global %s",lineaDeEjecucionReduccionGlobal);
		log_info(logInfo,"Ruta resutlado reduccion %s",rutaResultadoReduccionGlobal);



		stat = system(lineaDeEjecucionReduccionGlobal);
		log_info(logInfo,"Stat lineaDeEjecucion  RG :%d ",stat);
		//stat=0;
		if(stat != 0){
			puts("error linea de ejecucion reduccion global");
			head->tipo_de_proceso = WORKER;
			head->tipo_de_mensaje = FIN_REDUCCIONGLOBALFAIL;
			enviarHeader(client_sock,head);

		}else{
		printf("FIN RG %s OK\n",infoReduccionGlobal->tempRedGlobal);
		head->tipo_de_proceso = WORKER;
		head->tipo_de_mensaje = FIN_REDUCCIONGLOBALOK;
		enviarHeader(client_sock,head);
		log_info(logInfo,"Envio header. fin reduccion global ok");

		}
		remove(rutaScriptReductor);
		free(lineaDeEjecucionReduccionGlobal);
		free(rutaResultadoReduccionGlobal);
		//close(client_sock);
		//log_info(logInfo,"free onmbre script red");
			free(nombreScriptReductor);
			//log_info(logInfo,"free pase nsr");

			//log_info(logInfo,"free rsr pase");
			//log_info(logInfo,"free rar");
			free(rutaApareoFinal);
			//log_info(logInfo,"free rar pase");

		log_info(logInfo,"fin fork rg");
		exit(0);

	}
	else
	{ /* padre */

		//	printf("Soy el padre (%d, hijo de %d)\n", getpid(),	getppid());
		//	printf("%d\n",cont);
		//waitpid(pid,pidStat,0);
	}

	//int pidStat;
	//waitpid(pidRedGl,&pidStat,WNOHANG);
	//log_info(logInfo,"free head");
	free(head);
	//log_info(logInfo,"free headpase");
	//log_info(logInfo,"free temp red glo");
	free(infoReduccionGlobal->tempRedGlobal);
	//log_info(logInfo,"free trglo pase");
	//log_info(logInfo,"free ldestro");
	list_destroy_and_destroy_elements(infoReduccionGlobal->listaNodos,liberarInfoNodos);
	//log_info(logInfo,"free ldstro pase");
	//log_info(logInfo,"free irg");
	free(infoReduccionGlobal);
	//log_info(logInfo,"free irg pase");
	free(nombreScriptReductor);
	free(rutaScriptReductor);



	return 0;

}







int realizarApareoGlobal(t_list * listaInfoNodos,char * rutaApareoGlobal){
	int i,stat;


	//int asd=system("export LC_ALL=C");
	//printf("Hice LC_ALL. stat: %d\n",asd);
	int packSize2;
	Theader headEnvio;
	headEnvio.tipo_de_proceso=WORKER;
	headEnvio.tipo_de_mensaje=GIVE_TMPREDUCCIONLOCAL;
	char * buffer2;
	int cantArch = list_size(listaInfoNodos);
	int fdWorker;
	char * rutaMiTemporal = string_new();
	string_append(&rutaMiTemporal,"/home/utnso/");
	t_list * listaFds = list_create();
	for(i=0;i<list_size(listaInfoNodos);i++){
		TinfoNodoReduccionGlobal *infoNodo = list_get(listaInfoNodos,i);
		if(infoNodo->nodoEncargado!=1){//no soy yo.(soy el unico encargado(==1)
			fdWorker = conectarAServidor(infoNodo->ipNodo,infoNodo->puertoNodo);
			if(fdWorker <0 ){
				puts("no se pudo enviar path del archivo temporal que necesitamos. worker desconectado. fallo apareo global ");
				return  FALLO_CONEXION;
			}
			buffer2=serializeBytes(headEnvio,infoNodo->temporalReduccion,infoNodo->temporalReduccionLen,&packSize2);
			if ((stat = send(fdWorker, buffer2, packSize2, 0)) == -1){
				puts("no se pudo enviar path del archivo temporal que necesitamos. worker desconectado. fallo apareo global ");
				return  FALLO_SEND;
			}
			TinfoApareoGlobal *infoWorker1 = malloc(sizeof (TinfoApareoGlobal));
			infoWorker1->fdWorker=fdWorker;
			infoWorker1->eofTemporal=false;
			infoWorker1->encargado=false;
			list_add(listaFds,infoWorker1);
			free(buffer2);
		}else{
			string_append(&rutaMiTemporal,infoNodo->temporalReduccion);
			TinfoApareoGlobal *infoWorker2 = malloc(sizeof (TinfoApareoGlobal));
			infoWorker2->fdWorker=-1;
			infoWorker2->eofTemporal=false;
			infoWorker2->encargado=true;
			list_add(listaFds,infoWorker2);

		}
	}

	log_info(logInfo,"pase pedir info. abro archivo");
	FILE * fdTempFilePropio;
	puts(rutaMiTemporal);
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
	//puts("pido la primer linea");
	//int fdWorker;
	//pido la primer linea a cada worker y leo la mia:
	log_info(logInfo,"pido la 1 linea de todos y leo la mia");
	int size=list_size(listaFds);
	for(i=0;i<size;i++){
		TinfoApareoGlobal *infoWorker4 =(TinfoApareoGlobal *) list_get(listaFds,i);
		//puts("pase infow");
		//printf("fdw %d\n ",infoWorker4->fdWorker);
		fdWorker = infoWorker4->fdWorker;
		//printf("fdw %d\n ",fdWorker);
		//puts("pase fdw");
		if(!infoWorker4->encargado){
			//puts("if 1");
			head->tipo_de_proceso=WORKER;
			head->tipo_de_mensaje=GIVE_NEXTLINE;
			stat = enviarHeader(fdWorker,head);
			if(stat < 0){
				puts("worker desconectado. fallo apareo global ");
				return  FALLO_CONEXION;
			}
			//puts("envie hejader");
			if ((stat = recv(fdWorker, head, sizeof(Theader), 0)) < 0){
				puts("worker desconectado. fallo apareo global");
				log_error(logError,"Error en la recepcion del header.");
				return FALLO_RECV;
			}
			//puts("recibo");
			if(head->tipo_de_mensaje==TAKE_NEXTLINE){
				if ((buffer = recvGenericWFlags(fdWorker,MSG_WAITALL)) == NULL){
					puts("Fallo recepcion de nextline. worker desconectado. fallo apareo global");
					return FALLO_RECV;
				}
				//puts("pase buffer");

				if ((siguienteLinea =  deserializeBytes(buffer)) == NULL){
					puts("Fallo apareo global. deser");
					return FALLO_GRAL;
				}
			//	log_info(logInfo,"free buffer");
				free(buffer);
				//	log_info(logInfo,"pase free buff");
				//puts("pase sig linea");
			//	log_info(logInfo,"Linea Recibida: %s\n",siguienteLinea->bytes);
				strcpy(lineas[i],siguienteLinea->bytes);

		//		log_info(logInfo,"free sig linea");
				free(siguienteLinea->bytes);
			//	log_info(logInfo,"pase free sig linea");
				free(siguienteLinea);
				///log_info(logInfo,"pase free sig linea2");
			}
		}else{
			//log_info(logInfo,"hago fscanf");
			//printf("FD:%d\n",fdTempFilePropio);
			//fscanf (fdTempFilePropio, "%s", lineaAux);
			if(fgets(lineaAux, 1024*1024,fdTempFilePropio) !=NULL){
				strcpy(lineas[i],lineaAux);
			}else{
				strcpy(lineas[i],"@@EOF@NULL!@@");
			}



			//log_info(logInfo,"linea:");
			//log_info(logInfo,lineaAux);


			//log_info(logInfo,"free linea aux asd");
			free(lineaAux);
			//log_info(logInfo,"pase free linea aux");

		}
	}
	/*puts("tengo estas lineas x el momento (las primeras4)");
	log_info(logInfo,"tengo estas lineas x el momento(las primeras 4)");
	for(i=0;i<list_size(listaInfoNodos);i++){
		printf("%s\n",lineas[i]);
		log_info(logInfo,"%s\n",lineas[i]);
	}*/

	int mayorIndice;

	FILE * fdArchivoResultado=fopen(rutaApareoGlobal,"w");

	while(cantEofRecibidos<eofTotal){
			mayorIndice = compararLineas(cantArch,lineas);
			lineaAux=lineas[mayorIndice];
			fprintf(fdArchivoResultado,"%s",lineaAux);

			//log_info(logInfo,"de todas las lineas que tengo:");
			//puts(" de todas las q tengo.  ");
			/*for(i=0;i<list_size(listaInfoNodos);i++){
				printf("%s",lineas[i]);
				log_info(logInfo,"%s",lineas[i]);
			}
			log_info(logInfo,"la mayor es %s",lineaAux);
			printf("la mayor es %s",lineaAux);
*/
			//puts("asd");
			//log_info(logInfo,"free linea aux asd2");
			//free(lineaAux);
			//log_info(logInfo,"pase free linea aux2");

			//lineaAux=pedirSiguienteLineaA(mayorIndice);

			//log_info(logInfo,"pido siguiente linea al mayor indice (el punteor q avanzo");
			//puts("pido siguiente linea al mayor indice (el punteor q avanzo");

			TinfoApareoGlobal *infoWorker = list_get(listaFds,mayorIndice);

			if(!infoWorker->encargado){
				headEnvio.tipo_de_proceso=WORKER;
				head->tipo_de_mensaje=GIVE_NEXTLINE;

				enviarHeader(infoWorker->fdWorker,head);
				//printf("pido sig a %d\n",infoWorker->fdWorker);
				//log_info(logInfo,"pido la prox a fd %d",infoWorker->fdWorker);
				if ((stat = recv(infoWorker->fdWorker, head, sizeof(Theader), 0)) < 0){
					puts("fallo conexion c worker");

					log_error(logError,"Error en la recepcion del header.");
					return FALLO_CONEXION;
				}
				//log_info(logInfo,"llego la rta de  fd %d",infoWorker->fdWorker);
				//printf("llego la rta de  fd %d",infoWorker->fdWorker);
				if(head->tipo_de_mensaje==TAKE_NEXTLINE){
					if ((buffer = recvGeneric(infoWorker->fdWorker)) == NULL){
						puts("Fallo recepcion de nextline. fallo apareo global");
						return FALLO_RECV;
					}

					if ((siguienteLinea =  deserializeBytes(buffer)) == NULL){
						puts("Fallo deserializacion de Bytes de nextline. fallo apareo global");
						return FALLO_GRAL;
					}

					//log_info(logInfo,"Linea Recibida: %s\n",siguienteLinea->bytes);
					//printf("Linea Recibida: %s\n",siguienteLinea->bytes);
					strcpy(lineas[mayorIndice],siguienteLinea->bytes);

					//log_info(logInfo,"free2");
					free(buffer);
					free(siguienteLinea->bytes);
					free(siguienteLinea);
					//log_info(logInfo,"pase free2");

				}else if(head->tipo_de_mensaje==EOF_TEMPORAL){
					//log_info(logInfo,"mando eof fd %d",infoWorker->fdWorker);
					//printf("mando eof fd %d",infoWorker->fdWorker);
					strcpy(lineas[mayorIndice],"@@EOF@NULL!@@");//hrdc
					cantEofRecibidos++;
				}
			}else{
				//log_info(logInfo,"soy el mayor. saco mi linea");
				//printf("soy el mayor. saco mi linea");
				if((stat=fgets(lineaAux, 1024*1024,fdTempFilePropio) !=NULL)){
					strcpy(lineas[mayorIndice],lineaAux);
				}else{
					//puts("llegue a eof propoi");
					strcpy(lineas[mayorIndice],"@@EOF@NULL!@@");//hrdc
					cantEofRecibidos++;
				}
				//log_info(logInfo,"saque mi linea");

			}
	}
	puts("Fin apareo global");
	log_info(logInfo,"fin apareo global");
	fclose(fdTempFilePropio);
	fclose(fdArchivoResultado);
	for(i=0;i<list_size(listaFds);i++){
		TinfoApareoGlobal *infoWorker = list_get(listaFds,i);
		if(infoWorker->encargado!=1){
			close(infoWorker->fdWorker);
		}
	}
	free(head);
	//log_info(logInfo,"free lista fds");
	list_destroy_and_destroy_elements(listaFds,liberarInfoApareoGlobal);
	//log_info(logInfo,"pase free lista fds");

	//log_info(logInfo,"free buffer2");
	//free(buffer2);
	//log_info(logInfo,"pase free buff 2");


	/*log_info(logInfo,"free buffer");
	free(buffer);
	log_info(logInfo,"pase free buff");
	*/
	/*log_info(logInfo,"free sig linea");
	free(siguienteLinea);
	log_info(logInfo,"pase free sig linea");
	*/

	//log_info(logInfo,"free rmt");
	free(rutaMiTemporal);
	//log_info(logInfo,"pase rmt");


/*
	log_info(logInfo,"free lineas");
	free(lineas);
	log_info(logInfo,"pase free lineas");

*///todo:chequear free


	return 0;
}
void liberarInfoApareoGlobal(void * info){
	//log_info(logInfo,"free info apa gl");
	TinfoApareoGlobal * infoApareo = (TinfoApareoGlobal*) info;
	free(infoApareo);
	//log_info(logInfo,"pase free info apa gl");
}

void liberarInfoNodos(void * info){
	//log_info(logInfo,"free info nodo list");
	TinfoNodoReduccionGlobal * infoNodo = (TinfoNodoReduccionGlobal*) info;
	free(infoNodo->ipNodo);
	free(infoNodo->nombreNodo);
	free(infoNodo->puertoNodo);
	free(infoNodo->temporalReduccion);
	free(infoNodo);
	//log_info(logInfo,"pase free info nodo list");
}


int compararLineas(int cantArchivos,char arrayLineas[cantArchivos][MAXSIZELINEA]){
	int i,ret;
	int indiceMenor=0;
	char *lineaMenor;
	int contadorEOF=0;
	int k=0;
	lineaMenor=arrayLineas[0];
	while(string_equals_ignore_case(lineaMenor,"@@EOF@NULL!@@")){
		lineaMenor=arrayLineas[k];
		indiceMenor=k;
		k++;
		if(k>cantArchivos+5) return -1;
	}
	/*for(i=0;i<cantArchivos;i++){
		if(!string_equals_ignore_case(arrayLineas[i],"@@EOF@NULL!@@")){
			//lineaMenor=arrayLineas[0];
			while(string_equals_ignore_case(lineaMenor,"@@EOF@NULL!@@")){
				k++;
				lineaMenor=arrayLineas[k];
				indiceMenor=k;
				if(k>cantArchivos+5) return -1;
			}
			i=cantArchivos;
		}else{
			contadorEOF++;
		}
	}*/

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



