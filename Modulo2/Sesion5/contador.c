#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#define NUM_SENIALES 31

static size_t contador_recepciones[NUM_SENIALES];



static void funcion_manejadora(int sig_num)
{
    contador_recepciones[sig_num]++;
    printf("Se ha recibido la señal: \"%s\", %zu veces.\n", strsignal(sig_num), contador_recepciones[sig_num]);
}

int main(void)
{
    // Inicializo el vector de contadores a 0
    for (size_t i = 0; i < NUM_SENIALES; i++)
    {
        contador_recepciones[i] = 0;
    }
    
    struct sigaction sig_action;

    sig_action.sa_handler = funcion_manejadora;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = 0;

    /*
    while (true)
    {
        //Leer señales entrantes y almacenar su numero en:
        int received_signal = '?';
        if (sigaction(received_signal, &sig_action, NULL) == -1)
        {
            if (errno == EINVAL)
                fprintf(stderr, "ERROR: No puedo manejar la señal: %s, no tengo permisos\n",strsignal(received_signal));
            else
                fprintf(stderr, "ERROR: %s al intentar manejar señal: %d", strsignal(received_signal), received_signal);
            
        }
    }
    */

   for (size_t i = 0; i < NUM_SENIALES; i++)
   {
        if (sigaction(i, &sig_action, NULL) == -1)
        {
            if (errno == EINVAL)
                fprintf(stderr, "ERROR No puedo manejar la señal: %s, no tengo permisos\n",strsignal(i));
            else
                fprintf(stderr, "ERROR: \"%s\" al intentar manejar señal: %zu\n", strsignal(i), i);
            
        }
   }
   
    while (true);

    return EXIT_SUCCESS;
}
