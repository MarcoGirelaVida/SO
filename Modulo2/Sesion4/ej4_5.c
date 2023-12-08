#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>


bool esprimo(int numero)
{
    bool loes = numero < 2 ? false : true;

    for (size_t i = 2; loes && i <= sqrt(numero); i++)
    {
        if (!(numero % i))
            loes = false;
    }
    
    //printf("FUNCION ESPRIMO: %d es primo?: %d\n", numero, loes);
    return loes;
}


int main_subproceso_i(int cota_inferior, int cota_superior, int fd, size_t numero_subproceso)
{
    printf("SUBPROCESO Nº%zu: Iniciada función del subproceso\n", numero_subproceso);
    printf("SUBPROCESO Nº%zu: Cota Inferior: %d, Cota Superior: %d\n", numero_subproceso, cota_inferior, cota_superior);
    for (int i = cota_inferior; i < cota_superior; i++)
    {
        if (esprimo(i))
        {
            //printf("SUBPROCESO Nº%zu: Identificado primo <%d>, se procede a su escritura\n", numero_subproceso, i);
            char numero_to_string[100];
            size_t bytes_escritos = snprintf(numero_to_string, sizeof(numero_to_string), "%d", i);
            numero_to_string[bytes_escritos] = '\n';
            write(fd, numero_to_string, bytes_escritos + 1);
        }
    }
    
    printf("SUBPROCESO Nº%zu: Finalizada función del subproceso\n", numero_subproceso);
    exit(EXIT_SUCCESS);
}

int main(int argc, char * argv[])
{
    // Valores por defecto
    size_t numero_subprocesos = 2;
    int cota_inferior = 0;
    int cota_superior = 50;

    // Lectura de argumentos proporcionados
    if (argc > 1) {
        cota_inferior = atoi(argv[1]);
        if (argc > 2) {
            cota_superior = atoi(argv[2]);

            if (cota_superior < cota_inferior) {
                fprintf(stderr, "ERROR, LA COTA SUPERIOR DEBE SER MAYOR QUE LA INFERIOR\n");
                exit(EXIT_FAILURE);
            }
            
            if (argc > 3) {
                numero_subprocesos = atoi(argv[3]);
                if (argc > 4) {
                    fprintf(stderr, "ERROR, MÁXIMO TRES ARGUMENTOS\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    // Inicialización de variables
    // Realmente para los negativos no funciona, habría que poner "max(cota_inferior/superior) - min(cota_inferior/superior)" pero no existen esas funciones
    const size_t intervalo_por_subproceso = (abs(cota_superior) - abs(cota_inferior)) / numero_subprocesos;
    const size_t intervalo_restante = (abs(cota_superior) - abs(cota_inferior)) % numero_subprocesos;
    pid_t pid_subproceso[numero_subprocesos];

    printf("PADRE: Cota Inferior: %d, Cota Superior: %d, Intervalo por Subproceso: %zu, Intervalo Restante: %zu\n", cota_inferior, cota_superior, intervalo_por_subproceso, intervalo_restante);
    printf("*****************************************************\n\n");
    /*************************************************************************/
    // Creo los pipes necesarios
    int fds_pipe_subproceso[numero_subprocesos][2];

    // Inicializamos los subprocesos
    for (size_t i = 0; i < numero_subprocesos; i++)
    {
        int cota_inferior_subproceso_i = i * intervalo_por_subproceso;
        int cota_superior_subproceso_i = cota_inferior_subproceso_i + intervalo_por_subproceso;
        char to_string_cota_inferior_subproceso_i[100];
        char to_string_cota_superior_subproceso_i[100];

        size_t bytes_escritos = snprintf(to_string_cota_inferior_subproceso_i, sizeof(to_string_cota_inferior_subproceso_i), "%d", cota_inferior_subproceso_i);
        to_string_cota_inferior_subproceso_i[bytes_escritos + 1] = '\0';

        bytes_escritos = snprintf(to_string_cota_superior_subproceso_i, sizeof(to_string_cota_superior_subproceso_i), "%d", cota_superior_subproceso_i);
        to_string_cota_superior_subproceso_i[bytes_escritos + 1] = '\0';

        /******/
        // Inicializo el pipe correspondiente
        if (pipe(fds_pipe_subproceso[i]) == -1)
        {
            fprintf(stderr, "ERROR: \"%s\" AL GENERAR PIPE DEL SUBPROCESO Nº%zu\n", strerror(errno), i);
            exit(errno);
        }
        else
            printf("PADRE: Se han generado los pipes con el subproceso Nº%zu\n", i);

        /*****/
        if ((pid_subproceso[i] = fork()) == -1)
        {
            fprintf(stderr, "PADRE: ERROR: \"%s\" AL GENERAR SUBPROCESO Nº%zu\n", strerror(errno), i);
            exit(errno);
        }
        else if (pid_subproceso[i])
        {
            printf("PADRE: Se ha generado el proceso hijo Nº%zu con PID <%d>\n", i, pid_subproceso[i]);
            if (close(fds_pipe_subproceso[i][1]) == -1)
            {
                fprintf(stderr, "PADRE: ERROR: \"%s\" AL CERRAR CANAL ESCRITURA DEL SUBPROCESO Nº%zu\n", strerror(errno), i);
                exit(errno);
            }
            else
                printf("PADRE: Se ha cerrado el canal de escritura con el subproceso Nº%zu\n", i);
        }
        else
        {
            for (size_t j = 0; j <= i; j++)
            {
                if (close(fds_pipe_subproceso[j][0]) == -1) {
                    fprintf(stderr, "SUBPROCESO Nº%zu: ERROR: \"%s\" AL CERRAR CANAL LECTURA CON EL HERMANO Nº%zu\n", i, strerror(errno), j);
                    exit(errno);
                }
                else
                    printf("SUBPROCESO Nº%zu: Se ha cerrado el canal de lectura con el hermano Nº%zu\n", i, j);
            }
            
            main_subproceso_i(cota_inferior_subproceso_i, cota_superior_subproceso_i, fds_pipe_subproceso[i][1], i);
            /*
            if(execlp("./programa_esclavo", to_string_cota_inferior_subproceso_i, to_string_cota_superior_subproceso_i, "&", NULL) == -1)
            {
                fprintf(stderr, "SUBPROCESO Nº%zu: ERROR: \"%s\" AL EJECUTAR PROGRAMA\n", i, strerror(errno));
                exit(errno);
            }
            */
        }
    }

    // A partir de aquí ya solo debería permanecer el programa padre en el main, este es su código
    for (size_t i = 0; i < numero_subprocesos; i++)
    {
        // ¿Habría que gestionar el caracter nulo?
        char char_leido[2];
        int resultado_lectura;
        while (resultado_lectura = read(fds_pipe_subproceso[i][0], char_leido, 1))
        {
            if (resultado_lectura == -1)
            {
                fprintf(stderr, "PADRE: ERROR: \"%s\" AL LEER DEL SUBPROCESO Nº%zu\n", strerror(errno), i);
                exit(errno);
            }
            else{
                char_leido[1] = '\0';
                fprintf(stderr, char_leido);
            }
        }
        
        if (close(fds_pipe_subproceso[i][0]) == -1)
        {
            fprintf(stderr, "PADRE: ERROR: \"%s\" AL CERRAR CANAL LECTURA DEL SUBPROCESO Nº%zu\n", strerror(errno), i);
            exit(errno);
        }
        else
            printf("PADRE: Se ha cerrado el canal de lectura con subproceso Nº%zu\n", i);
    }
    
    exit(EXIT_SUCCESS);
}