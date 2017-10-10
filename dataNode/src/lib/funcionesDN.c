#include "funcionesDN.h"

TdataNode *obtenerConfiguracionDN(char* ruta){
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	TdataNode *pDataNode = malloc(sizeof(TdataNode));

	pDataNode->ip_filesystem       =    malloc(MAXIMA_LONGITUD_IP);
	pDataNode->ip_nodo       =    malloc(MAXIMA_LONGITUD_IP);
	pDataNode->puerto_entrada = malloc(MAXIMA_LONGITUD_PUERTO);
	pDataNode->puerto_master = malloc(MAXIMA_LONGITUD_PUERTO);
	pDataNode->puerto_worker = malloc(MAXIMA_LONGITUD_PUERTO);
	pDataNode->puerto_filesystem = malloc(MAXIMA_LONGITUD_PUERTO);
	pDataNode->ruta_databin=malloc(MAXIMA_LONGITUD_RUTA);
	pDataNode->nombre_nodo=malloc(MAXIMA_LONGITUD_RUTA);

	t_config *dataNodeConfig = config_create(ruta);

	strcpy(pDataNode->ip_filesystem, config_get_string_value(dataNodeConfig, "IP_FILESYSTEM"));
	strcpy(pDataNode->ip_nodo, config_get_string_value(dataNodeConfig, "IP_NODO"));
	strcpy(pDataNode->puerto_entrada, config_get_string_value(dataNodeConfig, "PUERTO_DATANODE"));
	strcpy(pDataNode->puerto_worker, config_get_string_value(dataNodeConfig, "PUERTO_WORKER"));
	strcpy(pDataNode->puerto_master, config_get_string_value(dataNodeConfig, "PUERTO_MASTER"));
	strcpy(pDataNode->puerto_filesystem, config_get_string_value(dataNodeConfig, "PUERTO_FILESYSTEM"));
	strcpy(pDataNode->ruta_databin, config_get_string_value(dataNodeConfig, "RUTA_DATABIN"));
	strcpy(pDataNode->nombre_nodo, config_get_string_value(dataNodeConfig, "NOMBRE_NODO"));



	pDataNode->tipo_de_proceso = DATANODE;

	config_destroy(dataNodeConfig);
	return pDataNode;
}

void mostrarConfiguracion(TdataNode *dn){

	printf("Puerto Entrada: %s\n",  dn->puerto_entrada);
	printf("IP Filesystem %s\n",    dn->ip_filesystem);
	printf("IP Nodo %s\n",    dn->ip_nodo);
	printf("Puerto Master: %s\n",       dn->puerto_master);
	printf("Puerto Filesystem: %s\n", dn->puerto_filesystem);
	printf("Puerto Worker: %s\n", dn->puerto_worker);
	printf("Ruta Databin: %s\n", dn->ruta_databin);
	printf("Nombre Nodo: %s\n", dn->nombre_nodo);
	printf("Tipo de proceso: %d\n", dn->tipo_de_proceso);
}

void setBloque(int posicion, char * bloque){

	memcpy(archivoMapeado + posicion* BLOQUE_SIZE, bloque,strlen(bloque));

	if (msync((void *)archivoMapeado, strlen(bloque), MS_SYNC) < 0) {
				logAndExit("Error al hacer msync");
		}
}

char * getBloque(int posicion){
	char * bloque= malloc(BLOQUE_SIZE);
	memcpy(bloque, archivoMapeado + posicion*BLOQUE_SIZE,BLOQUE_SIZE);
	return bloque;

}
