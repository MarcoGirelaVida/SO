// DUDA SOBRE LA ENTRADA ESTANDAR Y POR QUE NO PRINTEA NADA CUANDO SE USA

#include<unistd.h>      /* POSIX Standard: 2.10 Symbolic Constants <unistd.h>*/
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>   /* Primitive system data types for abstraction	\
                        of implementation-dependent data types.
                        POSIX Standard: 2.6 Primitive System Data Types
                        <sys/types.h>
                        */
#include<sys/stat.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include <fcntl.h>      // PARA LOS FILE ACCESS MODE (las flags)

int main(int argc, char * argv[])
{
    char * pathname;
    if (argc == 1){
        printf("Leyendo del standar input\n");
        pathname = STDIN_FILENO;
    }
    else{
        printf("Leyendo de argumento\n");
        pathname = argv[1];
    }

    // Abrimos los ficheros
    int fichero_lectura = open(pathname, O_RDONLY);
    int fichero_escritura = open("salida.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    size_t bytes_por_bloque = 80;
    char buffer_lectura[bytes_por_bloque+1];    //+1 para dejarle espacio al \0
    size_t bytes_leidos_en_bloque_i;

    char mensaje_bloque_i[12];
    int numero_bloque = 0;

    while (bytes_leidos_en_bloque_i = read(fichero_lectura, buffer_lectura, bytes_por_bloque))  // Mientras los bytes leido sean mayor que 0
    {    
        buffer_lectura[bytes_por_bloque+1] = '\0';
        sprintf(mensaje_bloque_i, "\n\nBLOQUE %d\n", numero_bloque);
        write(fichero_escritura, mensaje_bloque_i, 12);
        write(fichero_escritura, buffer_lectura, bytes_leidos_en_bloque_i);
        numero_bloque++;
    }

    char mensaje_final[21];
    sprintf(mensaje_final, "\nNUM BLOQUES: <%d>\n", numero_bloque);
    lseek(fichero_escritura, 0, SEEK_SET);
    write(fichero_escritura, mensaje_final, 21);
    close(fichero_lectura);
    close(fichero_escritura);
}