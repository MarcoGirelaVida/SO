#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);

    printf("Procedo a esperar a SIGUSR1\n");

    if( sigsuspend(&mask) == -1 )
        fprintf(stderr, "ERROR: %s al hacer sigsuspend\n", strerror(errno));

    printf("Ha llegado la señal SIGUSR1, se termina el bloqueo\n");

    return EXIT_SUCCESS;
}