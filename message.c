//
// Created by clement on 29/04/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "message.h"

int send_string(int fd, char *str){

    // Vérifie si le descripteur de fichier est valide
    if (fd < 0) {
        perror("Error: invalid file descriptor");
        exit(1);
    }

    // Calcule de la longueur de la chaîne de caractères
    size_t len = strlen(str);

    // Envoie de la longueur de la chaîne en premier
    ssize_t n = write(fd, &len, sizeof(len));
    if (n != sizeof(len)) {
        perror("Error: don't sending string length");
        exit(1);
    }

    // Envoie de la chaîne de caractères
    n = write(fd, str, len);
    if (n != len) {
        perror("Error: error sending string data");
        exit(1);
    }

    // Fermetur du descripteur de fichier
    if (close(fd) < 0) {
        perror("Error: error closing file descriptor");
        exit(1);
    }

    return fd;
}

char *recv_string(int fd){

    // Vérifie si le descripteur de fichier est valide
    if (fd < 0){
        perror("Error: invalid file descriptor");
        exit(1);
    }

    // Lecture de la longueur de la chaîne
    size_t length = 0;
    ssize_t sz = read(fd, &length, sizeof(size_t));
    if(sz < 0){
        perror("Error: read");
        exit(1);
    }

    // Allouer un espace mémoire pour la chaîne
    char *str = calloc(length + 1, sizeof(char));
    if(str == NULL){
        perror("Error: calloc");
        exit(1);
    }

    // Lecture de la chaîne de caractère
    sz = read(fd, str, length);
    if(sz < 0){
        perror("Error: read");
        exit(1);
    }
    str[length] = '\0';

    // Fermer le descripteur de fichier
    int ret = close(fd);
    if (ret == -1) {
        perror("close");
    }

    return str;
}

int send_argv(int fd, char *argv[]) {
    int i = 0;
    int size = 0;

    // Calcul de la taille du tableau
    while (argv[i] != NULL) {
        ++i;
    }
    size = i;
    printf("Taille : %d", i);
    // Envoi de la taille du tableau
    if (write(fd, &size, sizeof(int)) == -1) {
        perror("Erreur lors de l'envoi de la taille du tableau");
        return -1;
    }

    // Envoi de chaque chaîne de caractères
    for (i = 0; i < size; ++i) {
        int len = strlen(argv[i]);

        // Envoi de la taille de la chaîne
        if (write(fd, &len, sizeof(len)) == -1) {
            perror("Erreur lors de l'envoi de la taille de la chaîne");
            return -1;
        }

        // Envoi de la chaîne
        if (write(fd, argv[i], len) == -1) {
            perror("Erreur lors de l'envoi de la chaîne");
            return -1;
        }
    }

    return 0;
}

char **recv_argv(int fd) {
    // Vérifie si le descripteur de fichier est valide
    if (fd < 0) {
        perror("Error: invalid file descriptor");
        exit(1);
    }

    int len;
    ssize_t n = read(fd, &len, sizeof(int)); // On lit la taille du tableau
    if (n < 0) {
        perror("Error: read");
        exit(1);
    }

    char **argv = calloc(len + 1 , sizeof(char *)); // On alloue de la mémoire pour le tableau
    if (argv == NULL) {
        perror("Error: calloc");
        exit(1);
    }

    for (int i = 0; i < len; ++i) {
        argv[i] = NULL;
        int len2;
        n = read(fd, &len2, sizeof(int)); // On lit la taille de la chaîne de caractères
        if (n < 0) {
            perror("Error: read");
            exit(1);
        }

        argv[i] = calloc((len2 + 1) , sizeof(char)); // On alloue de la mémoire pour la chaîne de caractères
        if (argv[i] == NULL) {
            perror("Error: calloc 2");
            exit(1);
        }

        n = read(fd, argv[i], len2); // On lit la chaîne de caractères
        if (n < 0) {
            perror("Error: read");
            exit(1);
        }

        argv[i][len2] = '\0';
    }

    argv[len] = NULL;

    return argv;
}

