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
		printf("Nodo:%s, ava:%d, clock:%d\n,",nodo->infoNodo.nombreNodo,nodo->availability,nodo->clock);
		list_add(listaWorkersPlanificacion,nodo);
	}

	if((stat = posicionarClock(listaWorkersPlanificacion))<0){
		puts("error");
	}
	int k;
	for(k=0;k<list_size(job->listaComposicionArchivo);k++){
		TpackageUbicacionBloques *bloqueAux=list_get(job->listaComposicionArchivo,k);
		//printf("asigno al bloque del archivo %d\n",bloqueAux->bloque);
		TpackInfoBloque *bloque = asignarBloque(bloqueAux,listaWorkersPlanificacion,job);
		printf("asigne al bloque %d, el nodo %s. bloque databin %d \n",bloque->bloqueDelArchivo,bloque->nombreNodo,bloque->bloqueDelDatabin);
		list_add(listaPlanificada,bloque);


	}

	int q;
	for(q=0;q<list_size(listaPlanificada);q++){
		TpackInfoBloque *aux = list_get(listaPlanificada,q);
		printf("b arch %d  bdata %d %d %s\n",aux->bloqueDelArchivo,aux->bloqueDelDatabin,aux->bytesOcupados,aux->nombreNodo);
	}


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
	puts("muestro tabla historica");
	int i;
	for(i=0;i<list_size(listaHistoricaTareas);i++){
			ThistorialTareas *historial = list_get(listaHistoricaTareas,i);
			printf("%s ; historico: %d\n",historial->nombreNodo,historial->tareasRealizadas);
	}
}

void mostrarTablaCargas(){
	int i;
	puts("muestro tabla cargas");
	for(i=0;i<list_size(listaCargaGlobal);i++){
			TcargaGlobal *historial = list_get(listaCargaGlobal,i);
			printf("%s ; carga: %d\n",historial->nombreNodo,historial->cargaGlobal);
	}
}

TpackInfoBloque * asignarBloque(TpackageUbicacionBloques *bloqueAux,t_list *listaWorkersPlanificacion,TjobMaster *job){


	Tplanificacion *nodoApuntado = getNodoApuntado(listaWorkersPlanificacion);
	TpackInfoBloque *bloqueRet=malloc(sizeof(TpackInfoBloque));

	//puts("en asignar bloque");
	printf("copias del bloque %d: %s y %s \n",bloqueAux->bloque,bloqueAux->nombreNodoC1,bloqueAux->nombreNodoC2);
	printf("nodo apuntado %s. availability %d\n",nodoApuntado->infoNodo.nombreNodo,nodoApuntado->availability);
	if((nodoApuntado->availability > 0 && string_equals_ignore_case(bloqueAux->nombreNodoC1,nodoApuntado->infoNodo.nombreNodo)) ||
			(nodoApuntado->availability > 0 && string_equals_ignore_case(bloqueAux->nombreNodoC2,nodoApuntado->infoNodo.nombreNodo))){
		if(string_equals_ignore_case(bloqueAux->nombreNodoC1,nodoApuntado->infoNodo.nombreNodo) ){
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC1;
			printf("asigno al bloque del archivo %d, el bloque adtabin: %d del %s\n",bloqueAux->bloque,bloqueRet->bloqueDelDatabin,nodoApuntado->infoNodo.nombreNodo);
		}
		if(string_equals_ignore_case(bloqueAux->nombreNodoC2,nodoApuntado->infoNodo.nombreNodo) ){
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC2;
			printf("asigno al bloque del archivo %d, el bloque adtabin: %d del %s\n",bloqueAux->bloque,bloqueRet->bloqueDelDatabin,nodoApuntado->infoNodo.nombreNodo);
		}

		mergeBloque(bloqueRet,nodoApuntado,bloqueAux,job->masterId);
		nodoApuntado->availability-=1;
	//	printf("cumplio. le asigno el bloque %d al nodo apuntado %s. bloque databn %d\n",bloqueRet->bloqueDelArchivo,bloqueRet->nombreNodo,bloqueRet->bloqueDelDatabin);
		avanzarClock(listaWorkersPlanificacion);

	}else{
		puts("no cumplio. busco el siguiente nodo");
		Tplanificacion *siguienteNodo = getSiguienteNodoDisponible(listaWorkersPlanificacion,bloqueAux->nombreNodoC1,bloqueAux->nombreNodoC2);
		printf("consegui el siguiente dispo. el nodo %s\n",siguienteNodo->infoNodo.nombreNodo);
		if(string_equals_ignore_case(siguienteNodo->infoNodo.nombreNodo,bloqueAux->nombreNodoC1)){
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC1;
			printf("asigno al bloque del archivo %d, el bloque adtabin: %d del %s\n",bloqueAux->bloque,bloqueRet->bloqueDelDatabin,siguienteNodo->infoNodo.nombreNodo);
		}else{
			bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC2;
			printf("asigno al bloque del archivo %d, el bloque adtabin: %d del %s\n",bloqueAux->bloque,bloqueRet->bloqueDelDatabin,siguienteNodo->infoNodo.nombreNodo);
		}

		mergeBloque(bloqueRet,siguienteNodo,bloqueAux,job->masterId);
		siguienteNodo->availability-=1;
		//printf("le asigno el bloque %d al nodo q recien consegui %s, bloque databind :%d \n",bloqueRet->bloqueDelArchivo,siguienteNodo->infoNodo.nombreNodo,bloqueRet->bloqueDelDatabin);
	}
	aumentarHistoricoEn(bloqueRet->nombreNodo,1);
	actualizarCargaWorkerEn(bloqueRet->nombreNodo,1);
	//printf("aumento carga e historico y asi queda mi tabla de nodos\n");
	int i;
	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux=list_get(listaWorkersPlanificacion,i);
		printf("nombrenodo %s. ava %d clock %d \n",aux->infoNodo.nombreNodo,aux->availability,aux->clock);
	}

	return bloqueRet;
}


void avanzarClock(t_list *listaWorkersPlanificacion){
	int i;
	puts("avanzo clock");
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
			printf("ahora %s tiene false y %s tiene true el clock\n",aux->infoNodo.nombreNodo,siguiente->infoNodo.nombreNodo);
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
	return NULL;
}
Tplanificacion * getSiguienteNodoDisponible(t_list * listaWorkersPlanificacion,char * nombreNodo1,char* nombreNodo2){
	//todo chequeo errores
	int i;

	for(i=0;i<list_size(listaWorkersPlanificacion);i++){
		Tplanificacion *aux= list_get(listaWorkersPlanificacion,i);
		if(aux->clock){
			printf("busco el siguiente al apuntado actualmente que es %s\n",aux->infoNodo.nombreNodo);
			Tplanificacion *siguienteDisponible = getSiguienteConDisponibilidadPositivaPosible(listaWorkersPlanificacion,i,nombreNodo1,nombreNodo2);
			return siguienteDisponible;

		}
	}


	return NULL;
}


Tplanificacion * getSiguienteConDisponibilidadPositivaPosible(t_list * listaWorkersPlanificacion, int indice,char * nombre1,char * nombre2){
//todo
	puts("en get sig c dispo positiva posibla");
	int i;
	for(i=indice+1;i<list_size(listaWorkersPlanificacion)+indice;i++){
		if(i>=list_size(listaWorkersPlanificacion)){
			i=0;
		}
		Tplanificacion *aux = list_get(listaWorkersPlanificacion,i);
		printf("pruebo con %s. availa: %d . necesito %s o %s\n",aux->infoNodo.nombreNodo,aux->availability,nombre1,nombre2);
		if(aux->availability>0){
			if(i!= indice){
				if(string_equals_ignore_case(aux->infoNodo.nombreNodo,nombre1)||string_equals_ignore_case(aux->infoNodo.nombreNodo,nombre2) ){
					printf("cumplio. devuelvo %s \n",aux->infoNodo.nombreNodo);
					return aux;
				}
			}else{
				sumarDisponibilidadBaseATodos(listaWorkersPlanificacion);
				printf("di la vuelta. i: %d, nombre nodo(tendria  qser el apuntado) %s. sumo dispo a todos.\n",i,aux->infoNodo.nombreNodo);
				i=indice;
			}
		}

		if(i+1==list_size(listaWorkersPlanificacion)){
			i=indice -1;
		}
	}



	puts("error getsiguientecdispopositiva");
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
	puts("posiciono clock");
	int i;
	int disponibilidadMasAlta = -1;
	bool empate=false;
	for(i=0;i<list_size(listaWorkers);i++){
		Tplanificacion *aux = list_get(listaWorkers,i);
		printf("dispo mas alta %d \n",disponibilidadMasAlta);
		printf("Nodo:%s, ava:%d, clock:%d\n,",aux->infoNodo.nombreNodo,aux->availability,aux->clock);
		if(aux->availability > disponibilidadMasAlta){
			disponibilidadMasAlta=aux->availability;
			printf("nueva dispo mas alta %d \n",disponibilidadMasAlta);
			puts("no hay empate");
			empate=false;
		}else if(aux->availability==disponibilidadMasAlta){
			puts("hay empate");
			empate=true;
		}
	}
	if(empate){
		puts("hubo empate. desempato");
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
		printf("Nodo:%s, ava:%d, clock:%d\n,",aux->infoNodo.nombreNodo,aux->availability,aux->clock);
		if(aux->availability == disponibilidadMasAlta){
			historico1=getHistorico(aux);
			printf("historico %s:%d\n,",aux->infoNodo.nombreNodo,historico1);
		}
		if(historico1>historico2){ //todo cambio (?????????????)
			printf("historico de %s:(%d) es el mayor. x ahora lo modifico\n,",aux->infoNodo.nombreNodo,historico1);
			historico2=historico1;
			indiceAModificar=i;
		}

	}
	aux=list_get(listaWorkers,indiceAModificar);
	printf("le pongo el clock a %s, historico %d\n",aux->infoNodo.nombreNodo,historico2);
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

	printf("Nodo encargado: %s. menor carga: %d \n",nodoElegido,menorCarga);

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

	printf("Libero carga en %d\n",divideYRedondea(carga,2));
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

void agregarTransformacionAListaEnProceso(TjobMaster *job, TpackInfoBloque *bloque){


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
	list_add(listaEstadoEnProceso,estado);

	mostrarTablaDeEstados();

}

void mostrarTablaDeEstados(){
	printf("\n\n\n");
	puts("Hubo un cambio de estado, va la tabla actual:");
	int i;
	int id,job,master;
	char * bloque;
	char * etapa;
	char * nodo;
	char * archivoTemporal;
	printf("%-4s%-4s%-7s%-7s%-25s%-15s%-40s%-10s\n", "id", "Job", "Master", "Nodo", "Bloque","Etapa","A Temporal","Estado");
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
		etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"E Pr");
		//free(etapa);
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
		etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"FinOK");
		//free(etapa);
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
		etapa=getNombreEtapa(aux->etapa);
		nodo = aux->nodo;
		archivoTemporal=aux->nombreArchTemporal;
		printf("%-4d%-4d%-7d%-7s%-25s%-15s%-40s%-10s\n",id,job,master,nodo,bloque,etapa,archivoTemporal,"ERR");
		//free(etapa);
	}

	printf("\n\n\n");
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
	return NULL;
}

bool sePuedeReplanificar(int idTarea,t_list * listaComposicionArchivo){
	TpackTablaEstados *tareaAReplanificar=getTareaPorId(idTarea);
	int i;
	if(tareaAReplanificar==NULL){
		puts("La tarea no existe en la tabla de estados");
		return false;
	}
	if(tareaAReplanificar->etapa==REDUCCIONGLOBAL || tareaAReplanificar->etapa==REDUCCIONLOCAL||tareaAReplanificar->etapa==ALMACENAMIENTOFINAL){
		puts("No existe posibilidad de replanificar tarea ya que se encontraba en etapa de Reduccion/AlmacenamientoFinal");
		return false;
	}
	if(tareaAReplanificar->fueReplanificada){
		puts("No se poseen nodos con copias de los datos. no se puede replanificar");
		return false;
	}
	for(i=0;i<list_size(listaComposicionArchivo);i++){
		TpackageUbicacionBloques * bloqueAux =list_get(listaComposicionArchivo,i);
		if(tareaAReplanificar->bloqueDelArchivo == bloqueAux->bloque){
			return true;
		}
	}
	puts("no se puede replanificar");
	return false;
}
int replanificar(int idTarea, int sockMaster,t_list * listaComposicionArchivo){

	sleep(retardoPlanificacionSegs);
	TpackTablaEstados *tareaAReplanificar=getTareaPorId(idTarea);
	TjobMaster *job = getJobPorNroJob(tareaAReplanificar->job);
	int i,packSize,stat;
	char * buffer;

	if(tareaAReplanificar==NULL){
			puts("La tarea no existe en la tabla de estados");
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

				bloqueRet->puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
				bloqueRet->puertoWorker=getPuertoNodo(bloqueRet->nombreNodo,job);

				bloqueRet->ipWorker=malloc(MAXIMA_LONGITUD_IP);
				bloqueRet->ipWorker=getIpNodo(bloqueRet->nombreNodo,job);


			}else{
				//le paso la info para q trabaje en el otronodo
				bloqueRet->bloqueDelDatabin=bloqueAux->bloqueC1;

				bloqueRet->nombreNodo=malloc(TAMANIO_NOMBRE_NODO);
				bloqueRet->nombreNodo=bloqueAux->nombreNodoC1;

				bloqueRet->puertoWorker=malloc(MAXIMA_LONGITUD_PUERTO);
				bloqueRet->puertoWorker=getPuertoNodo(bloqueRet->nombreNodo,job);

				bloqueRet->ipWorker=malloc(MAXIMA_LONGITUD_IP);
				bloqueRet->ipWorker=getIpNodo(bloqueRet->nombreNodo,job);
			}

			bloqueRet->bloqueDelArchivo=tareaAReplanificar->bloqueDelArchivo;
			bloqueRet->bytesOcupados=bloqueAux->finBloque;

			bloqueRet->idTarea=idTareaGlobal++;

			bloqueRet->nombreTemporal=malloc(strlen(tareaAReplanificar->nombreArchTemporal)+1);
			tareaAReplanificar->fueReplanificada=true;



			Theader head;
			head.tipo_de_proceso=YAMA;
			head.tipo_de_mensaje=INFOBLOQUEREPLANIFICADO;
			packSize=0;
			buffer=serializeInfoBloque(head,bloqueRet,&packSize);

			printf("Info del bloque %d serializado, enviamos\n",bloqueRet->bloqueDelArchivo);

			if ((stat = send(sockMaster, buffer, packSize, 0)) == -1){
				puts("no se pudo enviar info del bloque. ");
				return  FALLO_SEND;
			}
			printf("se enviaron %d bytes de la info del bloque\n",stat);

			agregarTransformacionAListaEnProceso(job,bloqueRet);
			actualizarCargaWorkerEn(tareaAReplanificar->nodo,1);
			aumentarHistoricoEn(bloqueRet->nombreNodo,1);
			return 0;
		}
	}
	return FALLO_GRAL;

}


char *  generarNombreTemporal(int idMaster){

	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append(&temp,string_itoa(idMaster));
	string_append(&temp,"-temp");
	string_append(&temp,string_itoa(idTempName++));


	return temp;
}

char *  generarNombreReductorTemporal(char * nombreNodo,int idMaster){

	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append(&temp,string_itoa(idMaster));
	string_append(&temp,"-");
	string_append(&temp,nombreNodo);


	return temp;
}


char *  generarNombreReduccionGlobalTemporal(int idMaster){

	char *temp = string_new();

	string_append(&temp,"tmp/Master");
	string_append(&temp,string_itoa(idMaster));
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

