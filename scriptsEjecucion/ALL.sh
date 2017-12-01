cd tp-2017-2c-Los-Ritchines/dataNode/src/ ;
gcc -g3 -Wall dataNode.c lib/funcionesDN.c  -lfuncionesCompartidas -lcommons -lreadline -lpthread -o ~/dn.out ;
cd;
cd tp-2017-2c-Los-Ritchines/worker/src/ ;
gcc -g3 -Wall worker.c lib/funcionesWK.c lib/masterHandler.c lib/reduccionGlobal.c lib/reduccionLocal.c lib/transformacion.c lib/workerHandler.c  lib/almacenamientoFinal.c -lfuncionesCompartidas -lcommons -lreadline -lpthread -o ~/w.out ;
cd;
cd tp-2017-2c-Los-Ritchines/yama/src/ ;
gcc -g3 -Wall yama.c lib/funcionesYM.c lib/almacenamiento.c lib/reduccionGlobal.c lib/reduccionLocal.c lib/planificacion.c lib/transformacion.c  -lfuncionesCompartidas -lcommons -lreadline -lpthread -o ~/y.out ;
cd;
cd tp-2017-2c-Los-Ritchines/master/src/ ;
gcc -g3 -Wall master.c lib/funcionesMS.c lib/hiloWorkerTransf.c lib/hiloWorkerRedLocal.c lib/hiloWorkerRedGlobal.c lib/hiloAlmacenadoFinal.c  -lfuncionesCompartidas -lcommons  -lm -lpthread -o ~/m.out ;
cd;
cd tp-2017-2c-Los-Ritchines/fileSystem/src/ ;
gcc -g3 -Wall fileSystem.c lib/funcionesFS.c lib/funciones/bitmap.c lib/funciones/conexionYAMA.c lib/funciones/conexiones.c lib/funciones/configuracion.c lib/funciones/consola.c lib/funciones/frees.c lib/funciones/listas.c lib/funciones/serializacionFS.c lib/funciones/tablaArchivos.c lib/funciones/tablaDirectorios.c lib/funciones/tablaNodos.c lib/funciones/tablas.c -lfuncionesCompartidas -lm -lcommons -lreadline -lpthread -o ~/fs.out ;
cd

