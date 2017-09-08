/*
 * filesystemConfigurator.c
 *
 *  Created on: 8/9/2017
 *      Author: utnso
 */


#include "filesystem.h"


int leerTablaDirectorios(){
	FILE * fp;


	fp = fopen ("/home/utnso/tp-2017-2c-Los-Ritchines/filesystem/metadata", "rw");


	t_directory* directorios = malloc(100 * sizeof *directorios);
	int i=0;
	int temp1,temp2;

	while(!feof(fp)){

		fscanf(fp, "%d %s %d ",&temp1, directorios[i].nombre,&temp2);
		directorios[i].index=temp1;
		directorios[i].padre=temp2;
		i++;
	}
	fclose(fp);

	return 0;
}

