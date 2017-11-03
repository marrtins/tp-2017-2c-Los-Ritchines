cd tp-2017-2c-Los-Ritchines/worker/src/ ;
gcc -g3 -Wall worker.c lib/funcionesWK.c lib/masterHandler.c lib/reduccionGlobal.c lib/reduccionLocal.c lib/transformacion.c lib/workerHandler.c  lib/almacenamientoFinal.c -lfuncionesCompartidas -lcommons -lreadline -lpthread -o ~/w.out ;
 cd
