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
    

    // 1. Leer el directorio
    const char * dirpath = argv[1];
    printf("El directorio es %s\n", dirpath);

    // 2. Leer el modo
    char * endptr;
    mode_t mode = (int)strtol(argv[2], 0, 8);
    printf("El modo es %d\n", mode);
    //strtol es para pasar de string a un sistema numerico, como atoi pero que sirve también para binario, octal...q
    // El endptr te indica cuál es el último bit transformado,
    // sirve para comprobar si se han transformado todos.
    // El 8 indica la base (octal), si quieres decimal pones 10

    // Si el final de la cadena es el principio, es que está vacía
    /*if (*endptr == dirpath) {
        fprintf(stderr, "Cadena vacía\n");
        exit(EXIT_FAILURE);
    }
    
    // Si el final de la cadena no es el final, es que hay caracteres no transformados
    if (**endptr != '\0')
        exit(EXIT_FAILURE);
    */

    // ----- FASE OPERACIONES ------ //
    
    // 1. Abrimos el directorio
    DIR * directorio = opendir(dirpath);

    // Comprobamos que se ha abierto correctamente
    if (directorio == NULL)
    {
        fprintf(stderr, "Error al abrir el directorio\n");
        exit(EXIT_FAILURE);
    }

    // 2. Vamos recorriendo el directorio fichero a fichero mientras cambiamos los permisos
    struct dirent *dirent_i;

    while ((dirent_i = readdir(directorio)) != NULL)
    {
        // Cambiamos los permisos
        if (chmod(dirent_i->d_name, mode) < 0) //Si chmod devuelve -1 es que ha habrido error
            fprintf(stderr, "Error al cambiar los permisos del archivo %s\n", dirent_i->d_name);
    }

    // 3. Cerramos el fichero
    // Comprobamos que se ha cerrado correctamente
    if (closedir(directorio) < 0)
    {
        fprintf(stderr, "Error al cerrar el directorio\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}