#include <errno.h>      // para usar errno
#include <stdio.h>      // para todos los prints
#include <sys/types.h>  // para pid
#include <unistd.h>     // para fork
#include <stdlib.h>     // Para exit success/failure y para atoi
#include <string.h>     // Para strerror


int main(int argc, char * argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "ERRO; NUMERO DE ARGUMENTOS INCORRECTO, DEBERIAN SER 2\n");
        exit(EXIT_FAILURE);
    }
    int input_number = atoi(argv[1]);
    
    pid_t pid;
    if ((pid = fork()) == -1)
    {
        fprintf(stderr, "ERROR %s AL HACER FORK\n", strerror(errno));
    }
     

    if (pid)    // Si es el padre
    {
        if (!(input_number % 4))
        {
            printf("El numero %d ES divisible por 4\n", input_number);
        }
        else
        {
            printf("El numero %d NO es divisible por 4\n", input_number);
        }
    }
    else    // Si es un hijo
    {
        if (!(input_number % 2))
        {
            printf("El numero %d ES par\n", input_number);
        }
        else
        {
            printf("El numero %d NO es par\n", input_number);
        }
    }
    
    
    return EXIT_SUCCESS;
}