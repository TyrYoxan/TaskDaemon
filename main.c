#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "message.h"


int main() {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Erreur lors de la création de la pipe");
        return -1;
    }

    char *argv[] = {"hello", "world", NULL};
    if (send_argv(fd[1], argv) == -1) {
        perror("Erreur lors de l'envoi du tableau");
        return -1;
    }

    char **recv_argv = recv_argv(fd[0]);
    if (recv_argv == NULL) {
        perror("Erreur lors de la réception du tableau");
        return -1;
    }

    // Vérifie si les tableaux sont identiques
    int i = 0;
    while (argv[i] != NULL && recv_argv[i] != NULL) {
        if (strcmp(argv[i], recv_argv[i]) != 0) {
            printf("Erreur: Les chaînes de caractères ne sont pas identiques\n");
            return -1;
        }
        i++;
    }

    if (argv[i] != NULL || recv_argv[i] != NULL) {
        printf("Erreur: Les tableaux ont des tailles différentes\n");
        return -1;
    }

    printf("Le tableau envoyé et reçu sont identiques\n");
    return 0;
}
