/*
 * planificacion.c
 *
 *  Created on: 9/11/2017
 *      Author: utnso
 */
#include "funcionesYM.h"

extern t_list * listaEstadoEnProceso,*listaEstadoFinalizadoOK,*listaEstadoError,*listaHistoricaTareas,*listaCargaGlobal,*listaJobFinalizados;
extern Tyama * yama;
extern float retardoPlanificacionSegs;
extern int idTareaGlobal;
extern int idTempName;


t_list * planificar(TjobMaster *job){




	t_list * listaPlanificada=list_create();
	t_list * listaWorkersPlanificacion = list_create();


	int stat,pwl;
	TpackageInfoNodo *aux;
	//lleno la lista con los workers asociados a esta transfo.
	int i;
	for(i=0;i<list_size(job->listaNodosArchivo);i++){
		aux = list_get(job->listaNodosArchivo,i);
		Tplanificacion * nodo = malloc(sizeof (Tplanificacion));
		nodo->infoNodo.nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
		nodo->infoNodo.nombreNodo=aux->nombreNodo;
		nodo->infoNodo.tamanioNombre=aux->tamanioNombre;
		nodo->infoNodo.ipNodo=malloc(MAXIMA_LONGITUD_IP);
		nodo->infoNodo.ipNodo=aux->ipNodo;
		nodo->infoNodo.tamanioIp=aux->tamanioIp;
		nodo->infoNodo.puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
		nodo->infoNodo.puertoWorker=aux->puertoWorker;
		nodo->infoNodo.tamanioPuerto=aux->tamanioPuerto;
		nodo->disponibilidadBase=yama->disponibilidadBase;

		if(yama->algoritmo_balanceo==WCLOCK){
			int wlMax=getMayorCargaAllWorkers();
			int wl = getCargaWorker(aux->nombreNodo);
			pwl=wlMax-wl;
		}else{
			pwl=0;
		}

		nodo->pwl=pwl;
		nodo->availability=nodo->disponibilidadBase+nodo->pwl;
		nodo->clock=false;
		log_info(logInfo,"Nodo:%s, ava:%d, clock:%d\n",nodo->infoNodo.nombreNodo,nodo->availability,nodo->clock);
		list_add(listaWorkersPlanificacion,nodo);
	}

	if((stat = posicionarClock(listaWorkersPlanificacion))<0){
		puts("error");
		log_info(logInfo,"error posicionar clock");
	}
	int k;
	for(k=0;k<list_size(job->listaComposicionArchivo);k++){
		TpackageUbicacionBloques *bloqueAux=list_get(job->listaComposicionArchivo,k);
		log_info(logInfo,"asigno al bloque del archivo %d\n",bloqueAux->bloque);
		TpackInfoBloque *bloque = asignarBloque(bloqueAux,listaWorkersPlanificacion,job);
		log_info(logInfo,"asigne al bloque %d, el nodo %s. bloque databin %d \n",bloque->bloqueDelArchivo,bloque->nombreNodo,bloque->bloqueDelDatabin);
		list_add(listaPlanificada,bloque);
	}

	int q;
	for(q=0;q<list_size(listaPlanificada);q++){
		TpackInfoBloque *aux = list_get(listaPlanificada,q);
		log_info(logInfo,"b arch %d  bdata %d %d %s",aux->bloqueDelArchivo,aux->bloqueDelDatabin,aux->bytesOcupados,aux->nombreNodo);
	}

	//list_destroy_and_destroy_elements(listaPlanificada,liberarBloquesPlanificados);

	//list_destroy_and_destroy_elements(listaWorkersPlanificacion,liberarWorkersPlanificacion);

	return listaPlanificada;
}

void actualizarCargaWorkerEn(char * nombreNodo, int cantidadAAumentar){

	int i;
	for(i=0;i<list_size(listaCargaGlobal);i++){
		TcargaGlobal *aux = list_get(listaCargaGlobal,i);
		if(string_equals_ignore_case(aux->nombreNodo,nombreNodo)){
			aux->cargaGlobal +=cantidadAAumentar;
			mostrarTablaCargas();
			return;
		}
	}


	TcargaGlobal *nuevaCarga = malloc(sizeof(TcargaGlobal));
	nuevaCarga->nombreNodo=malloc(strlen(nombreNodo)+1);
	nuevaCarga->nombreNodo=nombreNodo;
	nuevaCarga->cargaGlobal=1;
	list_add(listaCargaGlobal,nuevaCarga);
	mostrarTablaCargas();
	return;

}
void aumentarHistoricoEn(char * nombreNodo,int cantidadAAumentar){
	int i;
	bool aumente =false;
	for(i=0;i<list_size(listaHistoricaTareas);i++){
		ThistorialTareas *historial = list_get(listaHistoricaTareas,i);
		if(string_equals_ignore_case(historial->nombreNodo,nombreNodo)){
			historial->tareasRealizadas+=cantidadAAumentar;
			aumente=true;
		}

	}

	if(!aumente){
		ThistorialTareas *historial = malloc(sizeof(ThistorialTareas));
		historial->nombreNodo=malloc(strlen(nombreNodo)+1);
		historial->nombreNodo=nombreNodo;
		historial->tareasRealizadas=1;
		list_add(listaHistoricaTareas,historial);
	}


	mostrarTablaHistorica();

}

void mostrarTablaHistorica(){
	log_info(logInfo,"muestro tabla historica");
	int i;
	for(i=0;i<list_size(listaHistoricaTareas);i++){
			ThistorialTareas *historial = list_get(listaHistoricaTareas,i);
			log_info(logInfo,"%s ; historico: %d\n",historial->nombreNodo,historial->tareasRealizadas);
	}
}

void mostrarTablaCargas(){
	int i;
	log_info(logInfo,"muestro tabla cargas");
	for(i=0;i<list_size(listaCargaGlobal);i++){
			TcargaGlobal *historial = list_get(listaCargaGlobal,i);
			log_info(logInfo,"%s ; carga: %d\n",historial->nombreNodo,historial->cargaGlobal);
	}
}

TpackInfoBloque * asignarBloque(TpackageUbicacionBloques *bloqueAux,t_list *listaWorkersPlanificacion,TjobMaster *job){


	Tplanificacion *nodoApuntado = getNodoApuntado(listaWorkersPlanificacion);
	TpackInfoBloque *bloqueRet=malloc(sizeof(TpackInfoBloque));

	log_info(logInfo,"en asignar bloque");
	log_info(logInfo,"copias del bloque %d: %s y %s \n",bloqueAux->bloque,bloqueAux->nombreNodoC1,bloqueAux->nombreNodoC2);
	log_info(logInfo,"nodo apuntado %s. availability %d\n",nodoApuntado->infoNodo.nombreNodo,nodoApuntado->availability);
	if((nodoApuntado->availability > 0 && string_equals_ignore_case(bloqueAux->nombreNodoC1,nodoApuntado->infoNodo.nombreNodo)) ||
			(nodoApuntado->availability > 0 && string_equals_ignore_case(bloqueAux->nombreNodoC2,nodoApuntado->infoNodo.nombreNodo))){
		if(string_equals_ignore_case(bloqueAux->nombreNodoC1,nodoApuntado->infoNodo.nombreNodo) ){
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC1;
			log_info(logInfo,"asigno al bloque del archivo %d, el bloque adtabin: %d del %s\n",bloqueAux->bloque,bloqueRet->bloqueDelDatabin,nodoApuntado->infoNodo.nombreNodo);
		}
		if(string_equals_ignore_case(bloqueAux->nombreNodoC2,nodoApuntado->infoNodo.nombreNodo) ){
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC2;
			log_info(logInfo,"asigno al bloque del archivo %d, el bloque adtabin: %d del %s\n",bloqueAux->bloque,bloqueRet->bloqueDelDatabin,nodoApuntado->infoNodo.nombreNodo);
		}

		mergeBloque(bloqueRet,nodoApuntado,bloqueAux,job->masterId);
		nodoApuntado->availability-=1;
		log_info(logInfo,"cumplio. le asigno el bloque %d al nodo apuntado %s. bloque databn %d\n",bloqueRet->bloqueDelArchivo,bloqueRet->nombreNodo,bloqueRet->bloqueDelDatabin);
		avanzarClock(listaWorkersPlanificacion);

	}else{
		log_info(logInfo,"no cumplio. busco el siguiente nodo");
		Tplanificacion *siguienteNodo = getSiguienteNodoDisponible(listaWorkersPlanificacion,bloqueAux->nombreNodoC1,bloqueAux->nombreNodoC2);
		log_info(logInfo,"consegui el siguiente dispo. el nodo %s\n",siguienteNodo->infoNodo.nombreNodo);
		if(string_equals_ignore_case(siguienteNodo->infoNodo.nombreNodo,bloqueAux->nombreNodoC1)){
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC1;
			log_info(logInfo,"asigno al bloque del archivo %d, el bloque adtabin: %d del %s\n",bloqueAux->bloque,bloqueRet->bloqueDelDatabin,siguienteNodo->infoNodo.nombreNodo);
		}else{
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC2;
			log_info(logInfo,"asigno al bloque del archivo %d, el bloque adtabin: %d del %s\n",bloqueAux->bloque,bloqueRet->bloqueDelDatabin,siguienteNodo->infoNodo.nombreNodo);
		}

		mergeBloque(bloqueRet,siguienteNodo,bloqueAux,job->masterId);
		siguienteNodo->availability-=1;
		log_info(logInfo,"le asigno el bloque %d al nodo q recien consegui %s, bloque databind :%d \n",bloqueRet->bloqueDelArchivo,siguienteNodo->infoNodo.nombreNodo,bloqueRet->bloqueDelDatabin);
	}
	aumentarHistoricoEn(bloqueRet->nombreNodo,1);
	actualizarCargaWorkerEn(bloqueRet->nombreNodo,1);
	log_info(logInfo,"aumento carga e historico y asi queda mi tabla de nodos\n");
	int i;
	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux=list_get(listaWorkersPlanificacion,i);
		log_info(logInfo,"nombrenodo %s. ava %d clock %d \n",aux->infoNodo.nombreNodo,aux->availability,aux->clock);
	}

	return bloqueRet;
}


void avanzarClock(t_list *listaWorkersPlanificacion){
	int i;
	log_info(logInfo,"avanzo clock");
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
			log_info(logInfo,"ahora %s tiene false y %s tiene true el clock\n",aux->infoNodo.nombreNodo,siguiente->infoNodo.nombreNodo);
			if(siguiente->availability==0){
				siguiente->availability=siguiente->disponibilidadBase;
				avanzarClock(listaWorkersPlanificacion);
			}
			break;
		}
	}
}
void mergeBloque(TpackInfoBloque *bloqueRet,Tplanificacion *nodoApuntado,TpackageUbicacionBloques *bloqueAux,int idMaster){



	bloqueRet->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
	bloqueRet->nombreNodo=nodoApuntado->infoNodo.nombreNodo;
	bloqueRet->tamanioNombre=nodoApuntado->infoNodo.tamanioNombre;
	bloqueRet->nombreTemporal=malloc(TAMANIO_NOMBRE_TEMPORAL);
	bloqueRet->nombreTemporal=generarNombreTemporal(idMaster);
	bloqueRet->nombreTemporalLen=strlen(bloqueRet->nombreTemporal)+1;
	bloqueRet->bytesOcupados=bloqueAux->finBloque;
	bloqueRet->ipWorker=malloc(MAXIMA_LONGITUD_IP);
	bloqueRet->ipWorker=nodoApuntado->infoNodo.ipNodo;
	bloqueRet->tamanioIp=nodoApuntado->infoNodo.tamanioIp;
	bloqueRet->puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
	bloqueRet->puertoWorker=nodoApuntado->infoNodo.puertoWorker;
	bloqueRet->tamanioPuerto=nodoApuntado->infoNodo.tamanioPuerto;
	bloqueRet->bloqueDelArchivo = bloqueAux->bloque;



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
	puts("error no hay nodo apuntado");
	return NULL;
}
Tplanificacion * getSiguienteNodoDisponible(t_list * listaWorkersPlanificacion,char * nombreNodo1,char* nombreNodo2){
	//todo chequeo errores
	int i;

	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux= list_get(listaWorkersPlanificacion,i);
		if(aux->clock){
			log_info(logInfo,"busco el siguiente al apuntado actualmente que es %s\n",aux->infoNodo.nombreNodo);
			Tplanificacion *siguienteDisponible = getSiguienteConDisponibilidadPositivaPosible(listaWorkersPlanificacion,i,nombreNodo1,nombreNodo2);
			return siguienteDisponible;

		}
	}

	puts("error no hay sig dispo");
	return NULL;
}

Tplanificacion *getSiguiente(t_list * listaWorkers,int indice){
	Tplanificacion *ret;

	if(indice>=list_size(listaWorkers)){
		ret = list_get(listaWorkers,0);
	}else{
		ret=list_get(listaWorkers,indice);
	}

	return ret;


}

Tplanificacion * getSiguienteConDisponibilidadPositivaPosible(t_list * listaWorkersPlanificacion, int indice,char * nombre1,char * nombre2){


	log_info(logInfo,"en get sig c dispo positiva posibla");
	int punt = indice +1;
	log_info(logInfo,"punt: %d\n",punt);
	log_info(logInfo,"indice: %d\n",indice);
	log_info(logInfo,"list size %d\n",list_size(listaWorkersPlanificacion));
	log_info(logInfo,"copia 1 %s copia 2 %s\n",nombre1,nombre2);
	int k=0;
	while(1){
		log_info(logInfo,"entro al while");
		log_info(logInfo,"punt: %d\n",punt);


		if(punt >=list_size(listaWorkersPlanificacion)){
			log_info(logInfo,"punt = lista size. punt=0");
			punt=0;
		}
		if(punt==indice){
			log_info(logInfo,"puts=indice");
			log_info(logInfo,"sumo dispo a todos");
			sumarDisponibilidadBaseATodos(listaWorkersPlanificacion);
		}
		Tplanificacion *aux = list_get(listaWorkersPlanificacion,punt);
		log_info(logInfo,"AUX: nodo:%s . availa: %d\n",aux->infoNodo.nombreNodo,aux->availability);
		if(aux->availability>0){
			if(string_equals_ignore_case(aux->infoNodo.nombreNodo,nombre1)||string_equals_ignore_case(aux->infoNodo.nombreNodo,nombre2) ){
				log_info(logInfo,"nombre nodo = n1 o n2");
				return aux;
			}
			log_info(logInfo,"nombre nodo distinto a n1 o n2");

		}
		log_info(logInfo,"doy la vuelta");
		punt++;
		k++;

		if(k>30){
			puts("atencion availa");
		}
	}


/*




	log_info(logInfo, "entron con indice= %d",indice);
	int i;
	for(i=indice+1;i<list_size(listaWorkersPlanificacion)+indice;i++){
		log_info(logInfo,"i:%d",i);
		if(i>=list_size(listaWorkersPlanificacion)){
			log_info(logInfo,"i>=list size. i=0");
			i=0;

		}
		Tplanificacion *aux = list_get(listaWorkersPlanificacion,i);
		log_info(logInfo,"pruebo con %s. availa: %d . necesito %s o %s\n",aux->infoNodo.nombreNodo,aux->availability,nombre1,nombre2);
		if(aux->availability>75){
			puts("ojo availability enorme");
			log_info(logInfo,"availa enorme");
		}
		log_info(logInfo,"if aux(%s) availa > 0. (availa = %d)",aux->infoNodo.nombreNodo,aux->availability);
		if(aux->availability>0){
			log_info(logInfo,"aux(%s) cumplio. ahora si i(%d) != indice(%d) entro",aux->infoNodo.nombreNodo,i,indice);
			if(i!= indice){
				log_info(logInfo,"cumplio. ahora if auxNombreNodo(%s)==nombre1%s || nombre 2 %s entro",aux->infoNodo.nombreNodo,nombre1,nombre2);
				if(string_equals_ignore_case(aux->infoNodo.nombreNodo,nombre1)||string_equals_ignore_case(aux->infoNodo.nombreNodo,nombre2) ){
					log_info(logInfo,"cumplio. devuelvo %s \n",aux->infoNodo.nombreNodo);
					return aux;
				}
			}else{
				log_info(logInfo,"no cumplio. sumo dispo a todos");
				sumarDisponibilidadBaseATodos(listaWorkersPlanificacion);
				log_info(logInfo,"di la vuelta. i: %d, nombre nodo(tendria  qser el apuntado) %s. sumo dispo a todos.\n",i,aux->infoNodo.nombreNodo);
				log_info(logInfo,("if i+1 = list size+indice entro"));
				if(i+1==list_size(listaWorkersPlanificacion)+indice){
					i=-1;
					log_info(logInfo,"entre. ahora i = %d",i);
				}log_info(logInfo," no entre. ahora i = %d y doy la vuelta",i);


				i=-1;
			}
		}

		if(i+1==list_size(listaWorkersPlanificacion)){
			i=-1;
		}
	}
 log_info(logInfo,"vuelvo a entrar al for");
	for(i=indice+1;i<list_size(listaWorkersPlanificacion)+indice;i++){
			log_info(logInfo,"i:%d",i);
			if(i>=list_size(listaWorkersPlanificacion)){
				log_info(logInfo,"i>=list size. i=0");
				i=0;

			}
			Tplanificacion *aux = list_get(listaWorkersPlanificacion,i);
			log_info(logInfo,"pruebo con %s. availa: %d . necesito %s o %s\n",aux->infoNodo.nombreNodo,aux->availability,nombre1,nombre2);
			if(aux->availability>75){
				puts("ojo availability enorme");
				log_info(logInfo,"availa enorme");
			}
			log_info(logInfo,"if aux(%s) availa > 0. (availa = %d)",aux->infoNodo.nombreNodo,aux->availability);
			if(aux->availability>0){
				log_info(logInfo,"aux(%s) cumplio. ahora si i(%d) != indice(%d) entro",aux->infoNodo.nombreNodo,i,indice);
				if(i!= indice){
					log_info(logInfo,"cumplio. ahora if auxNombreNodo(%s)==nombre1%s || nombre 2 %s entro",aux->infoNodo.nombreNodo,nombre1,nombre2);
					if(string_equals_ignore_case(aux->infoNodo.nombreNodo,nombre1)||string_equals_ignore_case(aux->infoNodo.nombreNodo,nombre2) ){
						log_info(logInfo,"cumplio. devuelvo %s \n",aux->infoNodo.nombreNodo);
						return aux;
					}
				}else{
					log_info(logInfo,"no cumplio. sumo dispo a todos");
					sumarDisponibilidadBaseATodos(listaWorkersPlanificacion);
					log_info(logInfo,"di la vuelta. i: %d, nombre nodo(tendria  qser el apuntado) %s. sumo dispo a todos.\n",i,aux->infoNodo.nombreNodo);
					log_info(logInfo,("if i+1 = list size+indice entro"));
					if(i+1==list_size(listaWorkersPlanificacion)+indice){
						i=indice -1;
						log_info(logInfo,"entre. ahora i = %d",i);
					}log_info(logInfo," no entre. ahora i = %d y doy la vuelta",i);


					//i=indice-1;
				}
			}

			 if(i+1==list_size(listaWorkersPlanificacion)){
				i=indice -1;
			}
		//}*/


	log_info(logInfo,"error getsig c dispo positiva");
	puts("error getsiguientecdispopositiva");
	return NULL;
}

void sumarDisponibilidadBaseATodos(t_list * listaWorkersPlanificacion){
	log_info(logInfo,"en sumar dispo base a todos");
	puts("sumo base");
	int i;
	for(i=0;i<list_size(listaWorkersPlanificacion);i++){

		Tplanificacion *aux = list_get(listaWorkersPlanificacion,i);
		aux->availability+=aux->disponibilidadBase;
		log_info(logInfo,"nueva dispo: %s . ava: %d",aux->infoNodo.nombreNodo,aux->availability);
		log_info(logInfo,"dispo base :%d\n",aux->disponibilidadBase);
		printf("dispo base :%d\n",aux->disponibilidadBase);
		printf("nueva dispo: %s . ava: %d\n",aux->infoNodo.nombreNodo,aux->availability);
	}
}
int posicionarClock(t_list *listaWorkers){
	log_info(logInfo,"posiciono clock");
	int i;
	int disponibilidadMasAlta = -1;
	int indiceAModificar;
	bool empate=false;
	for(i=0;i<list_size(listaWorkers);i++){
		Tplanificacion *aux = list_get(listaWorkers,i);
		log_info(logInfo,"dispo mas alta %d \n",disponibilidadMasAlta);
		log_info(logInfo,"Nodo:%s, ava:%d, clock:%d\n,",aux->infoNodo.nombreNodo,aux->availability,aux->clock);
		if(aux->availability > disponibilidadMasAlta){
			disponibilidadMasAlta=aux->availability;
			log_info(logInfo,"nueva dispo mas alta %d \n",disponibilidadMasAlta);
			log_info(logInfo,"no hay empate");
			empate=false;
			indiceAModificar = i;
		}else if(aux->availability==disponibilidadMasAlta){
			log_info(logInfo,"hay empate");
			empate=true;
		}
	}
	if(empate){
		log_info(logInfo,"hubo empate. desempato");
		desempatarClock(disponibilidadMasAlta,listaWorkers);
	}else{
		Tplanificacion *aux = list_get(listaWorkers,indiceAModificar);
		aux->clock=true;
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
		log_info(logInfo,"Nodo:%s, ava:%d, clock:%d\n,",aux->infoNodo.nombreNodo,aux->availability,aux->clock);
		if(aux->availability == disponibilidadMasAlta){
			historico1=getHistorico(aux);
			log_info(logInfo,"historico %s:%d\n,",aux->infoNodo.nombreNodo,historico1);
		}
		if(historico1>historico2){ //todo cambio (?????????????)
			log_info(logInfo,"historico de %s:(%d) es el mayor. x ahora lo modifico\n,",aux->infoNodo.nombreNodo,historico1);
			historico2=historico1;
			indiceAModificar=i;
		}

	}
	aux=list_get(listaWorkers,indiceAModificar);
	log_info(logInfo,"le pongo el clock a %s, historico %d\n",aux->infoNodo.nombreNodo,historico2);
	aux->clock=true;

	return 0;
}

int getHistorico(Tplanificacion *infoWorker){
	int i;

	for(i=0;i<list_size(listaHistoricaTareas);i++){
		ThistorialTareas *aux = list_get(listaHistoricaTareas,i);
		if(string_equals_ignore_case(infoWorker->infoNodo.nombreNodo,aux->nombreNodo)){

			return aux->tareasRealizadas;
		}
	}

	return 0;
}

int getCargaWorker(char * nombreWorker){
	int i;
	int cargaRet =0;
	for(i=0;i<list_size(listaCargaGlobal);i++){
		TcargaGlobal * cargaWorker = list_get(listaCargaGlobal,i);
		if(string_equals_ignore_case(cargaWorker->nombreNodo,nombreWorker)){
			cargaRet=cargaWorker->cargaGlobal;
		}
	}

	return cargaRet;
}

int getMayorCargaAllWorkers(){

	int mayorCarga=0;
	int i;
	for(i=0;i<list_size(listaCargaGlobal);i++){
		TcargaGlobal * cargaWorker = list_get(listaCargaGlobal,i);
		if(cargaWorker->cargaGlobal>mayorCarga){
			mayorCarga=cargaWorker->cargaGlobal;
		}
	}

	return mayorCarga;

}

void asignarNodoElegido(t_list * listaReduccionGlobal){
	TinfoNodoReduccionGlobal *aux = list_get(listaReduccionGlobal,0);
	char * nodoElegido=aux->nombreNodo;
	int menorCarga =getCargaWorker(nodoElegido);
	int cargaAux;
	int i;
	for(i=0;i<list_size(listaReduccionGlobal);i++){
		aux=list_get(listaReduccionGlobal,i);
		cargaAux=getCargaWorker(aux->nombreNodo);
		if(cargaAux<menorCarga){
			menorCarga=cargaAux;
			nodoElegido=aux->nombreNodo;
		}
	}

	for(i=0;i<list_size(listaReduccionGlobal);i++){
		aux=list_get(listaReduccionGlobal,i);
		if(string_equals_ignore_case(aux->nombreNodo,nodoElegido)){
			aux->nodoEncargado=1;
		}
	}

	log_info(logInfo,"Nodo encargado: %s. menor carga: %d \n",nodoElegido,menorCarga);

	return;
}
int getCargaReduccionGlobal(int job){
	int carga=0;
	int i;
	TpackTablaEstados * estado;
	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
		estado = list_get(listaEstadoFinalizadoOK,i);
		if(estado->etapa==REDUCCIONLOCAL){
			carga++;
		}
	}

	log_info(logInfo,"Libero carga en %d\n",divideYRedondea(carga,2));
	return divideYRedondea(carga,2);
}


bool yaFueAgregadoAlistaJobFinalizados(int idTareaFinalizada){
	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	int i;

	for(i=0;i<list_size(listaJobFinalizados);i++){
		TjobFinalizado *job=list_get(listaJobFinalizados,i);
		if(job->nroJob==tareaFinalizada->job){

			return true;
		}
	}

	return false;
}


void liberarCargaNodos(int idTareaFinalizada){




	TpackTablaEstados *tareaFinalizada=getTareaPorId(idTareaFinalizada);
	int jobALiberar = tareaFinalizada->job;

	//primero me fijo q no haya sido agregado a la lista de job finalizados, en ese caso ya se liberaron las cargas

	if(!yaFueFinalizadoPorErrorDeReplanificacion(jobALiberar)){


		//reduzco de todas las etpas q estan en proceso.
		int i;


		for(i=0;i<list_size(listaEstadoEnProceso);i++){
			TpackTablaEstados *aux  = list_get(listaEstadoEnProceso,i);
			if(aux->idTarea==jobALiberar){
				if(aux->etapa==TRANSFORMACION){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONLOCAL){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONGLOBAL){
					int cargaAReducir = getCargaReduccionGlobal(jobALiberar);
					liberarCargaEn(aux->nodo,cargaAReducir);
				}
			}
		}
		for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
			TpackTablaEstados *aux  = list_get(listaEstadoFinalizadoOK,i);
			if(aux->idTarea==jobALiberar){
				if(aux->etapa==TRANSFORMACION){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONLOCAL){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONGLOBAL){
					int cargaAReducir = getCargaReduccionGlobal(jobALiberar);
					liberarCargaEn(aux->nodo,cargaAReducir);

				}
			}
		}
		for(i=0;i<list_size(listaEstadoError);i++){
			TpackTablaEstados *aux  = list_get(listaEstadoError,i);
			if(aux->idTarea==jobALiberar){
				if(aux->etapa==TRANSFORMACION){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONLOCAL){
					liberarCargaEn(aux->nodo,1);
				}else if(aux->etapa==REDUCCIONGLOBAL){
					int cargaAReducir = getCargaReduccionGlobal(jobALiberar);
					liberarCargaEn(aux->nodo,cargaAReducir);
				}
			}
		}

		mostrarTablaDeEstados();


	}

}
void liberarCargaEn(char * nombreNodo,int cantidad){
	int i;
	for(i=0;i<list_size(listaCargaGlobal);i++){
		TcargaGlobal *cargaNodo = list_get(listaCargaGlobal,i);
		cargaNodo->cargaGlobal-=cantidad;
		break;
	}
}

void agregarReduccionGlobalAListaEnProceso(TreduccionGlobal *infoReduccion,char * bloquesReducidos,TjobMaster *job){

		TpackTablaEstados * estado = malloc(sizeof (TpackTablaEstados));
		estado->idTarea=infoReduccion->idTarea;
		estado->job=infoReduccion->job;
		estado->master=job->masterId;
		estado->bloqueDelArchivo=-1;
		estado->nodo=malloc(TAMANIO_NOMBRE_NODO);
		estado->nodo=getNodoElegido(infoReduccion->listaNodos);
		estado->etapa=REDUCCIONGLOBAL;
		estado->nombreArchTemporal=infoReduccion->tempRedGlobal;
		estado->bloquesReducidos=malloc(strlen(bloquesReducidos)+1);
		estado->bloquesReducidos=bloquesReducidos;
		list_add(listaEstadoEnProceso,estado);

		mostrarTablaDeEstados();
}
bool esNodoEncargado(char * nombreNodo){
	return false;
}
char * getNodoElegido(t_list * listaNodos){
	int i;
	for(i=0;i<list_size(listaNodos);i++){
		TinfoNodoReduccionGlobal *nodo =list_get(listaNodos,i);
		if(nodo->nodoEncargado==1){
			return nodo->nombreNodo;
		}
	}
	puts("error get nodo elegido");
	log_info(logInfo,"error get nodo elegido");
	return "null";
}

bool yaFueFinalizadoPorErrorDeReplanificacion(int job){

	int i;
	for(i=0;i<list_size(listaJobFinalizados);i++){
		TjobFinalizado *jobAux = list_get(listaJobFinalizados,i);
		if(jobAux->nroJob == job){


			return true;
		}
	}


	return false;
}

int moverAListaFinalizadosOK(int idTareaFinalizada){

	int i;
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados *aux  = list_get(listaEstadoEnProceso,i);
		if(aux->idTarea==idTareaFinalizada){
			list_add(listaEstadoFinalizadoOK,aux);
			list_remove(listaEstadoEnProceso,i);
			mostrarTablaDeEstados();
			return 0;
		}
	}
	mostrarTablaDeEstados();
	puts("error mover a finalizados ok");
	log_info(logInfo,"error mover a fina ok");
	return -1;
}

int moverAListaError (int idTareaFinalizada){

	int i;
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados *aux  = list_get(listaEstadoEnProceso,i);
		if(aux->idTarea==idTareaFinalizada){
			list_add(listaEstadoError,aux);
			list_remove(listaEstadoEnProceso,i);
			mostrarTablaDeEstados();
			return 0;
		}
	}

	mostrarTablaDeEstados();
	puts("error movera lista error");
	log_info(logInfo,"error mover a lista error");
	return -1;
}


int moverFinalizadaAListaError (int idTareaFinalizada){

	int i;
	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
		TpackTablaEstados *aux  = list_get(listaEstadoFinalizadoOK,i);
		if(aux->idTarea==idTareaFinalizada){
			list_add(listaEstadoError,aux);
			list_remove(listaEstadoFinalizadoOK,i);
			mostrarTablaDeEstados();
			return 0;
		}
	}

	mostrarTablaDeEstados();
	puts("error mover finalizada lista error");
	log_info(logInfo,"error mover a lista error");
	return -1;
}

void agregarReduccionLocalAListaEnProceso(TreduccionLocal * infoReduccion,char * bloquesReducidos,TjobMaster *job){


	TpackTablaEstados * estado = malloc(sizeof (TpackTablaEstados));
	estado->idTarea=infoReduccion->idTarea;
	estado->job=infoReduccion->job;
	estado->master=job->masterId;
	estado->bloqueDelArchivo=-1;
	estado->nodo=malloc(TAMANIO_NOMBRE_NODO);
	estado->nodo=infoReduccion->nombreNodo;
	estado->etapa=REDUCCIONLOCAL;
	estado->nombreArchTemporal=infoReduccion->tempRed;
	estado->bloquesReducidos=malloc(strlen(bloquesReducidos)+1);
	estado->bloquesReducidos=bloquesReducidos;
	list_add(listaEstadoEnProceso,estado);

	mostrarTablaDeEstados();
}

void agregarTransformacionAListaEnProceso(TjobMaster *job, TpackInfoBloque *bloque,bool mostrarTabla){


	TpackTablaEstados * estado = malloc(sizeof (TpackTablaEstados));
	estado->idTarea=bloque->idTarea;
	estado->job=job->nroJob;
	estado->master= job->masterId;
	estado->nodo=malloc(TAMANIO_NOMBRE_NODO);
	estado->nodo=bloque->nombreNodo;
	estado->bloqueDelArchivo=bloque->bloqueDelArchivo;
	estado->etapa=TRANSFORMACION;
	estado->nombreArchTemporal=malloc(TAMANIO_NOMBRE_TEMPORAL);
	estado->nombreArchTemporal=bloque->nombreTemporal;
	estado->fueReplanificada=false;
	list_add(listaEstadoEnProceso,estado);

	//mostrarTablaDeEstados();

}

void mostrarTablaDeEstados(){
	printf("\n\n\n");
	log_info(logInfo,"\n\n\n");
	puts("Hubo un cambio de estado, va la tabla actual:");
	log_info(logInfo,"Hubo un cambio de estado, va la tabla actual:");
	int i;
	int id,job,master;
	char * bloque;
	char * etapa;
	char * nodo;
	char * archivoTemporal;
	printf("%-4s%-4s%-7s%-7s%-25s%-15s%-40s%-10s\n", "id", "Job", "Master", "Nodo", "Bloque","Etapa","A Temporal","Estado");
	log_info(logInfo,"%-4s%-4s%-7s%-7s%-25s%-15s%-40s%-10s", "id", "Job", "Master", "Nodo", "Bloque","Etapa","A Temporal","Estado");
	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados * aux = list_get(listaEstadoEnProceso,i);
		id = aux->idTarea;
		job=aux->job;
		master = aux->master;
		if(aux->bloqueDelArchivo != -1){
			bloque=string_itoa(aux->bloqueDelArchivo);
		}else{
			bloque = aux->bloquesReducidos;
		}


		etapa=string_new();
		if(aux->etapa==TRANSFORMACION){
			string_append(&etapa,"TR");
		}else if(aux->etapa==REDUCCIONGLOBAL){
			string_append(&etapa,"RGL");
		}else if(aux->etapa==REDUCCIONLOCAL){
			string_append(&etapa,"RL");
		}else if(aux->etapa==ALMACENAMIENTOFINAL){
			string_append(&etapa,"A F");
		}else{
			string_append(&etapa,"");
		}



		//etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"E Pr");
		log_info(logInfo,"%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s",id,job,master,nodo,bloque,etapa,archivoTemporal,"E Pr");
		log_info(logInfo,"1asd");
		free(etapa);
		log_info(logInfo,"2");
		//free(bloque);
		//log_info(logInfo,"3");
		//free(nodo);
		//log_info(logInfo,"4");
		//free(archivoTemporal);
		//log_info(logInfo,"5");
	}

	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
		TpackTablaEstados * aux = list_get(listaEstadoFinalizadoOK,i);
		id = aux->idTarea;
		job=aux->job;
		master = aux->master;
		if(aux->bloqueDelArchivo != -1){
			bloque=string_itoa(aux->bloqueDelArchivo);
		}else{
			bloque = aux->bloquesReducidos;
		}

		etapa=string_new();
		if(aux->etapa==TRANSFORMACION){
			string_append(&etapa,"TR");
		}else if(aux->etapa==REDUCCIONGLOBAL){
			string_append(&etapa,"RGL");
		}else if(aux->etapa==REDUCCIONLOCAL){
			string_append(&etapa,"RL");
		}else if(aux->etapa==ALMACENAMIENTOFINAL){
			string_append(&etapa,"A F");
		}else{
			string_append(&etapa,"");
		}


		//etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"FinOK");
		log_info(logInfo,"%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s",id,job,master,nodo,bloque,etapa,archivoTemporal,"FinOK");

		log_info(logInfo,"6");
		free(etapa);
		log_info(logInfo,"7");
		//free(bloque);
		//log_info(logInfo,"8");
		//free(nodo);
		//log_info(logInfo,"9");
		//free(archivoTemporal);
		//log_info(logInfo,"10");
	}

	for(i=0;i<list_size(listaEstadoError);i++){
		TpackTablaEstados * aux = list_get(listaEstadoError,i);
		id = aux->idTarea;
		job=aux->job;
		master = aux->master;
		if(aux->bloqueDelArchivo != -1){
			bloque=string_itoa(aux->bloqueDelArchivo);
		}else{
			bloque = aux->bloquesReducidos;
		}
		etapa=string_new();
		if(aux->etapa==TRANSFORMACION){
			string_append(&etapa,"TR");
		}else if(aux->etapa==REDUCCIONGLOBAL){
			string_append(&etapa,"RGL");
		}else if(aux->etapa==REDUCCIONLOCAL){
			string_append(&etapa,"RL");
		}else if(aux->etapa==ALMACENAMIENTOFINAL){
			string_append(&etapa,"A F");
		}else{
			string_append(&etapa,"");
		}
		//etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"ERR");
		log_info(logInfo,"%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s",id,job,master,nodo,bloque,etapa,archivoTemporal,"ERR");

		log_info(logInfo,"11");
		free(etapa);
		log_info(logInfo,"12");
		//free(bloque);
		//log_info(logInfo,"13");
		//free(nodo);
		//log_info(logInfo,"14");
		//free(archivoTemporal);
		//log_info(logInfo,"15");
	}

	printf("\n\n\n");
	log_info(logInfo,"\n\n\n");

}

char  * getNombreEtapa(int etapaEnum){
	char * ret=malloc(6);
	if(etapaEnum==TRANSFORMACION){
		ret="Tra";
	}else if(etapaEnum==REDUCCIONGLOBAL){
		ret="R Gl";
	}else if(etapaEnum==REDUCCIONLOCAL){
		ret="R Lo";
	}else if(etapaEnum==ALMACENAMIENTOFINAL){
			ret="A F";
	}else{
		ret = " ";
	}
	return ret;
}




char * getIpNodo(char * nombreNodo,TjobMaster *job){
	TpackageInfoNodo *nodoAux;
	int i;
	for(i=0;i<list_size(job->listaNodosArchivo);i++){
		nodoAux=list_get(job->listaNodosArchivo,i);
		if(string_equals_ignore_case(nodoAux->nombreNodo, nombreNodo)){
			return nodoAux->ipNodo;
		}
	}
	puts("error get ip nodo");
	log_info(logInfo,"error get ip nodo");
	return NULL;
}

char * getPuertoNodo(char * nombreNodo,TjobMaster *job){
	TpackageInfoNodo *nodoAux;
	int i;
	for(i=0;i<list_size(job->listaNodosArchivo);i++){
		nodoAux=list_get(job->listaNodosArchivo,i);
		if(string_equals_ignore_case(nodoAux->nombreNodo, nombreNodo)){
			return nodoAux->puertoWorker;
		}
	}
	puts("error get puerto nodo");
	log_info(logInfo,"error get puerto nodo");
	return NULL;
}



TpackTablaEstados * getTareaPorId(int idTarea){
	int i;

	for(i=0;i<list_size(listaEstadoEnProceso);i++){
		TpackTablaEstados * aux = list_get(listaEstadoEnProceso,i);
		if(aux->idTarea==idTarea){

			return aux;
		}
	}



	for(i=0;i<list_size(listaEstadoError);i++){
			TpackTablaEstados * aux = list_get(listaEstadoError,i);
			if(aux->idTarea==idTarea){

				return aux;
			}
		}



	for(i=0;i<list_size(listaEstadoFinalizadoOK);i++){
			TpackTablaEstados * aux = list_get(listaEstadoFinalizadoOK,i);
			if(aux->idTarea==idTarea){

				return aux;
			}
		}

	printf("no se encontro %d en la tbala de estados\n",idTarea);
	log_info(logInfo,"error get tarea x id");
	return NULL;
}

bool sePuedeReplanificar(int idTarea,t_list * listaComposicionArchivo){
	TpackTablaEstados *tareaAReplanificar=getTareaPorId(idTarea);
	int i;
	log_info(logInfo,"en se puede replanificar");
	if(tareaAReplanificar==NULL){
		log_info(logInfo,"La tarea no existe en la tabla de estados");
		return false;
	}
	if(tareaAReplanificar->etapa==REDUCCIONGLOBAL || tareaAReplanificar->etapa==REDUCCIONLOCAL||tareaAReplanificar->etapa==ALMACENAMIENTOFINAL){
		log_info(logInfo,"No existe posibilidad de replanificar tarea ya que se encontraba en etapa de Reduccion/AlmacenamientoFinal");
		return false;
	}
	if(tareaAReplanificar->fueReplanificada){
		log_info(logInfo,"No se poseen nodos con copias de los datos. no se puede replanificar");
		return false;
	}
	for(i=0;i<list_size(listaComposicionArchivo);i++){
		TpackageUbicacionBloques * bloqueAux =list_get(listaComposicionArchivo,i);
		if(tareaAReplanificar->bloqueDelArchivo == bloqueAux->bloque){
			return true;
		}
	}
	log_info(logInfo,"no se puede replanificar");
	return false;
}
int replanificar(int idTarea, int sockMaster,t_list * listaComposicionArchivo){
	log_info(logInfo,"en replanificar");
	sleep(retardoPlanificacionSegs);
	TpackTablaEstados *tareaAReplanificar=getTareaPorId(idTarea);
	TjobMaster *job = getJobPorNroJob(tareaAReplanificar->job);
	int i,packSize,stat;
	char * buffer;

	if(tareaAReplanificar==NULL){
		puts("error tarea a replanificar ==NULL");
		log_info(logInfo,"La tarea no existe en la tabla de estados");
			return FALLO_GRAL;
	}

	for(i=0;i<list_size(listaComposicionArchivo);i++){
		TpackageUbicacionBloques * bloqueAux =list_get(listaComposicionArchivo,i);
		if(tareaAReplanificar->bloqueDelArchivo == bloqueAux->bloque){
			TpackInfoBloque *bloqueRet=malloc(sizeof(TpackInfoBloque));
			if(string_equals_ignore_case(tareaAReplanificar->nodo, bloqueAux->nombreNodoC1)){
				//le paso la info para q labure en el otronodo

				bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC2;

				bloqueRet->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
				bloqueRet->nombreNodo=bloqueAux->nombreNodoC2;
				bloqueRet->tamanioNombre=strlen(bloqueRet->nombreNodo)+1;

				bloqueRet->puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
				bloqueRet->puertoWorker=getPuertoNodo(bloqueRet->nombreNodo,job);
				bloqueRet->tamanioPuerto=strlen(bloqueRet->puertoWorker)+1;
				bloqueRet->ipWorker=malloc(MAXIMA_LONGITUD_IP);
				bloqueRet->ipWorker=getIpNodo(bloqueRet->nombreNodo,job);
				bloqueRet->tamanioIp=strlen(bloqueRet->ipWorker)+1;

			}else{
				//le paso la info para q trabaje en el otronodo
				bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC1;

				bloqueRet->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
				bloqueRet->nombreNodo=bloqueAux->nombreNodoC1;
				bloqueRet->tamanioNombre=strlen(bloqueRet->nombreNodo)+1;

				bloqueRet->puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
				bloqueRet->puertoWorker=getPuertoNodo(bloqueRet->nombreNodo,job);
				bloqueRet->tamanioPuerto=strlen(bloqueRet->puertoWorker)+1;

				bloqueRet->ipWorker=malloc(MAXIMA_LONGITUD_IP);
				bloqueRet->ipWorker=getIpNodo(bloqueRet->nombreNodo,job);
				bloqueRet->tamanioIp=strlen(bloqueRet->ipWorker)+1;

			}

			bloqueRet->bloqueDelArchivo=tareaAReplanificar->bloqueDelArchivo;
			bloqueRet->bytesOcupados=bloqueAux->finBloque;

			bloqueRet->idTarea=idTareaGlobal++;

			bloqueRet->nombreTemporal=malloc(TAMANIO_NOMBRE_TEMPORAL);
			bloqueRet->nombreTemporal=generarNombreTemporal(job->masterId);
			bloqueRet->nombreTemporalLen=strlen(bloqueRet->nombreTemporal)+1;

			tareaAReplanificar->fueReplanificada=true;

			log_info(logInfo,"replanifique la tarea %d. ahora es la %d. job %d, nuevo nodo: %s, nuevo arch temp %s",idTarea,bloqueRet->idTarea,job->nroJob,bloqueRet->nombreNodo,bloqueRet->nombreTemporal);

			Theader head;
			head.tipo_de_proceso=YAMA;
			head.tipo_de_mensaje=INFOBLOQUEREPLANIFICADO;
			packSize=0;
			buffer=serializeInfoBloque(head,bloqueRet,&packSize);

			log_info(logInfo,"Info del bloque %d serializado, enviamos\n",bloqueRet->bloqueDelArchivo);

			if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
				puts("no se pudo enviar info del bloque. ");
				return  FALLO_SEND;
			}
			log_info(logInfo,"se enviaron %d bytes de la info del bloque\n",stat);
			bool mostrarTabla=true;
			agregarTransformacionAListaEnProceso(job,bloqueRet,mostrarTabla);
			actualizarCargaWorkerEn(tareaAReplanificar->nodo,1);
			aumentarHistoricoEn(bloqueRet->nombreNodo,1);
			free(buffer);
			return 0;
		}
	}

	log_info(logInfo,"fallo gral en planificar");
	puts("fallo gral en planificar");
	return FALLO_GRAL;

}


char *  generarNombreTemporal(int idMaster){

	log_info(logInfo,"en gnrar nombre temp");
	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append_with_format(&temp,"%d",idMaster);
	//string_append(&temp,string_itoa(idMaster));
	string_append(&temp,"-temp");
	string_append_with_format(&temp,"%d",idTempName++);
	//string_append(&temp,string_itoa(idTempName++));


	return temp;
}

char *  generarNombreReductorTemporal(char * nombreNodo,int idMaster){
	log_info(logInfo,"en generar nombre red temp");
	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append_with_format(&temp,"%d",idMaster);
	//string_append(&temp,string_itoa(idMaster));
	string_append(&temp,"-");
	string_append(&temp,nombreNodo);


	return temp;
}


char *  generarNombreReduccionGlobalTemporal(int idMaster){
	log_info(logInfo,"en generar nombre rg temp");
	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append_with_format(&temp,"%d",idMaster);
	//string_append(&temp,string_itoa(idMaster));
	string_append(&temp,"-final");



	return temp;
}








void generarListaComposicionArchivoHardcode(t_list * listaComposicion){

	//toda esta funcion va a volar, esta hecho asi nomas para ir probandolo. esta tod hardco
	int maxNombreNodo=10;
	TpackageUbicacionBloques *bloque0 = malloc(sizeof(TpackageUbicacionBloques));
	bloque0->bloque=0;
	bloque0->nombreNodoC1=malloc(sizeof(maxNombreNodo));
	bloque0->nombreNodoC1="Nodo1";
	bloque0->nombreNodoC1Len=strlen(bloque0->nombreNodoC1)+1;
	bloque0->bloqueC1=0;
	bloque0->nombreNodoC2=malloc(sizeof(maxNombreNodo));
	bloque0->nombreNodoC2="Nodo2";
	bloque0->nombreNodoC2Len=strlen(bloque0->nombreNodoC2)+1;
	bloque0->bloqueC2=0;
	bloque0->finBloque=1048576;

	list_add(listaComposicion,bloque0);

	TpackageUbicacionBloques *bloque1 = malloc(sizeof(TpackageUbicacionBloques));
	bloque1->bloque=1;
	bloque1->nombreNodoC1=malloc(sizeof(maxNombreNodo));
	bloque1->nombreNodoC1="Nodo2";
	bloque1->nombreNodoC1Len=strlen(bloque1->nombreNodoC1)+1;
	bloque1->bloqueC1=0;
	bloque1->nombreNodoC2=malloc(sizeof(maxNombreNodo));
	bloque1->nombreNodoC2="Nodo3";
	bloque1->nombreNodoC2Len=strlen(bloque1->nombreNodoC2)+1;
	bloque1->bloqueC2=0;
	bloque1->finBloque=1048500;
	list_add(listaComposicion,bloque1);

	TpackageUbicacionBloques *bloque2 = malloc(sizeof(TpackageUbicacionBloques));
	bloque2->bloque=2;
	bloque2->nombreNodoC1=malloc(sizeof(maxNombreNodo));
	bloque2->nombreNodoC1="Nodo3";
	bloque2->nombreNodoC1Len=strlen(bloque2->nombreNodoC1)+1;
	bloque2->bloqueC1=0;
	bloque2->nombreNodoC2=malloc(sizeof(maxNombreNodo));
	bloque2->nombreNodoC2="Nodo1";
	bloque2->nombreNodoC2Len=strlen(bloque2->nombreNodoC2)+1;
	bloque2->bloqueC2=0;
	bloque2->finBloque=1048516;
	list_add(listaComposicion,bloque2);

}

