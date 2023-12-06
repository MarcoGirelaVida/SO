#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define HAVE_GRP_EX(mode) ((mode & S_IXGRP) == S_IXGRP)
#define HAVE_OTH_EX(mode) ((mode & S_IXOTH) == S_IXOTH)

struct salida
{
    size_t contador_archivos_regulares_permisos_grupoyotros;
    size_t tamanio_total_ocupado;
};

struct salida recorrer__directorio(char * pathname)
{
    fprintf(stderr, "Se ha entrado en la funcion\n");
    DIR * directorio;
    struct dirent * elemento;
    struct stat stats_elemento;
    struct salida s;
    s.contador_archivos_regulares_permisos_grupoyotros = 0;
    s.tamanio_total_ocupado = 0;

    if ((directorio = opendir (pathname)) == NULL){
        fprintf(stderr, "ERROR \"%s\" AL ABRIR DIRECTORIO \"%s\"\n", strerror(errno), pathname);
        exit(errno);
    }
    
    //fprintf(stderr, "Se ha terminado la inicialización\n");

    while ((elemento = readdir(directorio)) != NULL)
    {
        //fprintf(stderr, "Entrada directorio: %s\n", elemento->d_name);
        lstat(elemento->d_name, &stats_elemento);

        if (S_ISREG(stats_elemento.st_mode) && HAVE_GRP_EX(stats_elemento.st_mode) && HAVE_OTH_EX(stats_elemento.st_mode))
        {
            fprintf(stderr, "\t%s %d\n", elemento->d_name, elemento->d_ino);

            s.contador_archivos_regulares_permisos_grupoyotros++;
            s.tamanio_total_ocupado += stats_elemento.st_size;
        }
        else if (S_ISDIR(stats_elemento.st_mode) && !(strcmp(elemento->d_name, ".") == 0 || strcmp(elemento->d_name, "..") == 0))
        {
            fprintf(stderr, "Es directorio: %s\n", elemento->d_name);
            /*
            if (chdir(elemento->d_name))
            {
                fprintf(stderr, "ERROR \"%s\" AL CAMBIAR A DIRECTORIO \"%s\"\n", strerror(errno), elemento->d_name);
                exit(errno); 
            }
            */
            struct salida local = recorrer__directorio(elemento->d_name);

            s.contador_archivos_regulares_permisos_grupoyotros += local.contador_archivos_regulares_permisos_grupoyotros;
            s.tamanio_total_ocupado += local.tamanio_total_ocupado;
        }
    }

    return s;
}

int main(int argc, char * argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "ERROR, NUMERO DE ARGUMENTOS INCORRECTO");
        exit(EXIT_FAILURE);
    }
    
    char * pathname = argv[1];

    /*
    DIR * directorio;
    if ((directorio = opendir (pathname)) == NULL){
        fprintf(stderr, "ERROR \"%s\" AL ABRIR DIRECTORIO \"%s\"\n", strerror(errno), pathname);
        exit(errno);
    }
    */

    printf("Los i-nodos son:\n");

    struct salida s = recorrer__directorio(pathname);

    printf("Existen %zu archivos regulares con permiso x para grupo y otros\n", s.contador_archivos_regulares_permisos_grupoyotros);
    printf("El tamanio total ocupado por dichos archivos es %zu bytes\n", s.tamanio_total_ocupado);

    return EXIT_SUCCESS;
}