
#include "funcionesYM.h"

extern int socketFS;

int idTempName,idPropio;


extern int idMasterGlobal;
extern t_list * listaHistoricaTareas;
extern pthread_mutex_t mux_listaHistorica;



pthread_mutex_t mux_idGlobal;




Tyama *obtenerConfiguracionYama(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	Tyama *yama = malloc(sizeof(Tyama));

	yama->ip_filesystem       =    malloc(MAXIMA_LONGITUD_IP);
	yama->puerto_entrada = malloc(MAXIMA_LONGITUD_PUERTO);
	yama->puerto_master = malloc(MAXIMA_LONGITUD_PUERTO);
	yama->puerto_filesystem = malloc(MAXIMA_LONGITUD_PUERTO);

	t_config *yamaConfig = config_create(ruta);

	strcpy(yama->ip_filesystem, config_get_string_value(yamaConfig, "IP_FILESYSTEM"));
	strcpy(yama->puerto_entrada, config_get_string_value(yamaConfig, "PUERTO_YAMA"));
	strcpy(yama->puerto_master, config_get_string_value(yamaConfig, "PUERTO_MASTER"));
	strcpy(yama->puerto_filesystem, config_get_string_value(yamaConfig, "PUERTO_FILESYSTEM"));


	yama->retardo_planificacion = config_get_int_value(yamaConfig, "RETARDO_PLANIFICACION");
	yama->algoritmo_balanceo =          config_get_int_value(yamaConfig, "ALGORITMO_BALANCEO");
	yama->tipo_de_proceso = YAMA;

	config_destroy(yamaConfig);
	return yama;
}

void mostrarConfiguracion(Tyama *yama){

	printf("Puerto Entrada: %s\n",  yama->puerto_entrada);
	printf("IP Filesystem %s\n",    yama->ip_filesystem);
	printf("Puerto Master: %s\n",       yama->puerto_master);
	printf("Puerto Filesystem: %s\n", yama->puerto_filesystem);
	printf("Retardo Planificacion: %d\n",   yama->retardo_planificacion);
	printf("Algoritmo Balanceo: %d\n", yama->algoritmo_balanceo);
	printf("Tipo de proceso: %d\n", yama->tipo_de_proceso);
}

void conectarAFS(int* socketFS, Tyama *yama){
	int estado;
	Theader *head = malloc(sizeof(Theader));
	char * mensaje = malloc(100);

	head->tipo_de_proceso=YAMA;
	head->tipo_de_mensaje=INICIOYAMA;

	//Theader head;
	// Se trata de conectar con FS
	if ((*socketFS = conectarAServidor(yama->ip_filesystem, yama->puerto_filesystem)) < 0){
		sprintf(mensaje, "No se pudo conectar con FS! sock_fs: %d\n", *socketFS);
		logAndExit(mensaje);
	}


	// No permitimos continuar la ejecucion hasta lograr un handshake con FS
	if ((estado = send(*socketFS, head, sizeof(Theader), 0)) == -1){
		sprintf(mensaje, "Fallo send() al socket: %d\n", *socketFS);
		logAndExit(mensaje);
	}

	printf("Se enviaron: %d bytes a FS del handshake \n", estado);
}



void masterHandler(void *client_sock){
	int sockMaster = (int *)client_sock;
	int stat;
	int estado;



	setearGlobales();

	Theader head = {.tipo_de_proceso = MASTER, .tipo_de_mensaje = 0};


	TpackBytes *pathArchivoAReducir;
	TpackBytes *pathResultado;

	char* buffer;

	puts("Nuevo hilo MASTERHANDLER creado");
	puts("Esperando solicitud de master");

	while((estado = recv(sockMaster, &head, HEAD_SIZE, 0)) > 0){
		switch(head.tipo_de_mensaje){

		case INICIOMASTER:
			puts("Master quiere iniciar un nuevo JOB. esperamos a recibir la info");

			break;

		case PATH_FILE_TOREDUCE:

			puts("Nos llega el path del archivo a reducir");


			if ((buffer = recvGeneric(sockMaster)) == NULL){
				puts("Fallo recepcion de PATH_FILE_TOREDUCE");
				return;
			}

			if ((pathArchivoAReducir = (TpackBytes *) deserializeBytes(buffer)) == NULL){
				puts("Fallo deserializacion de Bytes del path arch a reducir");
				return;
			}

			printf("Path archivo a reducir: : %s\n",pathArchivoAReducir->bytes);

		break;

		case PATH_RES_FILE:
			puts("Nos llega el path del resultado");
			if ((buffer = recvGeneric(sockMaster)) == NULL){
				puts("Fallo recepcion de PATH_RES_FILE");
				return;
			}
			if ((pathResultado = (TpackBytes *) deserializeBytes(buffer)) == NULL){
				puts("Fallo deserializacion de Bytes del path_res_file");
				return;
			}
			printf("Path del resultado: : %s\n",pathResultado->bytes);


			puts("Pido info a filesystem sobre el archivo a transformar");

			head.tipo_de_proceso=YAMA;
			head.tipo_de_mensaje=PREG_FILEINFO;
			//packSize = 0;

			/*buffer=serializeBytes(head,pathArchivoAReducir,(strlen(pathArchivoAReducir)+1),&packSize);
			puts("Path del archivo a reducir serializado; lo enviamos");
			if ((stat = send(socketFS, buffer, packSize, 0)) == -1){
				puts("no se pudo enviar Path del archivo a reducir a FILESYSTEM. ");
				return;
			}
			printf("se enviaron %d bytes del Path del archivo a reducir a FS\n",stat);


			//Espero a que FS me envie toda la informacion del archivo para seguir ejecutando

			while((stat = recv(socketFS, &head, sizeof(Theader), 0))>0){
				if(head->tipo_char  *nombre =malloc(MAXSIZETEMPNAME)de_proceso==FILESYSTEM && head->tipo_de_mensaje==RTA_FILEINFO){
					//recibimos la lista de bloques y demases que componen al archivo a reducir
				}

			}
			 */
			//Como este es el último atributo que recibimos de master.  Aca le enviamos la "info" de los workers a los que se tiene que conectar
			//Por el momento info hardcode y sera la info de los unicos 2 workers conectados. Se la pedimos al filesystem y cuando nos la devuelve, le reenviamos a master.
			/* pido info, la proceso y ahora se a que workers el master se va a conectar y se los paso...*/
			//x ahora solo le aviso q inice la transf local
			//Theader * head; esto no sabemos por que va


			t_list *listaComposicionArchivo=list_create();
			generarListaComposicionArchivoHardcode(listaComposicionArchivo);


			t_list *listaInfoNodos=list_create();
			generarListaInfoNodos(listaInfoNodos);



			t_list *listaBloquesPlanificados=planificar(listaComposicionArchivo,listaInfoNodos);


			//generarListaBloquesHardcode(listaBloquesPlanificados);

			if((stat=responderSolicTransf(sockMaster,listaBloquesPlanificados))<0){
				puts("No se pudo responder la solicitud de transferencia");
				return;
			}



			break;
		default:
			break;


		}
	}
	freeAndNULL((void **) &buffer);
}

char *  generarNombreTemporal(int idMaster){

	char *temp = malloc(MAXSIZETEMPNAME);

	string_append(&temp,"tmp/Master");
	string_append(&temp,string_itoa(idMaster));
	string_append(&temp,"-temp");
	string_append(&temp,string_itoa(idTempName++));


	return temp;


}

void setearGlobales(){

	idTempName=0;
	pthread_mutex_init(&mux_idGlobal,   NULL);


	pthread_mutex_lock(&mux_idGlobal);
	idPropio=idMasterGlobal++;
	pthread_mutex_unlock(&mux_idGlobal);

}

int responderSolicTransf(int sockMaster,t_list * listaBloques){


	//aca planifico, pido la inf del archivo,etcetc

	//x ahora envio una rta hardcodeada pero siguiendo el formato
	int i,packSize,stat;
	char *buffer;

	Theader head;
	head.tipo_de_proceso=YAMA;
	head.tipo_de_mensaje=INFOBLOQUE;


	printf("Cantidad de paquetes con info de bloques a enviar: %d\n",list_size(listaBloques));
	for (i=0;i<list_size(listaBloques);i++){
		if(i<=list_size(listaBloques)-2){
			head.tipo_de_mensaje=INFOBLOQUE;
		}else{
			head.tipo_de_mensaje=INFOULTIMOBLOQUE;
		}
		packSize=0;
		TpackInfoBloque *bloqueAEnviar = list_get(listaBloques,i);
		buffer=serializeInfoBloque(head,bloqueAEnviar,&packSize);

		printf("Info del bloque %d serializado, enviamos\n",bloqueAEnviar->bloque);
		if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
			puts("no se pudo enviar info del bloque. ");
			return  FALLO_SEND;
		}
		printf("se enviaron %d bytes de la info del bloque\n",stat);
	}

	puts("Se envio la info de todos los bloques.");





	return 0;


}

t_list * planificar(t_list * listaComposicionArchivo,t_list * listaInfoNodos){

	t_list * listaPlanificada=list_create();
	t_list * listaWorkersPlanificacion = list_create();

	int base = 2;
	int pwlClock=0;
	int stat;
	TpackageInfoNodo *aux;// = malloc(sizeof aux);
	//lleno la lista con los workers asociados a esta transfo.
	int i;
	for(i=0;i<list_size(listaInfoNodos);i++){
		aux = list_get(listaInfoNodos,i);
		Tplanificacion * nodo = malloc(sizeof nodo);
		nodo->infoNodo.nombreNodo=malloc(MAXSIZETEMPNAME);
		nodo->infoNodo.nombreNodo=aux->nombreNodo;
		nodo->infoNodo.nombreLen=aux->nombreLen;
		nodo->infoNodo.ipNodo=malloc(MAXIMA_LONGITUD_IP);
		nodo->infoNodo.ipNodo=aux->ipNodo;
		nodo->infoNodo.ipLen=aux->ipLen;
		nodo->infoNodo.puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
		nodo->infoNodo.puertoWorker=aux->puertoWorker;
		nodo->infoNodo.puertoLen=aux->puertoLen;
		nodo->disponibilidadBase=base;
		nodo->pwl=pwlClock;
		nodo->availability=nodo->disponibilidadBase+nodo->pwl;
		nodo->clock=false;
		list_add(listaWorkersPlanificacion,nodo);
	}

	stat = posicionarClock(listaWorkersPlanificacion);
	int k;
	for(k=0;k<list_size(listaComposicionArchivo);k++){
		TpackageUbicacionBloques *bloqueAux=list_get(listaComposicionArchivo,k);

		TpackInfoBloque *bloque1 = asignarBloque(bloqueAux,listaWorkersPlanificacion);

		list_add(listaPlanificada,bloque1);
	}


	return listaPlanificada;
}

TpackInfoBloque * asignarBloque(TpackageUbicacionBloques *bloqueAux,t_list *listaWorkersPlanificacion){


	Tplanificacion *nodoApuntado = getNodoApuntado(listaWorkersPlanificacion);
	TpackInfoBloque *bloqueRet=malloc(sizeof(bloqueRet));
	if(nodoApuntado->availability>0){
		if(bloqueAux->nombreNodoC1==nodoApuntado->infoNodo.nombreNodo || bloqueAux->nombreNodoC2==nodoApuntado->infoNodo.nombreNodo){
			if(bloqueAux->nombreNodoC1==nodoApuntado->infoNodo.nombreNodo ){
				bloqueRet->bloque=bloqueAux->bloqueC1;
			}
			if(bloqueAux->nombreNodoC2==nodoApuntado->infoNodo.nombreNodo ){
				bloqueRet->bloque=bloqueAux->bloqueC2;
			}
			mergeBloque(bloqueRet,nodoApuntado,bloqueAux);
			nodoApuntado->availability-=1;
			avanzarClock(listaWorkersPlanificacion);

		}else{
			avanzarClock(listaWorkersPlanificacion);
			asignarBloque(bloqueAux,listaWorkersPlanificacion);
		}
	}else{
		avanzarClock(listaWorkersPlanificacion);
		asignarBloque(bloqueAux,listaWorkersPlanificacion);
	}


	return bloqueRet;
}
void avanzarClock(t_list *listaWorkersPlanificacion){
	int i;
	Tplanificacion *siguiente;
	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux=list_get(listaWorkersPlanificacion,i);
		if(aux->clock){
			if(i+1!=list_size(listaWorkersPlanificacion)){
				siguiente=list_get(listaWorkersPlanificacion,i+1);
			}else{
				siguiente=list_get(listaWorkersPlanificacion,0);
			}
			aux->clock=false;
			siguiente->clock=true;
			if(siguiente->availability==0){
				siguiente->availability=siguiente->disponibilidadBase;
				avanzarClock(listaWorkersPlanificacion);
			}
			break;
		}
	}
}
void mergeBloque(TpackInfoBloque *bloqueRet,Tplanificacion *nodoApuntado,TpackageUbicacionBloques *bloqueAux){
	int nombreLen=6;


	bloqueRet->nombreNodo=malloc(nombreLen);
	bloqueRet->nombreNodo=nodoApuntado->infoNodo.nombreNodo;
	bloqueRet->nombreLen=nodoApuntado->infoNodo.nombreLen;
	bloqueRet->nombreTemporal=malloc(MAXSIZETEMPNAME);
	bloqueRet->nombreTemporal="tmp-asd";
	bloqueRet->nombreTemporalLen=strlen(bloqueRet->nombreTemporal)+1;
	bloqueRet->bytesOcupados=bloqueAux->finBloque;
}

Tplanificacion * getNodoApuntado(t_list * listaWorkersPlanificacion){
	//todo chequeo errores
	int i;
	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux= list_get(listaWorkersPlanificacion,i);
		if(aux->clock){
			return aux;
		}
	}
	return NULL;
}


int posicionarClock(t_list *listaWorkers){

	int i;
	int disponibilidadMasAlta = -1;
	bool empate=false;
	for(i=0;i<list_size(listaWorkers);i++){
		Tplanificacion *aux = list_get(listaWorkers,i);
		if(aux->availability > disponibilidadMasAlta){
			disponibilidadMasAlta=aux->availability;
			empate=false;
		}else if(aux->availability==disponibilidadMasAlta){
			empate=true;
		}
	}
	if(empate){
		desempatarClock(disponibilidadMasAlta,listaWorkers);
	}



	return 0;
}

int desempatarClock(int disponibilidadMasAlta,t_list * listaWorkers){

	int i;
	int historico1=-1;
	int historico2=-1;
	int indiceAModificar;
	Tplanificacion *aux;
	for(i=0;i<list_size(listaWorkers);i++){
		aux = list_get(listaWorkers,i);
		if(aux->availability == disponibilidadMasAlta){
			historico1=getHistorico(aux);
		}
		if(historico1>=historico2){
			historico2=historico1;
			indiceAModificar=i;
		}

	}
	aux=list_get(listaWorkers,indiceAModificar);
	aux->clock=true;

	return 0;
}

int getHistorico(Tplanificacion *infoWorker){
	int i;
	pthread_mutex_lock(&mux_listaHistorica);
	for(i=0;i<list_size(listaHistoricaTareas);i++){
		ThistorialTareas *aux = list_get(listaHistoricaTareas,i);
		if(infoWorker->infoNodo.nombreNodo==aux->nombreNodo){
			pthread_mutex_unlock(&mux_listaHistorica);
			return aux->tareasRealizadas;
		}
	}
	pthread_mutex_unlock(&mux_listaHistorica);
	return 0;
}


/*void generarListaBloquesHardcode(t_list * listaBloques){


	//toda esta funcion va a volar, esta hecho asi nomas para ir probandolo. esta tod hardco



	int nombreLen=6;
	int ipLen=10;
	int puertoLen=5;
	int tmpLen=MAXSIZETEMPNAME;




	TpackInfoBloque *bloque1 = malloc(sizeof bloque1);


	bloque1->nombreNodo=malloc(nombreLen);
	bloque1->nombreNodo="Nodo1";
	bloque1->nombreLen=strlen(bloque1->nombreNodo)+1;
	bloque1->ipNodo=malloc(ipLen);
	bloque1->ipNodo="127.0.0.1";
	bloque1->ipLen=strlen(bloque1->ipNodo)+1;
	bloque1->puertoWorker=malloc(puertoLen);
	bloque1->puertoWorker = "5013";
	bloque1->puertoLen=strlen(bloque1->puertoWorker)+1;
	bloque1->bloque=38;
	bloque1->bytesOcupados=10180;
	bloque1->nombreTemporal=malloc(tmpLen);
	bloque1->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque1->nombreTemporalLen=strlen(bloque1->nombreTemporal) +1;
	list_add(listaBloques,bloque1);


	TpackInfoBloque *bloque2 = malloc(sizeof bloque2);
	bloque2->nombreNodo=malloc(nombreLen);
	bloque2->nombreNodo="Nodo1";
	bloque2->nombreLen=strlen(bloque2->nombreNodo)+1;
	bloque2->ipNodo=malloc(ipLen);
	bloque2->ipNodo="127.0.0.1";
	bloque2->ipLen=strlen(bloque2->ipNodo)+1;
	bloque2->puertoWorker=malloc(puertoLen);
	bloque2->puertoWorker = "5013";
	bloque2->puertoLen=strlen(bloque2->puertoWorker)+1;
	bloque2->bloque=39;
	bloque2->bytesOcupados=1048576;
	bloque2->nombreTemporal=malloc(tmpLen);
	bloque2->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque2->nombreTemporalLen=strlen(bloque2->nombreTemporal) +1;
	list_add(listaBloques,bloque2);




	TpackInfoBloque *bloque3 = malloc(sizeof bloque3);
	bloque3->nombreNodo=malloc(nombreLen);
	bloque3->nombreNodo="Nodo1";
	bloque3->nombreLen=strlen(bloque3->nombreNodo)+1;
	bloque3->ipNodo=malloc(ipLen);
	bloque3->ipNodo="127.0.0.1";
	bloque3->ipLen=strlen(bloque3->ipNodo)+1;
	bloque3->puertoWorker=malloc(puertoLen);
	bloque3->puertoWorker = "5013";
	bloque3->puertoLen=strlen(bloque3->puertoWorker)+1;
	bloque3->bloque=44;
	bloque3->bytesOcupados=1048576;
	bloque3->nombreTemporal=malloc(tmpLen);
	bloque3->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque3->nombreTemporalLen=strlen(bloque3->nombreTemporal) +1;
	list_add(listaBloques,bloque3);




	TpackInfoBloque *bloque4 = malloc(sizeof bloque4);
	bloque4->nombreNodo=malloc(nombreLen);
	bloque4->nombreNodo="Nodo1";
	bloque4->nombreLen=strlen(bloque4->nombreNodo)+1;
	bloque4->ipNodo=malloc(ipLen);
	bloque4->ipNodo="127.0.0.1";
	bloque4->ipLen=strlen(bloque4->ipNodo)+1;
	bloque4->puertoWorker=malloc(puertoLen);
	bloque4->puertoWorker = "5013";
	bloque4->puertoLen=strlen(bloque4->puertoWorker)+1;
	bloque4->bloque=39;
	bloque4->bytesOcupados=1048576;
	bloque4->nombreTemporal=malloc(tmpLen);
	bloque4->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque4->nombreTemporalLen=strlen(bloque4->nombreTemporal) +1;
	list_add(listaBloques,bloque4);




	TpackInfoBloque *bloque5 = malloc(sizeof bloque5);
	bloque5->nombreNodo=malloc(nombreLen);
	bloque5->nombreNodo="Nodo1";
	bloque5->nombreLen=strlen(bloque5->nombreNodo)+1;
	bloque5->ipNodo=malloc(ipLen);
	bloque5->ipNodo="127.0.0.1";
	bloque5->ipLen=strlen(bloque1->ipNodo)+1;
	bloque5->puertoWorker=malloc(puertoLen);
	bloque5->puertoWorker = "5013";
	bloque5->puertoLen=strlen(bloque5->puertoWorker)+1;
	bloque5->bloque=46;
	bloque5->bytesOcupados=1048576;
	bloque5->nombreTemporal=malloc(tmpLen);
	bloque5->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque5->nombreTemporalLen=strlen(bloque5->nombreTemporal) +1;
	list_add(listaBloques,bloque5);



}*/

void generarListaComposicionArchivoHardcode(t_list * listaComposicion){

	//toda esta funcion va a volar, esta hecho asi nomas para ir probandolo. esta tod hardco
	int maxNombreNodo=10;
	TpackageUbicacionBloques *bloque0 = malloc(sizeof(bloque0));
	bloque0->bloque=0;
	bloque0->nombreNodoC1=malloc(sizeof(maxNombreNodo));
	bloque0->nombreNodoC1="Nodo1";
	bloque0->nombreNodoC1Len=strlen(bloque0->nombreNodoC1)+1;
	bloque0->bloqueC1=5;
	bloque0->nombreNodoC2=malloc(sizeof(maxNombreNodo));
	bloque0->nombreNodoC2="Nodo2";
	bloque0->nombreNodoC2Len=strlen(bloque0->nombreNodoC2)+1;
	bloque0->bloqueC2=2;
	bloque0->finBloque=1048576;

	list_add(listaComposicion,bloque0);

	TpackageUbicacionBloques *bloque1 = malloc(sizeof(bloque1));
	bloque1->bloque=1;
	bloque1->nombreNodoC1=malloc(sizeof(maxNombreNodo));
	bloque1->nombreNodoC1="Nodo2";
	bloque1->nombreNodoC1Len=strlen(bloque1->nombreNodoC1)+1;
	bloque1->bloqueC1=10;
	bloque1->nombreNodoC2=malloc(sizeof(maxNombreNodo));
	bloque1->nombreNodoC2="Nodo1";
	bloque1->nombreNodoC2Len=strlen(bloque1->nombreNodoC2)+1;
	bloque1->bloqueC2=7;
	bloque1->finBloque=1048500;
	list_add(listaComposicion,bloque1);

	TpackageUbicacionBloques *bloque2 = malloc(sizeof(bloque2));
	bloque2->bloque=2;
	bloque2->nombreNodoC1=malloc(sizeof(maxNombreNodo));
	bloque2->nombreNodoC1="Nodo2";
	bloque2->nombreNodoC1Len=strlen(bloque2->nombreNodoC1)+1;
	bloque2->bloqueC1=12;
	bloque2->nombreNodoC2=malloc(sizeof(maxNombreNodo));
	bloque2->nombreNodoC2="Nodo1";
	bloque2->nombreNodoC2Len=strlen(bloque2->nombreNodoC2)+1;
	bloque2->bloqueC2=3;
	bloque2->finBloque=1048516;
	list_add(listaComposicion,bloque2);

}

void generarListaInfoNodos(t_list * listaNodos){



	int nombreLen=6;
	int ipLen=10;
	int puertoLen=5;


	TpackageInfoNodo *nodo1 = malloc(sizeof(nodo1));
	nodo1->nombreNodo=malloc(nombreLen);
	nodo1->nombreNodo="Nodo1";
	nodo1->nombreLen=strlen(nodo1->nombreNodo)+1;
	nodo1->ipNodo=malloc(ipLen);
	nodo1->ipNodo="127.0.0.1";
	nodo1->ipLen=strlen(nodo1->ipNodo)+1;
	nodo1->puertoWorker=malloc(puertoLen);
	nodo1->puertoWorker = "5013";
	nodo1->puertoLen=strlen(nodo1->puertoWorker)+1;
	list_add(listaNodos,nodo1);

	TpackageInfoNodo *nodo2 = malloc(sizeof(nodo2));
	nodo2->nombreNodo=malloc(nombreLen);
	nodo2->nombreNodo="Nodo2";
	nodo2->nombreLen=strlen(nodo2->nombreNodo)+1;
	nodo2->ipNodo=malloc(ipLen);
	nodo2->ipNodo="127.0.0.1";
	nodo2->ipLen=strlen(nodo2->ipNodo)+1;
	nodo2->puertoWorker=malloc(puertoLen);
	nodo2->puertoWorker = "5014";
	nodo2->puertoLen=strlen(nodo2->puertoWorker)+1;

	list_add(listaNodos,nodo2);


}








