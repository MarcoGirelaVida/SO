#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <sys/wait.h>
#define TAM_MENSAJES_RW 1024
#define longnombre 50

int funcion_proxy(int fd_cerrojo)
{
    // Creo el pipe cliente-proxy con nombre el pid del proxy
    char nombre_fichero_proxy[longnombre];
    if(sprintf(nombre_fichero_proxy, "fifo.%d", getpid()) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al pasar de pid a string\n", strerror(errno));
        exit(errno);
    }
    if (mkfifo(nombre_fichero_proxy, S_IRWXU) == -1){
        fprintf(stderr, "ERROR: \"%s\" al ejecutar crear cauce con nombre: %s\n", strerror(errno), nombre_fichero_proxy);
        exit(errno);
    }
    else
        printf("PROXY %d: Se ha creado el cauce con nombre: %s satisfactoriamente\n", getpid(), nombre_fichero_proxy);

    // Empiezo a leer del fifo, para ello creo un pipe anonimo que utilizaré para ir almacenando los datos
    int fd[2];
    if(pipe(fd) == -1)
    {
        fprintf(stderr, "ERROR: \"%s\" al crear pipe anonimo en el proxy\n", strerror(errno));
        exit(errno);
    }
    else
        printf("PROXY %d: Se ha generado el pipe anonimo satisfactoriamente\n", getpid());

    // Leo del fifo proxy y lo guardo temporalmente en mi pipe local
    char tmp[TAM_MENSAJES_RW];
    size_t bytes_leidos;
    int fd_fifo_cliente_proxy;
    if ((fd_fifo_cliente_proxy = open(nombre_fichero_proxy, O_RDONLY)) == -1){
        fprintf(stderr, "ERROR: \"%s\" al abrir cauce con nombre: %s\n", strerror(errno), nombre_fichero_proxy);
        exit(errno);
    }
    else
        printf("PROXY %d: Se ha abierto el fichero %s satisfactoriamente\n", getpid(), nombre_fichero_proxy);

    while ((bytes_leidos = read(fd_fifo_cliente_proxy, tmp, TAM_MENSAJES_RW)) != 0)
    {
        if (bytes_leidos == -1)
        {
            fprintf(stderr, "ERROR: \"%s\" leer de proxy fifo\n", strerror(errno));
            exit(errno);
        }
        else if (write(fd[1], tmp, bytes_leidos) == -1)
        {
            fprintf(stderr, "ERROR: \"%s\" al escribir en fifo anonimo\n", strerror(errno));
            exit(errno);
        }
        else
            printf("PROXY %d: Se ha leido menasje %s del fichero %s\n", getpid(), tmp, nombre_fichero_proxy);
    }

    // [ activo cerrojo o me bloqueo si ya hay alguien dentro ]
    struct flock cerrojo;
    cerrojo.l_type = F_WRLCK;
    cerrojo.l_whence = SEEK_SET;
    cerrojo.l_len = 0;
    cerrojo.l_start = 0;
    if (fcntl(fd_cerrojo, F_SETLKW, &cerrojo) == -1)
    {
        fprintf(stderr, "ERROR: \"%s\" al intentar poner cerrojo\n", strerror(errno));
        exit(errno);
    }
    else
        printf("PROXY %d: Se ha establecido el cerrojo satisfactoriamente\n", getpid());

    // Leo del fifo anonimo local y printeo en la pantalla
    // alternativa: hacer un mapeo de memoria y printear el buffer de golpe
    while ((bytes_leidos = read(fd[0], tmp, TAM_MENSAJES_RW)) != 0)
    {
        if (bytes_leidos == -1)
        {
            fprintf(stderr, "ERROR: \"%s\" leer de proxy fifo\n", strerror(errno));
            exit(errno);
        }
        else
            printf("%s", tmp);
    }

    // [ desactivo cerrojo ]
    cerrojo.l_type = F_UNLCK;
    if (fcntl(fd_cerrojo, F_SETLKW, &cerrojo) == -1)
    {
        fprintf(stderr, "ERROR: \"%s\" al intentar poner cerrojo\n", strerror(errno));
        exit(errno);
    }
    else
        printf("PROXY %d: Se ha desbloqueado el cerrojo satisfactoriamente\n", getpid());

    // elimino el archivo, pues ya no es necesario
    if (unlink(nombre_fichero_proxy) == -1)
    {
        fprintf(stderr, "ERROR: \"%s\" al eliminar el fichero fifo del proxy\n", strerror(errno));
        exit(errno);
    }
    else
        printf("PROXY %d: Se ha eliminado el fichero %s satisfactoriamnente.\n", getpid(), nombre_fichero_proxy);
    
    return EXIT_SUCCESS;
}

void handler(int sig_num)
{
    while (waitpid(-1, NULL, WNOHANG) != -1);
}

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        printf ("Clientes: Formato: servidor <nombre_fifo> <nombre_cerrojo>\n");
        exit(EXIT_FAILURE);
    }

    char nombrefifoe[longnombre], nombrefifos[longnombre];
    sprintf(nombrefifoe, "%se", argv[1]);
    sprintf(nombrefifos, "%ss", argv[1]);
    // Creo y abro el archivo fifo con nombre para recibir mensajes de los clientes
    if (mkfifo(nombrefifoe, S_IRWXU) == -1){
        fprintf(stderr, "ERROR: \"%s\" al ejecutar crear cauce con nombre: %s\n", strerror(errno), nombrefifoe);
        exit(errno);
    }
    else
        printf("SERVIDOR: Se ha generado el fifo: %s\n", nombrefifoe);
    // Creo y abro el archivo fifo con nombre para emitir mensajes a los clientes
    if (mkfifo(nombrefifos, S_IRWXU) == -1){
        fprintf(stderr, "ERROR: \"%s\" al ejecutar crear cauce con nombre: %s\n", strerror(errno), nombrefifos);
        exit(errno);
    }
    else
        printf("SERVIDOR: Se ha generado el fifo: %s\n", nombrefifos);

    //Creo el proceso cliente
    pid_t pid;
    if ((pid = fork()) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al crear proceso cliente\n", strerror(errno));
        exit(errno);
    }
    if (!pid)
    {
        if (execlp("./clientes", "clientes", argv[1], "1", "&", NULL) == -1) {
            fprintf(stderr, "ERROR: \"%s\" al ejecutar proceso cliente\n", strerror(errno));
            exit(errno);
        }
    }
    else
        printf("SERVIDOR: Se ha generado el proceso cliente con PID: %d\n", pid);

    // [leer petición]
    char mensaje_recibido[sizeof(int)];
    int fd_fifoe;
    if ((fd_fifoe = open(nombrefifoe, O_RDONLY)) == -1){
        fprintf(stderr, "ERROR: \"%s\" al abrir cauce con nombre: %s\n", strerror(errno), nombrefifoe);
        exit(errno);
    }
    else
        printf("SERVIDOR: Se ha abierto el archivo: %s con solo lectura\n", nombrefifoe);
    if(read(fd_fifoe, mensaje_recibido, sizeof(int)) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al leer sobre fifo de recepcion\n", strerror(errno));
        exit(errno);
    }
    else
        printf("SERVIDOR: Se ha leido el mensaje \"%s\" del fichero %s \n",mensaje_recibido, nombrefifoe);

    // Antes de crear los hijos sobrecargo el manejador de SIGCHLD para que no queden hijos zombies
    struct sigaction new_sa;
    new_sa.sa_handler = handler;
    sigemptyset(&new_sa.sa_mask);
    new_sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &new_sa, NULL) == -1)
    {
        fprintf(stderr, "ERROR: \"%s\" al sobrecargar manejador de SIGCHLD\n", strerror(errno));
        exit(errno);
    }
    else
        printf("SERVIDOR: Se ha sobrecargado el handler de SIGCHLD satisfactoriamente\n");

    // Además creo el archivo cerrojo que usaran los procesos proxy para evitar que ambos escriban a la vez en la salida estandar
    int fd_cerrojo;
    if((fd_cerrojo = open(argv[2], O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
    {
        fprintf(stderr, "ERROR: \"%s\" al crear archivo cerrojo\n", strerror(errno));
        exit(errno);
    }
    else
        printf("SERVIDOR: Se ha abierto y creado el archivo: %s con lectura y escritura\n", argv[2]);
    // La otra opción es open(argv[2], O_TMPFILE | O_RDWR, S_IRUSR | S_IWUSR), para crear un archivo temporal anonimo

    //Creo un proceso hijo que ejecute el proceso proxy
    if ((pid = fork()) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al crear proceso proxy\n", strerror(errno));
        exit(errno);
    }
    if (!pid)
    {
        // Por seguridad cierro los fifos abiertos del padre
        if (close(fd_fifoe) == -1) {
            fprintf(stderr, "ERROR: \"%s\" al cerrar los archivos del padre\n", strerror(errno));
            exit(errno);
        }
        else
            printf("PROXY %d: Se ha cerrado el fd_fifoe satifactoriamente\n", getpid());
    
        funcion_proxy(fd_cerrojo);
    }
    else
        printf("SERVIDOR: Se ha generado el proceso proxy con PID: %d\n", pid);

    // Se le manda al cliente el pid del proceso proxy generado
    int fd_fifos;
    if ((fd_fifos = open(nombrefifos, O_WRONLY)) == -1){
        fprintf(stderr, "ERROR: \"%s\" al abrir cauce con nombre: %s\n", strerror(errno), nombrefifos);
        exit(errno);
    }
    else
        printf("SERVIDOR: Se ha abierto el archivo: %s con lectura y escritura\n", nombrefifos);
    if(write(fd_fifos, &pid, sizeof(pid_t)) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al escribir sobre fifo de emisión\n", strerror(errno));
        exit(errno);
    }
    else
        printf("SERVIDOR: Se ha escrito el mensaje \"%d\" en el fichero: %s\n", pid, nombrefifos);

    // Esperar a que todos los hijos terminen
    // Elimino el fichero cerrojo
    if (unlink(argv[2]))
    {
        fprintf(stderr, "ERROR: \"%s\" al eliminar archivo cerrojo\n", strerror(errno));
        exit(errno);
    }
    else
        printf("SERVIDOR: Se ha eliminado el archivo %s satisfactoriamente\n", argv[2]);

    return EXIT_SUCCESS;
}

        // alternativa funcion_proxy();
        /*
        if (execlp("proxy","cerrojo", "&", NULL) == -1) {
            fprintf(stderr, "ERROR: \"%s\" al ejecutar proxy\n", strerror(errno));
            exit(errno);
        }
        */



    // Se le responde al cliente
    /*
    char * pid_proxy;
    if(sprintf(pid_proxy, "%d", pid) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al pasar de pid a string\n", strerror(errno));
        exit(errno);
    }
    */
    // Se le indica al proxy que ya puede empezar a leer
    // kill(pid, SIGUSR1);
    
    

/*
// Se suspende hasta que le manden la señal SIGUSR1
sigset_t mask;
sigfillset(&mask);
sigaddset(&mask, SIGUSR1);
if (sigsuspend(&mask) == -1)
{
    fprintf(stderr, "ERROR: \"%s\" a hacer sigsuspend en el proxy\n", strerror(errno));
    exit(errno);
}
*/


/*
char * leer_indefinidamente(int fd)
{
    char * output = malloc(TAM_MENSAJE);
    size_t bytes_leidos;
    while (bytes_leidos = read(fd, output, TAM_MENSAJE))
    {
        if (bytes_leidos == -1)
        {
            fprintf(stderr, "ERROR: \"%s\" leer de fifo recepcion\n", strerror(errno));
            exit(errno);
        }
        else if (bytes_leidos == TAM_MENSAJE)
        {
            char * aux = output;
            output = malloc(sizeof(aux) + TAM_MENSAJE);
            memcpy(output, aux, sizeof(aux));
        }
    }

    return output;
}
*/