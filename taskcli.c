#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "message.h"

void affUsage(char *nomProgram) {
    fprintf(stderr, "Usage : %s START PERIOD CMD [ARG]...\n", nomProgram);
    fprintf(stderr, "Usage : %s\n", nomProgram);

}

pid_t check_taskd_running() {
    FILE *pid_file;
    pid_t taskd_pid;
    const char *pid_file_path = "/tmp/taskd.pid";

    pid_file = fopen(pid_file_path, "r");
    if (pid_file == NULL) {
        if (errno == ENOENT) {
            perror("Aucun processus taskd détecté.");
        } else {
            perror("Erreur lors de l'ouverture du fichier taskd.pid");
        }
        exit(1);
    }

    if (fscanf(pid_file, "%d", &taskd_pid) != 1) {
        perror("Erreur lors de la lecture du PID du processus taskd.");
        fclose(pid_file);
        exit(1);
    }

    fclose(pid_file);
    return taskd_pid;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        affUsage(argv[0]);
        exit(1);
    }

    // Conversion des arguments START et PERIOD en entiers longs
    char *endptr;
    long start = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid start : %s\n", argv[1]);
        affUsage(argv[0]);
        exit(1);
    }
    long period = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid period : %s\n", argv[2]);
        affUsage(argv[0]);
        exit(1);
    }

    char *cmd = argv[3];


    pid_t taskd_pid = check_taskd_running();
    if (taskd_pid == -1) {
        perror("Aucun processus taskd en cours d'exécution.");
        exit(1);
    }

    int fifo_fd = open("/tmp/tasks.fifo", O_WRONLY);
    if (fifo_fd == -1) {
        perror("Erreur lors de l'ouverture du tube nommé");
        exit(1);
    }

    send_argv(fifo_fd, argv);

    printf("GG");

    kill(taskd_pid,SIGUSR1);
    printf("taskd : %d \n",taskd_pid);
    int ret = close(fifo_fd);
    if (ret == -1) {
        perror("close");
        exit(1);
    }

    return 0;
}
