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
    if (argc < 3)
    {
        perror("\nError, número de argumentos incorrectos\n");
        exit(-1);
    }
    
    char *endptr;
    char dirpath[] = argv[1];
    int mode = (int)strtol(argv[2],endptr,8);
    // El endptr te indica cuál es el último bit transformado,
    // sirve para comprobar si se han transformado todos.
    // El 8 indica la base (octal), si quieres decimal pones 10
}