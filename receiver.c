//
// Created by clement on 29/04/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "message.h"

#define MAX_STRING_LENGTH 1024

int main() {
    char **recv_strings;
    char *strings;
    int fd;

    // Ouverture du tube nommé en lecture
    fd = open("myfifo", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }


    // Réception des chaînes de caractères
    recv_strings = recv_argv(fd);
    printf("Received strings:\n");
    for (int i = 0; recv_strings[i] != NULL; i++) {
        printf("%s ", recv_strings[i]);
        free(recv_strings[i]);
    }
    printf("\n");
    free(recv_strings);

    // Réception des chaînes de caractères
    strings = recv_string(fd);
    printf("Received strings:\n");

    printf("%s\n", strings);
    free(strings);

    // Fermeture du tube nommé
    close(fd);

    // Suppression du tube nommé
    unlink("myfifo");
    return 0;
}
