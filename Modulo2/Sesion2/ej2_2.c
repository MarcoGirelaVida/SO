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
    
    char **endptr;
    const char * dirpath = argv[1];
    int mode = strtol(argv[2],endptr,10);
    fprintf(stderr, "MODE = %d", mode);
    const mode_t new_mode = strtol(argv[2],endptr,10);

    //strtol es para pasar de string a un sistema numerico, como atoi pero que sirve también para binario, octal...q
    // El endptr te indica cuál es el último bit transformado,
    // sirve para comprobar si se han transformado todos.
    // El 8 indica la base (octal), si quieres decimal pones 10

    /*
    if (*endptr == argv[2]) {
        fprintf(stderr, "Cadena vacía\n");
        exit(EXIT_FAILURE);
    }
    if (**endptr != '\0')
        exit(EXIT_FAILURE);
    */


    // ----- FASE OPERACIONES ------ //
    
    // 1. Abrimos el directorio y lo recorremos
    DIR * directorio = opendir(dirpath);
    struct dirent * elemento_directorio;


    while ((elemento_directorio = readdir(directorio)) != NULL)
    {  
        struct stat stats_elemento;
        stat(elemento_directorio->d_name, &stats_elemento);
        fprintf(stderr, "holahola");  
        mode_t antiguos_mode = stats_elemento.st_mode;

        if (chmod(elemento_directorio->d_name, new_mode) == 0)
        {
            printf("<%s> : <%d> --> <%d>", elemento_directorio->d_name, antiguos_mode, stats_elemento.st_mode);
        }
        else
        {
            printf("<%s> : <%d> -X-> <%d>", elemento_directorio->d_name, errno, stats_elemento.st_mode);
        }
    }
    
}