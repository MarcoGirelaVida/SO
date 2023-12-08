#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    size_t num_procesos_hijos = 5;
    size_t numero_hijos_actuales = 0;
    pid_t pids_hijos[num_procesos_hijos];
    pid_t pid;
    bool es_hijo = false;

    for (size_t i = 0; !es_hijo && i < num_procesos_hijos; i++)
    {
        if ((pid = fork()) == -1)
        {
            fprintf(stderr, "ERROR: \"%s\" AL GENERAR HIJO Nº%zu\n", strerror(errno), i);
            exit(errno);
        }
        else if (pid){
            pids_hijos[i] = pid;
            numero_hijos_actuales++;
        }
        else
            es_hijo = true;
    }
    
    if (es_hijo){
        printf("Soy el hijo: <%d>\n", (numero_hijos_actuales + 1));
        sleep(2);
    }
    else
    {
        siginfo_t info;
        for (size_t i = 0; i < num_procesos_hijos; i=i+2)
        {
            if (waitid(P_PID, pids_hijos[i], &info, WEXITED) == -1){
                fprintf(stderr, "ERROR: \"%s\" AL ESPERAR HIJO Nº%zu\n", strerror(errno), i);
                exit(errno);
            }

            numero_hijos_actuales--;
            printf("Acaba de terminar mi hijo numero %zu con pid: <%d>\n",(i+1), pids_hijos[i]);
            printf("Sólo me quedan <%zu> hijos vivos\n", numero_hijos_actuales);
        }
        
        for (size_t i = 1; i < num_procesos_hijos; i=i+2)
        {
            if (waitid(P_PID, pids_hijos[i], &info, WEXITED) == -1){
                fprintf(stderr, "ERROR: \"%s\" AL ESPERAR HIJO Nº%zu\n", strerror(errno), i);
                exit(errno);
            }

            numero_hijos_actuales--;
            printf("Acaba de terminar mi hijo numero %zu con pid: <%d>\n",(i+1), pids_hijos[i]);
            printf("Sólo me quedan <%zu> hijos vivos\n", numero_hijos_actuales);
        }
    }
    
    return EXIT_SUCCESS;
}