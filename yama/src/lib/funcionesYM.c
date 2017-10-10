#include "funcionesYM.h"
extern int socketFS;
#include <commons/collections/list.h>

int idTempName,idMasterPropio;


extern int idMasterGlobal;



pthread_mutex_t mux_idGlobal;


#define MAXSIZETEMPNAME 35








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
				if(head->tipo_de_proceso==FILESYSTEM && head->tipo_de_mensaje==RTA_FILEINFO){
					//recibimos la lista de bloques y demases que componen al archivo a reducir
				}

			}
			 */
			//Como este es el último atributo que recibimos de master.  Aca le enviamos la "info" de los workers a los que se tiene que conectar
			//Por el momento info hardcode y sera la info de los unicos 2 workers conectados. Se la pedimos al filesystem y cuando nos la devuelve, le reenviamos a master.
			/* pido info, la proceso y ahora se a que workers el master se va a conectar y se los paso...*/
			//x ahora solo le aviso q inice la transf local
			//Theader * head; esto no sabemos por que va

			t_list *listaBloques=list_create();
			generarListaBloquesHardcode(listaBloques);

			if((stat=responderSolicTransf(sockMaster,listaBloques))<0){
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

	pthread_mutex_lock(&mux_idGlobal);
	idMasterPropio=idMasterGlobal++;
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



void generarListaBloquesHardcode(t_list * listaBloques){


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
	bloque1->puertoWorker = "5010";
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
	bloque2->puertoWorker = "5010";
	bloque2->puertoLen=strlen(bloque2->puertoWorker)+1;
	bloque2->bloque=39;
	bloque2->bytesOcupados=1048576;
	bloque2->nombreTemporal=malloc(tmpLen);
	bloque2->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque2->nombreTemporalLen=strlen(bloque2->nombreTemporal) +1;
	list_add(listaBloques,bloque2);




	TpackInfoBloque *bloque3 = malloc(sizeof bloque3);
	bloque3->nombreNodo=malloc(nombreLen);
	bloque3->nombreNodo="Nodo2";
	bloque3->nombreLen=strlen(bloque3->nombreNodo)+1;
	bloque3->ipNodo=malloc(ipLen);
	bloque3->ipNodo="127.0.0.1";
	bloque3->ipLen=strlen(bloque3->ipNodo)+1;
	bloque3->puertoWorker=malloc(puertoLen);
	bloque3->puertoWorker = "5011";
	bloque3->puertoLen=strlen(bloque3->puertoWorker)+1;
	bloque3->bloque=44;
	bloque3->bytesOcupados=1048576;
	bloque3->nombreTemporal=malloc(tmpLen);
	bloque3->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque3->nombreTemporalLen=strlen(bloque3->nombreTemporal) +1;
	list_add(listaBloques,bloque3);




	TpackInfoBloque *bloque4 = malloc(sizeof bloque4);
	bloque4->nombreNodo=malloc(nombreLen);
	bloque4->nombreNodo="Nodo2";
	bloque4->nombreLen=strlen(bloque4->nombreNodo)+1;
	bloque4->ipNodo=malloc(ipLen);
	bloque4->ipNodo="127.0.0.1";
	bloque4->ipLen=strlen(bloque4->ipNodo)+1;
	bloque4->puertoWorker=malloc(puertoLen);
	bloque4->puertoWorker = "5011";
	bloque4->puertoLen=strlen(bloque4->puertoWorker)+1;
	bloque4->bloque=39;
	bloque4->bytesOcupados=1048576;
	bloque4->nombreTemporal=malloc(tmpLen);
	bloque4->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque4->nombreTemporalLen=strlen(bloque4->nombreTemporal) +1;
	list_add(listaBloques,bloque4);




	TpackInfoBloque *bloque5 = malloc(sizeof bloque5);
	bloque5->nombreNodo=malloc(nombreLen);
	bloque5->nombreNodo="Nodo2";
	bloque5->nombreLen=strlen(bloque5->nombreNodo)+1;
	bloque5->ipNodo=malloc(ipLen);
	bloque5->ipNodo="127.0.0.1";
	bloque5->ipLen=strlen(bloque1->ipNodo)+1;
	bloque5->puertoWorker=malloc(puertoLen);
	bloque5->puertoWorker = "5011";
	bloque5->puertoLen=strlen(bloque5->puertoWorker)+1;
	bloque5->bloque=46;
	bloque5->bytesOcupados=1048576;
	bloque5->nombreTemporal=malloc(tmpLen);
	bloque5->nombreTemporal=generarNombreTemporal(idMasterPropio);;
	bloque5->nombreTemporalLen=strlen(bloque5->nombreTemporal) +1;
	list_add(listaBloques,bloque5);



}



