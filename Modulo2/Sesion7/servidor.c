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
#define TAM_MENSAJES_RW (1024)


int CREAT_NAMED_FIFO(char * pathname, int open_flag)
{
    int fd;
    if (mkfifo(pathname, S_IRWXU) == -1){
        fprintf(stderr, "ERROR: \"%s\" al ejecutar crear cauce con nombre: %s\n", strerror(errno), pathname);
        exit(errno);
    }
    if ((fd = open(pathname, open_flag)) == -1){
        fprintf(stderr, "ERROR: \"%s\" al abrir cauce con nombre: %s\n", strerror(errno), pathname);
        exit(errno);
    }

    return fd;
}


int funcion_proxy()
{
    // Creo el pipe cliente-proxy con nombre el pid del proxy
    char * nombre_fichero_proxy;
    if(sprintf(nombre_fichero_proxy, "fifo.%d", getpid()) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al pasar de pid a string\n", strerror(errno));
        exit(errno);
    }
    int fifo_cliente_proxy = CREAT_NAMED_FIFO(nombre_fichero_proxy, O_RDONLY);

    // Empiezo a leer del fifo, para ello creo un pipe anonimo que utilizaré para ir almacenando los datos
    int fd[2];
    if(pipe(fd) == -1)
    {
        fprintf(stderr, "ERROR: \"%s\" al crear pipe anonimo en el proxy\n", strerror(errno));
        exit(errno);
    }

    // Leo del fifo proxy y lo guardo temporalmente en mi pipe local
    char tmp[TAM_MENSAJES_RW];
    size_t bytes_leidos;
    while (bytes_leidos = read(fifo_cliente_proxy, tmp, TAM_MENSAJES_RW))
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
    }

    // [ activo cerrojo del stdout ]
    // Leo del fifo anonimo local y printeo en la pantalla
    while (bytes_leidos = read(fd[0], tmp, TAM_MENSAJES_RW))
    {
        if (bytes_leidos == -1)
        {
            fprintf(stderr, "ERROR: \"%s\" leer de proxy fifo\n", strerror(errno));
            exit(errno);
        }
        else
            printf("%s", tmp);
    }
    // [ desactivo cerrojo del stdout]

    // elimino el archivo, pues ya no es necesario
    if (unlink(nombre_fichero_proxy) == -1)
    {
        fprintf(stderr, "ERROR: \"%s\" al eliminar el fichero fifo del proxy\n", strerror(errno));
        exit(errno);
    }
    
    return EXIT_SUCCESS;
}

void handler(int sig_num)
{
    struct siginfo_t infop;
    while (waitid(P_ALL, 0, &infop, WNOHANG) != -1);
}
int main()
{
    // Creo y abro el archivo fifo con nombre para recibir mensajes de los clientes
    int fifo_recepcion = CREAT_NAMED_FIFO("FIFO_cliente>servidor", O_RDONLY);
    // Creo y abro el archivo fifo con nombre para emitir mensajes a los clientes
    int fifo_emision = CREAT_NAMED_FIFO("FIFO_cliente<servidor", O_WRONLY);

    // [leer petición]
    char * mensaje_recibido[sizeof(int)];
    if(read(fifo_recepcion, mensaje_recibido, sizeof(int)) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al leer sobre fifo de recepcion\n", strerror(errno));
        exit(errno);
    }

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
    //Creo un proceso hijo que ejecute el proceso proxy
    pid_t pid;
    if ((pid = fork()) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al crear proceso proxy\n", strerror(errno));
        exit(errno);
    }
    if (!pid)
    {
        // Por seguridad cierro los fifos abiertos del padre
        if ((close(fifo_emision) == -1) || (close(fifo_recepcion) == -1)) {
            fprintf(stderr, "ERROR: \"%s\" al cerrar los archivos del padre\n", strerror(errno));
            exit(errno);
        }
    
        // alternativa funcion_proxy();
        if (execlp("proxy","cliente", "&", NULL) == -1) {
            fprintf(stderr, "ERROR: \"%s\" al ejecutar proxy\n", strerror(errno));
            exit(errno);
        }
    }

    // Se le responde al cliente
    /*
    char * pid_proxy;
    if(sprintf(pid_proxy, "%d", pid) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al pasar de pid a string\n", strerror(errno));
        exit(errno);
    }
    */
    if(write(fifo_emision, &pid, sizeof(pid_t)) == -1) {
        fprintf(stderr, "ERROR: \"%s\" al escribir sobre fifo de emisión\n", strerror(errno));
        exit(errno);
    }

    // Se le indica al proxy que ya puede empezar a leer
    // kill(pid, SIGUSR1);
    
    
    return EXIT_SUCCESS;
}





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