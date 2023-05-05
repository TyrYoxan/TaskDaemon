//
// Created by clement on 29/04/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "message.h"

#define MAX_STRING_LENGTH 1024

int main() {
    char *strings[] = {"Hello", "world", "!", NULL};
    char *strings2 = "+10az 5 echo bonjour";
    int fd;

    // Création du tube nommé
    mkfifo("myfifo", 0644);

    // Ouverture du tube nommé en écriture
    fd = open("myfifo", O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Envoi des chaînes de caractères
    send_argv(fd, strings);

/**
    // Envoi des chaînes de caractères
    send_string(fd, strings2);
**/
    // Fermeture du tube nommé
    close(fd);


    return 0;
}
