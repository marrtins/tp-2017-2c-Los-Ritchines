
#include "funcionesYM.h"



int idTempName,idPropio;


extern int idMasterGlobal,idJobGlobal,idTareaGlobal;


extern t_list * listaHistoricaTareas,*listaCargaGlobal,* listaEstadoEnProceso,*listaEstadoError,*listaEstadoFinalizadoOK;
extern pthread_mutex_t mux_idTareaGlobal,mux_listaHistorica,mux_listaCargaGlobal,mux_idGlobal,mux_listaEnProceso,mux_listaError,mux_listaFinalizado,mux_jobIdGlobal;





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
	int idTareaFinalizada;
	idTempName=0;




	pthread_mutex_lock(&mux_idGlobal);
	idPropio=idMasterGlobal++;
	pthread_mutex_unlock(&mux_idGlobal);

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
		case FINTRANSFORMACIONLOCALOK:
			idTareaFinalizada = recibirValor(sockMaster);
			printf("FINTRANSFORMACIONLOCAL OK de la tarea%d\n",idTareaFinalizada);
			puts("actuializo tbala de estados");
			moverAListaFinalizadosOK(idTareaFinalizada);

			break;
		case FINTRANSFORMACIONLOCALFAIL:
			idTareaFinalizada = recibirValor(sockMaster);
			printf("FINTRANSFORMACIONLOCAL FAIL de la tarea%d\n",idTareaFinalizada);
			puts("actuializo tbala de estados");
			moverAListaError(idTareaFinalizada);

			break;
		default:
			break;


		}
	}
	freeAndNULL((void **) &buffer);
}
int moverAListaFinalizadosOK(int idTareaFinalizada){

	int i;
	MUX_LOCK(&mux_listaEnProceso);
	MUX_LOCK(&mux_listaFinalizado);
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados *aux  = list_get(listaEstadoEnProceso,i);
		if(aux->idTarea==idTareaFinalizada){
			list_add(listaEstadoFinalizadoOK,aux);
			list_remove(listaEstadoEnProceso,i);
			mostrarTablaDeEstados();
			MUX_UNLOCK(&mux_listaFinalizado);
			MUX_UNLOCK(&mux_listaEnProceso);
			return 0;
		}
	}
	mostrarTablaDeEstados();
	MUX_UNLOCK(&mux_listaFinalizado);
	MUX_UNLOCK(&mux_listaEnProceso);
	return -1;
}

int moverAListaError (int idTareaFinalizada){

	int i;
	MUX_LOCK(&mux_listaEnProceso);
	MUX_LOCK(&mux_listaError);
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados *aux  = list_get(listaEstadoEnProceso,i);
		if(aux->idTarea==idTareaFinalizada){
			list_add(listaEstadoError,aux);
			list_remove(listaEstadoEnProceso,i);
			mostrarTablaDeEstados();
			MUX_UNLOCK(&mux_listaError);
			MUX_UNLOCK(&mux_listaEnProceso);
			return 0;
		}
	}
	mostrarTablaDeEstados();
	MUX_UNLOCK(&mux_listaError);
	MUX_UNLOCK(&mux_listaEnProceso);
	return -1;
}

char *  generarNombreTemporal(){

	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append(&temp,string_itoa(idPropio));
	string_append(&temp,"-temp");
	string_append(&temp,string_itoa(idTempName++));


	return temp;
}



int responderSolicTransf(int sockMaster,t_list * listaBloques){


	//aca planifico, pido la inf del archivo,etcetc

	//x ahora envio una rta hardcodeada pero siguiendo el formato
	int i,packSize,stat,jobActual;
	char *buffer;


	pthread_mutex_lock(&mux_jobIdGlobal);
	jobActual=idJobGlobal++;
	pthread_mutex_unlock(&mux_jobIdGlobal);

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
		MUX_LOCK(&mux_idTareaGlobal);
		bloqueAEnviar->idTarea = idTareaGlobal++;
		MUX_UNLOCK(&mux_idTareaGlobal);
		buffer=serializeInfoBloque(head,bloqueAEnviar,&packSize);

		printf("Info del bloque %d serializado, enviamos\n",bloqueAEnviar->bloqueDelArchivo);

		if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
			puts("no se pudo enviar info del bloque. ");
			return  FALLO_SEND;
		}
		printf("se enviaron %d bytes de la info del bloque\n",stat);

		agregarAListaEnProceso(jobActual,bloqueAEnviar->idTarea,TRANSFORMACION,bloqueAEnviar);


	}

	puts("Se envio la info de todos los bloques.");





	return 0;


}

void agregarAListaEnProceso(int jobActual, int idTarea,int etapa, TpackInfoBloque *bloque){

	MUX_LOCK(&mux_listaEnProceso);
	TpackTablaEstados * estado = malloc(sizeof estado);
	estado->idTarea=idTarea;
	estado->job=jobActual;
	estado->master= idPropio;
	estado->nodo=malloc(TAMANIO_NOMBRE_NODO);
	estado->nodo=bloque->nombreNodo;
	estado->bloque=bloque->bloqueDelArchivo;
	estado->etapa=etapa;
	estado->nombreArchTemporal=malloc(TAMANIO_NOMBRE_TEMPORAL);
	estado->nombreArchTemporal=bloque->nombreTemporal;
	list_add(listaEstadoEnProceso,estado);
	MUX_UNLOCK(&mux_listaEnProceso);

	mostrarTablaDeEstados();

}

void mostrarTablaDeEstados(){
	puts("Hubo un cambio de estado, va la tabla actual:");
	int i;
	int id,job,master,bloque;
	char * etapa;
	char * nodo;
	char * archivoTemporal;
	printf("%-10s%-10s%-10s%-25s%-10s%-25s%-25s%-25s\n", "idTarea", "Job", "Master", "Nodo", "Bloque","Etapa","Archivo Temporal","Estado");
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados * aux = list_get(listaEstadoEnProceso,i);
		id = aux->idTarea;
		job=aux->job;
		master = aux->master;
		bloque=aux->bloque;
		etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-10d%-10d%-10d%-25s%-10d%-25s%-25s%-25s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"En Proceso");
	}

	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
		TpackTablaEstados * aux = list_get(listaEstadoFinalizadoOK,i);
		id = aux->idTarea;
		job=aux->job;
		master = aux->master;
		bloque=aux->bloque;
		etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-10d%-10d%-10d%-25s%-10d%-25s%-25s%-25s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"Finalizado OK");
	}

	for(i=0;i<list_size(listaEstadoError);i++){
		TpackTablaEstados * aux = list_get(listaEstadoError,i);
		id = aux->idTarea;
		job=aux->job;
		master = aux->master;
		bloque=aux->bloque;
		etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-10d%-10d%-10d%-25s%-10d%-25s%-25s%-25s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"ERROR");
	}
}

char * getNombreEtapa(int etapaEnum){
	char * ret = string_new();
	if(etapaEnum==TRANSFORMACION){
		string_append(&ret,"Transformacion");
	}else if(etapaEnum==REDUCCIONGLOBAL){
		string_append(&ret,"Reduccion Global");
	}else if(etapaEnum==REDUCCIONLOCAL){
		string_append(&ret,"Reduccion Local");
	}else{
		string_append(&ret,"");
	}
	return ret;
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
	nodo1->tamanioNombre=strlen(nodo1->nombreNodo)+1;
	nodo1->ipNodo=malloc(ipLen);
	nodo1->ipNodo="127.0.0.1";
	nodo1->tamanioIp=strlen(nodo1->ipNodo)+1;
	nodo1->puertoWorker=malloc(puertoLen);
	nodo1->puertoWorker = "5013";
	nodo1->tamanioPuerto=strlen(nodo1->puertoWorker)+1;
	list_add(listaNodos,nodo1);

	TpackageInfoNodo *nodo2 = malloc(sizeof(nodo2));
	nodo2->nombreNodo=malloc(nombreLen);
	nodo2->nombreNodo="Nodo2";
	nodo2->tamanioNombre=strlen(nodo2->nombreNodo)+1;
	nodo2->ipNodo=malloc(ipLen);
	nodo2->ipNodo="127.0.0.1";
	nodo2->tamanioIp=strlen(nodo2->ipNodo)+1;
	nodo2->puertoWorker=malloc(puertoLen);
	nodo2->puertoWorker = "5014";
	nodo2->tamanioPuerto=strlen(nodo2->puertoWorker)+1;

	list_add(listaNodos,nodo2);


}








