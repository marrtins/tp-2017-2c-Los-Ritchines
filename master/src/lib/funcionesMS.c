#include "funcionesMS.h"

Tmaster * obtenerConfiguracionMaster(char* ruta){
	Tmaster *master = malloc(sizeof(Tmaster));
	t_config * masterConfig = config_create(ruta);

	master->ipYama=malloc(MAXIMA_LONGITUD_IP);
	master->puertoYama = malloc(MAXIMA_LONGITUD_IP);

	strcpy(master->ipYama, config_get_string_value(masterConfig, "IP_YAMA"));
	strcpy(master->puertoYama, config_get_string_value(masterConfig, "PUERTO_YAMA"));

	master->tipoDeProceso = MASTER;

	config_destroy(masterConfig);
	return master;
}

void mostrarConfiguracion(Tmaster *master){

	printf("IP Yama %s\n",    master->ipYama);
	printf("Puerto Yama: %s\n",       master->puertoYama);
	printf("Tipo de proceso: %d\n", master->tipoDeProceso);
}
