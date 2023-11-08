#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<errno.h>
#include<dirent.h>


int main(int argc, char*argv[])
{
    // ------ FASE LECTURA ------- //
    if (argc < 3)
    {
        fprintf(stderr, "\nError, número de argumentos incorrectos\n");
        exit(EXIT_FAILURE);
    }
    
    char *endptr;
    const char dirpath[] = argv[1];
    const int mode = (int)strtol(argv[2],endptr,8);
    //strtol es para pasar de string a un sistema numerico, como atoi pero que sirve también para binario, octal...q
    // El endptr te indica cuál es el último bit transformado,
    // sirve para comprobar si se han transformado todos.
    // El 8 indica la base (octal), si quieres decimal pones 10
    if (endptr == dirpath) {
        fprintf(stderr, "Cadena vacía\n");
        exit(EXIT_FAILURE);
    }
    if (*endptr != '\0')
        exit(EXIT_FAILURE);


    // ----- FASE OPERACIONES ------ //
    
    // 1. Abrimos el directorio y lo recorremos
    opendir(dirpath);
}