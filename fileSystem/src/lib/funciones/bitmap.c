#include "../funcionesFS.h"

t_bitarray* crearBitmap(int tamanioDatabin){
	int tamanioEnBits = ceil(tamanioDatabin/8.0);
	char * bitarray = calloc(tamanioEnBits,1);
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
