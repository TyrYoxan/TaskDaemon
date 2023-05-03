#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "message.h"

#define MAX_STRING_LENGTH 1024

int main() {

    // Vérifie si taskd.pid existe déjà
    if(access("/tmp/taskd.pid", F_OK) == 0){
        perror("Error: un processus execute déjà ce programme.");
        exit(1);
    }

    // Ecrire le PID du processus
    pid_t pid = getpid();
    FILE *fp;

    fp = fopen("/tmp/taskd.pid", "w");
    if (fp == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier /tmp/taskd.pid");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%d", pid);

    fclose(fp);

    // Vérifie si le tube existe
    if(access("/tmp/tasks.fifo", F_OK) == -1){
        // Création du tube nommé
        if(mkfifo("/tmp/tasks.fifo", 0644) == -1){
            perror("Error: mkfifo");
            exit(1);
        }
    }

    // Création ou tronquage de tesk.txt
    FILE* f = fopen("/tmp/tasks.txt", "w");
    if (f == NULL) {
        printf("Error: fopen");
        exit(1);
    }
    fclose(f);
    

    unlink("/tmp/tasks.fifo");
    
    return 0;
}