cd tp-2017-2c-Los-Ritchines/fileSystem/src/ ;
gcc -g3 -Wall fileSystem.c lib/funcionesFS.c lib/funciones/bitmap.c lib/funciones/conexiones.c lib/funciones/conexionYAMA.c lib/funciones/configuracion.c lib/funciones/consola.c lib/funciones/frees.c lib/funciones/listas.c lib/funciones/serializacionFS.c lib/funciones/tablaArchivos.c lib/funciones/tablaDirectorios.c lib/funciones/tablaNodos.c lib/funciones/tablas.c   -lfuncionesCompartidas -lcommons -lreadline -lpthread -lm -o ~/fs.out ;
 cd
