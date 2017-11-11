#include "../funcionesFS.h"

t_bitarray* crearBitmap(unsigned int tamanioDatabin){
	int tamanioEnBytes = ceil(tamanioDatabin/8.0);
	printf("tamanioEnBytes %d", tamanioEnBytes);
	puts("voy a romper");
	char * bitarray = calloc(tamanioEnBytes,sizeof(char));
	puts("no pase");
	t_bitarray* bitmap = bitarray_create_with_mode(bitarray,tamanioEnBytes,LSB_FIRST);
	return bitmap;
}

void mostrarBitmap(t_bitarray* bitmap){
	int i;
	puts("El bitmap es:");
	for(i=0;i < bitarray_get_max_bit(bitmap);i++){
		printf("%d",bitarray_test_bit(bitmap,i));
	}
	puts("");
}

void levantarBitmapDeUnNodo(Tnodo * nodo){
	char * rutaArchivo = malloc(150);
	rutaArchivo = string_from_format("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/bitmaps/%s.dat", nodo->nombre);
	puts(rutaArchivo);
	FILE * archivo = fopen(rutaArchivo, "r");
	if(archivo == NULL){
		logErrorAndExit("No se pudo abrir el bitmap del nodo, probablemente no existe.");
	}
	int i = 0;
	char bitChar;
	int bit;
	while(!feof(archivo)){
		fread(&bitChar, 1, sizeof(char), archivo);
		putchar(bitChar);
		bit = bitChar - '0';
		if(bit == 1){
			bitarray_set_bit(nodo->bitmap, i);
		}
		else if(bit == 0){
			bitarray_clean_bit(nodo->bitmap, i);
		}
		else{
			logErrorAndExit("Hubo un error turbio al leer un bit del archivo bitmap.");
		}
		i++;
	}
	mostrarListaDeNodos(listaDeNodosDesconectados);
	free(rutaArchivo);
}

void almacenarTodosLosBitmaps(t_list * lista){
	int i = 0;
	Tnodo * nodo;
	while(i < list_size(lista)){
		nodo = (Tnodo *) list_get(lista, i);
		almacenarBitmap(nodo);
		i++;
	}
}

void almacenarBitmap(Tnodo * nodo){
	char * rutaArchivo = string_from_format("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/bitmaps/%s.dat", nodo->nombre);
	FILE * archivo = fopen(rutaArchivo, "w");
	int i = 0;
	int bit;
	char * bitString;

	while(i < nodo->cantidadBloquesTotal){
		bit = bitarray_test_bit(nodo->bitmap, i);
		bitString = string_itoa(bit);
		fwrite(bitString, 1, strlen(bitString), archivo);
		free(bitString);
		i++;
	}

	fclose(archivo);
	free(rutaArchivo);
}

void almacenarBitEnBitmap(Tnodo * nodo, int numeroDeBloque){
	char * rutaArchivo = string_from_format("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/bitmaps/%s.dat", nodo->nombre);
	FILE * archivo = fopen(rutaArchivo, "r+");
	char * bitChar = string_itoa(bitarray_test_bit(nodo->bitmap, numeroDeBloque));
	fseek(archivo, numeroDeBloque, SEEK_SET);
	fwrite(bitChar, 1, sizeof(char), archivo);
	fclose(archivo);
	free(bitChar);
	free(rutaArchivo);
}


void desocuparBloque(Tnodo * nodo, int numeroDeBloque){
	bitarray_clean_bit(nodo->bitmap, numeroDeBloque);
	nodo->cantidadBloquesLibres++;
	almacenarBitEnBitmap(nodo, numeroDeBloque);
	desocuparBloqueEnTablaDeNodos(nodo->nombre);
}

void ocuparBloque(Tnodo * nodo, int numeroDeBloque){
	bitarray_set_bit(nodo->bitmap, numeroDeBloque);
	nodo->cantidadBloquesLibres--;
	almacenarBitEnBitmap(nodo, numeroDeBloque);
	ocuparBloqueEnTablaNodos(nodo->nombre);
}

void inicializarBitmaps(){
	mkdir("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/bitmaps/",0777);
	removerArchivos("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/bitmaps");
}

void inicializarBitmap(Tnodo* nodo){
	int i = 0;
	while(i < nodo->cantidadBloquesTotal){
		bitarray_clean_bit(nodo->bitmap, i);
		i++;
	}
}
