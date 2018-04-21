cd tp-2017-2c-Los-Ritchines/master/src/ ;
gcc -g3 -Wall master.c lib/funcionesMS.c lib/hiloWorkerTransf.c lib/hiloWorkerRedLocal.c lib/hiloWorkerRedGlobal.c lib/hiloAlmacenadoFinal.c  -lfuncionesCompartidas -lcommons  -lm -lpthread -o ~/m.out ;
 cd
