//
// Created by clement on 05/05/23.
//
#include <bits/types/FILE.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "message.h"

//
// Created by clement on 01/05/23.
//
int main(int argc, char *argv[]) {
    if(argc != 1 && argc < 4){
        perror("Usage : ./taskcli START PERIOD CMD [ARG]...\n"
               "Usage : ./taskcli\n");
        exit(1);
    }

    // Ouvrir le fichier
    FILE* fichier = fopen("/tmp/taskd.pid", "r");
    if (fichier == NULL) {
        printf("Impossible d'ouvrir le fichier.\n");
        return 1;
    }

    // Lire le contenu du fichier ligne par ligne
    int pid;
    fscanf(fichier,"%d",&pid);


    // Fermer le fichier
    fclose(fichier);

/**
    // Allouer un tableau dynamique de chaînes de caractères
    char **args = malloc(argc * sizeof(char*));
    if (args == NULL) {
        // Gérer l'erreur d'allocation mémoire
        perror("Error: malloc");
        exit(1);
    }

    // Allouer dynamiquement chaque chaîne de caractères et les copier dans le tableau
    for (int i = 0; i < argc; i++) {
        char *arg = malloc(strlen(argv[i]) + 1);
        if (arg == NULL) {
            // Gérer l'erreur d'allocation mémoire
            perror("Error: malloc2");
            exit(1);
        }
        strcpy(arg, argv[i]);
        args[i] = arg;
    }
    args[argc] = NULL;**/

    // Faire quelque chose avec les arguments
    // Ouverture du tube nommé en écriture
    int fd;
    fd = open("/tmp/tasks.fifo", O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    char *strings[] = {"Hello", "world", "!", NULL};
    // Envoi des chaînes de caractères
    for(int i = 0; i < argc; i++) {
        printf("%s ", strings[i]);
    }
    printf("\n");
    send_argv(fd, strings);

    // Libérer la mémoire allouée
    for (int i = 0; i < argc; i++) {
        free(strings[i]);
    }

    free(strings);

    kill(pid, SIGUSR1);

    // Fermeture du tube nommé
    close(fd);

    return 0;
}
