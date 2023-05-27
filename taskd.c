#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include "message.h"
#include <time.h>
#include <locale.h>
#include <limits.h>
#include <sys/wait.h>

#define MAX_LENGTH 64
/*************  Structures de donnée    *************/

typedef struct {
    int num;            // Numéro de la commande
    int period;         // Période de la commande (en secondes)
    int start;          // Début de la commande
    int argc;           // Nombre d'arguments de la commande
    char** argv;        // Tableau d'arguments de la commande
}Commande;

typedef struct {
    Commande **commande; // tableau de pointeurs vers les commandes
    int size;            // taille actuelle de la liste (nombre de commandes)
    int capacity;        // capacité du tableau (taille allouée dans le tas)
} Liste;

/*************  Méthode pour les structures *************/

// Création de la liste
Liste *initListe(){
    Liste *l = malloc(sizeof(Liste));
    l->commande = NULL;
    l->size = 0;
    l->capacity = 0;

    return l;
}

int command_counter = 1;

// Ajouter une commande
void add_cmd(Liste *l, Commande cmd) {
    if (l->size == l->capacity) {
        // Réallouer le tableau avec une nouvelle capacité plus grande
        int new_capacity = (l->capacity == 0) ? 1 : l->capacity * 2;
        l->commande = realloc(l->commande, new_capacity * sizeof(Commande *));
        if (!l->commande) {
            perror("Erreur lors de l'allocation de mémoire");
            exit(1);
        }
        l->capacity = new_capacity;
    }

    // Allouer de la mémoire pour la nouvelle commande
    Commande *new_cmd = malloc(sizeof(Commande));
    if (!new_cmd) {
        perror("Erreur lors de l'allocation de mémoire");
        exit(1);
    }

    // Copier les valeurs de la commande dans la nouvelle zone allouée
    memcpy(new_cmd, &cmd, sizeof(Commande));

    l->commande[l->size] = new_cmd;
    l->size++;

}

// Supprimer une commande
void supp_cmd(Liste* l, int index) {

        Commande* cmd = l->commande[index];

        // Libérer les chaînes de caractères individuelles dans argv
        for (int i = 0; i < cmd->argc + 1; ++i) {
            free(cmd->argv[i]);
        }

        // Libérer le tableau de commandes argv
        free(cmd->argv);


        if (l->size != 1) {
            // Libérer la structure Commande elle-même
            free(cmd);
            // Déplacer les commandes suivantes d'un indice vers la gauche
            for (int i = index + 1; i < l->size; ++i) {
                l->commande[i - 1] = l->commande[i];
            }

            // Mettre à jour la taille de la liste
            l->size--;

            // Réallouer l'espace mémoire pour le tableau de commandes
            l->commande = realloc(l->commande, l->size * sizeof(Commande *));
        }
}

// Supprimer liste
void supprimer_liste(Liste* l) {
    if (l == NULL) {
        return; // Vérifier si la liste est nulle
    }

    for (int i = 0; i < l->size; i++) {
        Commande* cmd = l->commande[i];

        // Libérer les chaînes de caractères individuelles dans argv
        for (int i = 0; i < cmd->argc + 1; ++i) {
            free(cmd->argv[i]);
        }

        // Libérer le tableau de commandes argv
        free(cmd->argv);


        if (l->size != 1) {
            // Libérer la structure Commande elle-même
            free(cmd);
            // Déplacer les commandes suivantes d'un indice vers la gauche
            for (int j = i + 1; j < l->size; ++j) {
                l->commande[j - 1] = l->commande[j];
            }

            // Mettre à jour la taille de la liste
            l->size--;

            // Réallouer l'espace mémoire pour le tableau de commandes
            l->commande = realloc(l->commande, l->size * sizeof(Commande *));
        }
    }

    free(l->commande);
    l->commande = NULL;
    printf("Liste size : %d \n", l->size);

    free(l); // Libération de la mémoire occupée par la structure Liste
}

// Afficher la liste des commandes
void print_liste(Liste *l) {
    printf("Liste des commandes : \n");
    for(int i=0; i<l->size; i++){
        printf("Commande %d:\n", i+1);
        printf("Numéro : %d\n", l->commande[i]->num);
        printf("Période : %d\n", l->commande[i]->period);
        printf("Début : %d\n", l->commande[i]->start);
        printf("Nombre d'arguments : %d\n", l->commande[i]->argc);
        printf("Arguments : ");
        for (int j = 0; j < l->commande[i]->argc; j++) {
            printf("%s ", l->commande[i]->argv[j]);
        }
        printf("\n\n");
    }
}

/*************  Méthode    *************/
Commande *exec_commande = NULL;

char* when(int start){
    time_t inputTime = start;

    struct tm *timeInfo;
    char *buffer = malloc(80 * sizeof(char));

    setlocale(LC_TIME, "fr_FR.UTF-8");  // Définit le locale en français

    timeInfo = localtime(&inputTime);
    if (timeInfo == NULL) {
        perror("Erreur lors de la conversion de la date");
        exit(1);
    }

    strftime(buffer, 80, "%e %B  %H:%M:%S", timeInfo);

    return buffer;
}

char* concat_args(char* argv[], int argc, Commande* cmd) {
    // Création de la commande
    cmd->num = command_counter;

    char start[MAX_LENGTH]; // Allouer de la mémoire pour start
    strcpy(start, argv[1]);
    cmd->start = time(NULL) + atoi(start);

    char period[MAX_LENGTH]; // Allouer de la mémoire pour period
    strcpy(period, argv[2]);
    cmd->period = atoi(period);
    cmd->argc = argc - 3;

    cmd->argv = malloc((cmd->argc + 1) * sizeof(char*));
    if (!cmd->argv) {
        perror("Erreur lors de l'allocation de mémoire");
        exit(1);
    }
    for (int i = 0; i < cmd->argc; ++i) {
        cmd->argv[i] = malloc(strlen(argv[i + 3]) + 1);
        strcpy(cmd->argv[i], argv[i + 3]);
    }
    cmd->argv[cmd->argc] = NULL;

    int max_len = 1024 + cmd->argc * 50; // Augmenter la taille maximale pour inclure suffisamment d'espace pour les arguments
    char* str = malloc(max_len * sizeof(char));
    if (!str) {
        perror("Erreur lors de l'allocation de mémoire");
        exit(1);
    }

    char *time = when(cmd->start);

    snprintf(str, max_len, "%d;%s;%d; ", cmd->num, time, cmd->period);
    for (int i = 0; i < cmd->argc; i++) {
        if (cmd->argv[i] != NULL && strlen(cmd->argv[i]) > 0) {
            size_t current_len = strlen(str);
            size_t arg_len = strlen(cmd->argv[i]);
            if (current_len + arg_len + 1 < max_len) {
                strncat(str, cmd->argv[i], max_len - current_len - 1);
                strncat(str, " ", max_len - strlen(str) - 1);
            } else {
                fprintf(stderr, "Erreur : La longueur maximale de la chaîne est dépassée.\n");
                break;
            }
        }
    }

    free(time);

    // Ajouter le caractère de fin de chaîne
    str[max_len - 1] = '\0';

    ++command_counter;

    for(int i = 0; i< argc; ++i){
        free(argv[i]);
    }
    free(argv);

    return str;
}

void getnextTime(Liste *l){

    time_t minDuree = INT_MAX;
    // Parcourir la liste des commandes
    for (int i = 0; i < l->size; i++) {
        if (l->commande[i]) {
            time_t duree = l->commande[i]->start - time(NULL);
            if (minDuree > duree && duree >= 0) {
                minDuree = duree;
                exec_commande = l->commande[i];
            }
        }
    }

    printf("Time : %ld\n", minDuree);
    alarm(minDuree);
}

/*************  Handler    *************/

volatile int sigusr1_received = 0;
volatile int alarm_received = 0;
volatile int suppression_received = 0;

void handler_sigusr1(int sig){
    printf("SIGUSR1 \n");
    sigusr1_received = 1;
}

void handler_alarm(int sig){
    printf("ALARM !!! \n");
    alarm_received = 1;
}

void handler_supression(int sig){
    suppression_received = 1;
}

void handler_sigchild(int sig){
    pid_t pid;
    int status;

    // Récupérer et afficher le statut de tous les fils terminés
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Le processus fils %d s'est terminé normalement avec le code de sortie %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Le processus fils %d s'est terminé suite à un signal %d\n", pid, WTERMSIG(status));
        }
    }
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
 
    struct sigaction action;
    struct sigaction action2;
    struct sigaction action3;
    struct sigaction action4;

    // Configurer le gestionnaire pour SIGUSR1
    action.sa_handler = handler_sigusr1;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGUSR1, &action, NULL);

    // Configurer le gestionnaire pour SIGALRM
    action2.sa_handler = handler_alarm;
    sigemptyset(&action2.sa_mask);
    action2.sa_flags = 0;
    sigaction(SIGALRM, &action2, NULL);

    // Configurer le gestionnaire pour SIGALRM
    action3.sa_handler = handler_supression;
    sigemptyset(&action3.sa_mask);
    action3.sa_flags = 0;
    sigaction(SIGQUIT, &action3, NULL);
    sigaction(SIGINT, &action3, NULL);
    sigaction(SIGTERM, &action3, NULL);

    // Configurer le gestionnaire pour SIGCHILD
    action4.sa_handler = handler_sigchild;
    sigemptyset(&action4.sa_mask);
    action4.sa_flags = 0;
    sigaction(SIGCHLD, &action4, NULL);


    int fd = open("/tmp/tasks.fifo", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    Liste *liste;
    liste = initListe();

    char **recv_strings;
    char *result;

    while(1){

        getnextTime(liste);

        /* Signal SIGUSR1 reçu */
        if(sigusr1_received){


            // Réception des chaînes de caractères
            recv_strings = recv_argv(fd);
            int size = 0;
            for (int i = 0; recv_strings[i] != NULL; i++) {
                printf("%s ", recv_strings[i]);
                size++;
            }
            printf("\n");


            Commande cmd = {0};

            result = concat_args(recv_strings, size , &cmd);

            add_cmd(liste,cmd);

            //print_liste(liste);

            f = fopen("/tmp/tasks.txt", "a");
            struct flock lock;
            memset(&lock, 0, sizeof(lock));
            lock.l_type = F_RDLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = 0;
            lock.l_len = 0;

            fcntl((int) f, F_SETLKW, &lock);
            fprintf(f,"%s\n",result);

            lock.l_type = F_UNLCK;
            fclose(f);


            /*for (int i = 0; recv_strings[i] != NULL; i++) {
                free(recv_strings[i]);
            }*/

            free(result);
            //free(recv_strings);

            sigusr1_received = 0;
        }

        /* Signal SIGALRM reçu */
        if(alarm_received){
            printf("NUM commande : %d\n", exec_commande->num);
            fprintf(stdout,"Start : %d\n",exec_commande->start);
            exec_commande->start = exec_commande->start + exec_commande->period;
            fprintf(stdout,"New start : %d\n",exec_commande->start);
            pid_t pid = fork();

            if(pid == 0){
                char file_out[32];
                sprintf(file_out,"/tmp/task/%d.out",exec_commande->num);
                FILE *fileOut = freopen(file_out,"a",stdout);

                char file_err[32];
                sprintf(file_err,"/tmp/task/%d.err",exec_commande->num);
                FILE *fileErr = freopen(file_err,"a",stderr);

                // Redirection de la sortie standard vers le fichier .out
                int fd_out = fileno(fileOut);
                if (dup2(fd_out, 1) == -1) {
                    perror("Erreur lors de la redirection de la sortie standard");
                    exit(1);
                }

                // Redirection de la sortie d'erreur standard vers le fichier .err
                int fd_err = fileno(fileErr);
                if (dup2(fd_err, 2) == -1) {
                    perror("Erreur lors de la redirection de la sortie d'erreur standard");
                    exit(1);
                }



                execvp(exec_commande->argv[0],exec_commande->argv);
                perror("execvp");
                exit(1);
            }
            alarm_received =0;

        }

        /* Signal d'interruption reçu */
        if(suppression_received){
            supprimer_liste(liste);
            //free(result);

            close(fd);
            // Suppression du tube
            unlink("/tmp/tasks.fifo");

            // Suppression de tasd.pid
            if(remove("/tmp/taskd.pid") == -1){
                perror("Error: remove taskd.pid");
                exit(1);
            }

            // Suppression de tasd.pid
            if(remove("/tmp/tasks.txt") == -1){
                perror("Error: remove taskd.txt");
                exit(1);
            }

            free(exec_commande);
            return 1;
        }

        sleep(1);
    }
}