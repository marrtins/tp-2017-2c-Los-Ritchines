/*
 * planificacionYM.c
 *
 *  Created on: 17/10/2017
 *      Author: utnso
 */
#include "funcionesYM.h"

extern t_list * listaHistoricaTareas,*listaCargaGlobal;
extern pthread_mutex_t mux_listaHistorica,mux_listaCargaGlobal;

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

	if((stat = posicionarClock(listaWorkersPlanificacion))<0){
		puts("error");
	}
	int k;
	for(k=0;k<list_size(listaComposicionArchivo);k++){
		TpackageUbicacionBloques *bloqueAux=list_get(listaComposicionArchivo,k);

		TpackInfoBloque *bloque = asignarBloque(bloqueAux,listaWorkersPlanificacion);

		list_add(listaPlanificada,bloque);
		actualizarCargaWorkerEn(bloque->nombreNodo,1);

	}

	int q;
	for(q=0;q<list_size(listaPlanificada);q++){
		TpackInfoBloque *aux = list_get(listaPlanificada,q);
		printf("%d %d %s\n",aux->bloque,aux->bytesOcupados,aux->nombreNodo);
	}


	return listaPlanificada;
}

void actualizarCargaWorkerEn(char * nombreNodo, int cantidadAAumentar){

	int i;
	for(i=0;i<list_size(listaCargaGlobal);i++){
		TcargaGlobal *aux = list_get(listaCargaGlobal,i);
		if(aux->nombreNodo == nombreNodo){
			aux->cargaGlobal +=cantidadAAumentar;

			return;
		}
	}

	//todo mutex
}
void aumentarHistoricoEn(char * nombreNodo,int cantidadAAumentar){
	int i;

	for(i=0;i<list_size(listaHistoricaTareas);i++){
		ThistorialTareas *historial = list_get(listaHistoricaTareas,i);
		if(historial->nombreNodo == nombreNodo){
			historial->tareasRealizadas+=cantidadAAumentar;

			return;
		}
//todo mutex
	}

}

TpackInfoBloque * asignarBloque(TpackageUbicacionBloques *bloqueAux,t_list *listaWorkersPlanificacion){


	Tplanificacion *nodoApuntado = getNodoApuntado(listaWorkersPlanificacion);
	TpackInfoBloque *bloqueRet=malloc(sizeof(bloqueRet));

	if((nodoApuntado->availability > 0 && bloqueAux->nombreNodoC1==nodoApuntado->infoNodo.nombreNodo) ||
			(nodoApuntado->availability > 0 && bloqueAux->nombreNodoC2==nodoApuntado->infoNodo.nombreNodo)){
		if(bloqueAux->nombreNodoC1==nodoApuntado->infoNodo.nombreNodo ){
			bloqueRet->bloque=bloqueAux->bloqueC1;
		}
		if(bloqueAux->nombreNodoC2==nodoApuntado->infoNodo.nombreNodo ){
			bloqueRet->bloque=bloqueAux->bloqueC2;
		}
		mergeBloque(bloqueRet,nodoApuntado,bloqueAux);
		nodoApuntado->availability-=1;
		aumentarHistoricoEn(bloqueRet->nombreNodo,1);
		avanzarClock(listaWorkersPlanificacion);

	}else{
		Tplanificacion *siguienteNodo = getSiguienteNodoDisponible(listaWorkersPlanificacion,bloqueAux->nombreNodoC1,bloqueAux->nombreNodoC2);

		if(siguienteNodo->infoNodo.nombreNodo == bloqueAux->nombreNodoC1){
			bloqueRet->bloque=bloqueAux->bloqueC1;
		}else{
			bloqueRet->bloque=bloqueAux->bloqueC2;
		}

		mergeBloque(bloqueRet,siguienteNodo,bloqueAux);
		siguienteNodo->availability-=1;
		aumentarHistoricoEn(bloqueRet->nombreNodo,1);
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
	bloqueRet->ipWorker=malloc(MAXIMA_LONGITUD_IP);
	bloqueRet->ipWorker=nodoApuntado->infoNodo.ipNodo;
	bloqueRet->ipLen=nodoApuntado->infoNodo.ipLen;
	bloqueRet->puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
	bloqueRet->puertoWorker=nodoApuntado->infoNodo.puertoWorker;
	bloqueRet->puertoLen=nodoApuntado->infoNodo.puertoLen;

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
Tplanificacion * getSiguienteNodoDisponible(t_list * listaWorkersPlanificacion,char * nombreNodo1,char* nombreNodo2){
	//todo chequeo errores
	int i;

	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux= list_get(listaWorkersPlanificacion,i);
		if(aux->clock){
			Tplanificacion *siguienteDisponible = getSiguienteConDisponibilidadPositivaPosible(listaWorkersPlanificacion,i,nombreNodo1,nombreNodo2);
			return siguienteDisponible;

		}
	}


	return NULL;
}


Tplanificacion * getSiguienteConDisponibilidadPositivaPosible(t_list * listaWorkersPlanificacion, int indice,char * nombre1,char * nombre2){

	int i;
	for(i=indice+1;i<list_size(listaWorkersPlanificacion)+indice;i++){
		if(i>=list_size(listaWorkersPlanificacion)){
			i=0;
		}
		Tplanificacion *aux = list_get(listaWorkersPlanificacion,i);
		if(aux->availability>0){
			if(i!= indice){
				if(aux->infoNodo.nombreNodo == nombre1 ||aux->infoNodo.nombreNodo == nombre2 ){
					return aux;
				}
			}else{
				sumarDisponibilidadBaseATodos(listaWorkersPlanificacion);
				i=indice+1;
			}
		}
	}

	return NULL;
}

void sumarDisponibilidadBaseATodos(t_list * listaWorkersPlanificacion){
	int i;
	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux = list_get(listaWorkersPlanificacion,i);
		aux->availability+=aux->disponibilidadBase;
	}
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
		if(historico1>historico2){ //todo cambio
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

	for(i=0;i<list_size(listaHistoricaTareas);i++){
		ThistorialTareas *aux = list_get(listaHistoricaTareas,i);
		if(infoWorker->infoNodo.nombreNodo==aux->nombreNodo){

			return aux->tareasRealizadas;
		}
	}//todo mutex

	return 0;
}
