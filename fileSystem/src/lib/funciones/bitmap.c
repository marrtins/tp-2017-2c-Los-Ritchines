#include "../funcionesFS.h"

t_bitarray* crearBitmap(int tamanioDatabin){
	int tamanioEnBits = ceil(tamanioDatabin/8.0);
	printf("tamanioEnBits %d", tamanioEnBits);
	puts("voy a romper");
	char * bitarray = calloc(tamanioEnBits,sizeof(char));
	puts("no pase");
	t_bitarray* bitmap = bitarray_create_with_mode(bitarray,tamanioEnBits,LSB_FIRST);
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
	char * rutaArchivo = string_from_format("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/bitmaps/%s.dat", nodo->nombre);
	FILE * archivo = fopen(rutaArchivo, "r");
	puts("pude abrir");
	int i = 0;
	char bitChar;
	int bit;
	while(!feof(archivo)){
		fread(bitChar, 1, sizeof(char), archivo);
		printf("el bitchar %c", bitChar);
		bit = bitChar;
		printf("el bit", bit);
		if(bit == 1){
			bitarray_set_bit(nodo->bitmap, i);
		}
		i++;
	}
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


void desocuparBloqueEnBitmap(Tnodo * nodo, int numeroDeBloque){
	bitarray_clean_bit(nodo->bitmap, numeroDeBloque);
	almacenarBitmap(nodo);
	desocuparBloqueEnTablaDeArchivo(nodo->nombre);
}
void inicializarBitmaps(){
	mkdir("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/bitmaps/",0777);
	removerArchivos("/home/utnso/tp-2017-2c-Los-Ritchines/fileSystem/src/metadata/bitmaps");
}
