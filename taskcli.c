#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "message.h"

void affUsage(char *nomProgram){
    fprintf(stderr, "Usage : %s START PERIOD CMD [ARG]...\n", nomProgram);
    fprintf(stderr, "Usage : %s\n", nomProgram);

}

pid_t check_taskd_running(){
    FILE *pid_file;
    pid_t taskd_pid;
    const char *pid_file_path = "/tmp/taskd.pid";

    pid_file = fopen(pid_file_path, "r");
    if(pid_file == NULL){
        if(errno == ENOENT){
            fprintf(stderr, "Aucun processus taskd détecté.\n");
        } else {
            perror("Erreur lors de l'ouverture du fichier taskd.pid");
        }
        return -1;
    }

    if(fscanf(pid_file, "%d", &taskd_pid) != 1){
        fprintf(stderr, "Erreur lors de la lecture du PID du processus taskd.\n");
        fclose(pid_file);
        return -1;
    }

    fclose(pid_file);
    return taskd_pid;
}

int main(int argc, char *argv[]){
    if(argc==1){
        FILE* file = fopen("/tmp/tasks.txt", "r");
        if(file==-1){
            perror("open");
            exit(1);
        }
        struct flock lock;
        memset(&lock, 0, sizeof(lock));
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;

        fcntl((int) file, F_SETLKW, &lock);

        sleep(10);


        printf("Commands to run :\n");

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            printf("%s", buffer);
        }

        lock.l_type = F_UNLCK;

        int ret = fclose(file);
        if(ret==-1){
            perror("close");
            exit(1);
        }

        return 0;


    }

    if(argc<4){
        affUsage(argv[0]);
        return 1;
    }

    // Conversion des arguments START et PERIOD en entiers longs
    char *endptr;
    long start = strtol(argv[1], &endptr, 10);
    if(*endptr != '\0'){
        fprintf(stderr, "Invalid start : %s\n", argv[1]);
        affUsage(argv[0]);
        return 1;
    }
    long period = strtol(argv[2], &endptr, 10);
    if(*endptr != '\0'){
        fprintf(stderr, "Invalid period : %s\n", argv[2]);
        affUsage(argv[0]);
        return 1;
    }

    char *cmd = argv[3];


    pid_t taskd_pid = check_taskd_running();
    if(taskd_pid == -1){
        fprintf(stderr, "Aucun processus taskd en cours d'exécution.\n");
        return 1;
    }

    int fifo_fd = open("/tmp/tasks.fifo", O_WRONLY);
    if(fifo_fd==-1){
        perror("Erreur lors de l'ouverture du tube nommé");
        return 1;
    }

    send_argv(fifo_fd, argv);

    kill(check_taskd_running(),SIGUSR1);

    int ret = close(fifo_fd);
    if(ret==-1){
        perror("close");
        exit(1);
    }

    return 0;
}