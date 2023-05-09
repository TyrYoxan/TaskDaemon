#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include "message.h"

#define MAX_STRING_LENGTH 1024

/*************  Structures de donnée    *************/

typedef struct {
    char* cmd;          // Commande à exécuter (chaîne de caractères)
    int period;         // Période de la commande (en secondes)
    int start;           // Début de la commande
    int argc;           // Nombre d'arguments de la commande
    char** argv;        // Tableau d'arguments de la commande
}Commande;

typedef struct {
    Commande *commande; // liste de commande
    int size;           // taille actuelle de la liste
}Liste;

/*************  Méthode pour les structures *************/

// Création de la liste
Liste* initListe(){
    Liste *l = malloc(sizeof(Liste));
    l->commande = NULL;
    l->size = 0;

    return l;
}

// Ajouter une commande
void add_cmd(Liste *l, Commande cmd){
    l->commande = realloc(l->commande, (l->size+1)*sizeof(Commande));
    l->commande[l->size] = cmd;
    l->size++;
}

// Supprimer une commande
void supp_cmd(Liste* l, int index){
    if(index >= 0 && index < l->size){
        free(l->commande[index].cmd);
        for(int i=index; i < l->size-1; i++) {
            l->commande[i] = l->commande[i+1];
        }
        l->size--;
        l->commande = realloc(l->commande, l->size*sizeof(Commande));
    }else{
        printf("Error: supp_cmd index invalide");
    }

}

// Récupération d'une commande de la liste
Commande* get_commande(Liste* l, int index) {
    if(index >= 0 && index < l->size) {
        return &(l->commande[index]);
    }
    return NULL;
}

// Supprimer liste
void supprimer_liste(Liste* l) {
    // Libération de la mémoire occupée par chaque commande
    for (int i = 0; i < l->size; i++) {
        Commande* cmd = &(l->commande[i]);
        free(cmd->cmd);
        for (int j = 0; j < cmd->argc; j++) {
            free(cmd->argv[j]);
        }
        free(cmd->argv);
    }

    // Libération de la mémoire occupée par la liste
    free(l->commande);
    l->size = 0;
}

/*************  Méthode    *************/

char* concat_args(char* argv[], int argc) {
    // Calculer la taille de la chaîne résultante
    int size = 0;
    for (int i = 1; i < argc; i++) {
        size += strlen(argv[i]) + 1; // +1 pour l'espace entre les arguments
    }

    // Allouer la mémoire pour la chaîne résultante
    char* result = (char*) malloc((size + 1) * sizeof(char));
    if (result == NULL) {
        perror("Erreur lors de l'allocation de mémoire");
        exit(1);
    }

    // Concaténer les arguments à la chaîne résultante
    strcpy(result, "");
    for (int i = 1; i < argc; i++) {
        strcat(result, argv[i]);
        strcat(result, ";");
    }

    // Ajouter le caractère de fin de chaîne
    result[size] = '\0';

    return result;
}


/*************  Handler    *************/
volatile int sigusr1_received = 0;

void handle_sigusr1(int sig){
    printf("SIGUSR1 \n");
    sigusr1_received =1;
}
/*************  Main    *************/

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
        exit(1);
    }

    fprintf(fp, "%d", pid);
    printf("PID: %d\n",pid);

    fclose(fp);

    // Vérifie si le tube existe
    if(access("/tmp/tasks.fifo", F_OK) == -1){
        // Création du tube nommé
        if(mkfifo("/tmp/tasks.fifo", 0664) == -1){
            perror("Error: mkfifo");
            exit(1);
        }
    }

    // Création ou tronquage de task.txt
    FILE* f = fopen("/tmp/tasks.txt", "w");
    if (f == NULL) {
        printf("Error: fopen");
        exit(1);
    }
    fclose(f);
    
    // Création du répertoire task
    DIR* dir = opendir("/tmp/task");
    if(dir == NULL){
        if(mkdir("/tmp/task", 0777) == -1){
            perror("Error: mkdir task");
            exit(1);
        }
    }
    closedir(dir);

    /** Reception d'un signal **/
    // enregistrement de la fonction de traitement pour SIGUSR1
    signal(SIGUSR1, handle_sigusr1);

    char **recv_strings;
    int fd = open("/tmp/tasks.fifo", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // Réception des chaînes de caractères
    recv_strings = recv_argv(fd);
    int size = 0;
    for (int i = 0; recv_strings[i] != NULL; i++) {
        printf("%s ", recv_strings[i]);
        size++;
    }
    printf("\n");

    char *result = concat_args(recv_strings, size);

    f = fopen("/tmp/task.txt", "w");
    fprintf(f,result);
    fclose(f);


    for (int i = 0; recv_strings[i] != NULL; i++) {
        free(recv_strings[i]);
    }

    free(result);
    free(recv_strings);

    close(fd);

    // Suppression du tube
    unlink("/tmp/tasks.fifo");

    // Suppression de tasd.pid
    if(remove("/tmp/taskd.pid") == -1){
        perror("Error: remove taskd.pid");
        exit(1);
    }

    return 0;
}