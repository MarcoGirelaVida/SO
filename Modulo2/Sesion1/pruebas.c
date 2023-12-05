#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>  // PARA LOS FILE ACCESS MODE (las flags)
#include<stdbool.h>


void prueba_lectura(char * pathname){
    
    // Abrimos los ficheros
    int fichero_lectura = open(pathname, O_RDONLY);

    int bytes_leidos = 1;
    while (bytes_leidos)
    {
        bool nueva_linea = false;
        char linea[200];
        int num_caracteres = 0;

        while (!nueva_linea && bytes_leidos)
        {
            char caracter[1];
            bytes_leidos = read(fichero_lectura, caracter, 1);

            if (bytes_leidos)
            {
                linea[num_caracteres] = caracter[0];
                num_caracteres++;

                if (caracter[0] == '\n')
                {
                    nueva_linea = true;
                }
            }
            
        }
        linea[num_caracteres++] = '\0';

        printf("\nNUEVA LINEA: \n");
        printf(linea);
    }
    
    close(fichero_lectura);
}


int main(int argc, char * argv[])
{
    prueba_lectura("tarea1.c");

    return 0;
}