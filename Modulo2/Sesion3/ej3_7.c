#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char * argv[])
{
    bool execute_in_background = false;
    size_t siguiente_argumento = 1;
    while (siguiente_argumento < argc && argv[siguiente_argumento][1] == '-')
    {
        if(!strcmp(argv[siguiente_argumento], "-bg"))
        {
            execute_in_background = true;
        }
        else
        {
            fprintf(stderr, "ERROR, OPCION INVALIDA\n");
            exit(EXIT_FAILURE);
        }
        
        siguiente_argumento++;
    }
    

    int argc_de_programa_a_ejecutar = argc - siguiente_argumento;
    if (argc_de_programa_a_ejecutar == 0){
        fprintf(stderr, "ERROR, DEBE PROPORCIONAR AL MENOS UN PROGRAMA COMO ARGUMENTO\n");
        exit(EXIT_FAILURE);
    }

    char * argv_de_programa_a_ejecutar[argc_de_programa_a_ejecutar + 1];
    for (size_t i = 0; i < argc_de_programa_a_ejecutar; i++, siguiente_argumento++)
        argv_de_programa_a_ejecutar[i] = argv[siguiente_argumento];

    if (execute_in_background)
    {
        argc_de_programa_a_ejecutar++;
        argv_de_programa_a_ejecutar[argc_de_programa_a_ejecutar] = "&";
    }
    
    argv_de_programa_a_ejecutar[argc_de_programa_a_ejecutar + 1] = NULL;
    
    

    /*******************************************************************************************************/
    pid_t pid;
    if ((pid = fork()) == -1)
    {
        fprintf(stderr, "ERROR: \"%s\" AL GENERAR HIJO\n", strerror(errno));
        exit(errno);
    }

    if (!pid)
    {
        if(execvp(argv_de_programa_a_ejecutar[0], argv_de_programa_a_ejecutar) == -1)
        {
            fprintf(stderr, "ERROR: \"%s\" AL EJECUTAR PROGRAMA %s HIJO\n", strerror(errno), argv_de_programa_a_ejecutar[0]);
            exit(errno);
        }
    }
    
    return(EXIT_SUCCESS);
}