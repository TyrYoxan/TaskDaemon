#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 
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
#include <signal.h>
#include <time.h>
#include <locale.h>

#define MAX_STRING_LENGTH 1024

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

    // Créer une copie de la commande et l'ajouter à la liste
    Commande *cmd_copy = malloc(sizeof(Commande));
    if (!cmd_copy) {
        perror("Erreur lors de l'allocation de mémoire");
        exit(1);
    }
    *cmd_copy = cmd;
    l->commande[l->size] = cmd_copy;
    l->size++;
}

// Supprimer une commande
void supp_cmd(Liste* l, int index){
    if(index >= 0 && index < l->size){
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
        for (int j = 0; j < cmd->argc; j++) {
            free(cmd->argv[j]);
        }
        free(cmd->argv);
    }

    // Libération de la mémoire occupée par la liste
    free(l->commande);
    l->size = 0;
}

void print_liste(Liste *l) {
    printf("Liste des commandes : \n");
    for(int i=0; i<l->size; i++){
        printf("Commande %d:\n", i+1);
        printf("Numéro : %d\n", l->commande[i]->num);
        printf("Période : %d\n", l->commande[i]->period);
        printf("Début : %s\n", l->commande[i]->start);
        printf("Nombre d'arguments : %d\n", l->commande[i]->argc);
        printf("Arguments : ");
        for (int j = 0; j < l->commande[i]->argc; j++) {
            printf("%s ", l->commande[i]->argv[j]);
        }
        printf("\n\n");
    }
}

/*************  Méthode    *************/

time_t getTime(){
    time_t currentTime = time(NULL);

    if (currentTime == (time_t)-1) {
        perror("Erreur lors de l'appel à time");
        exit(1);
    }

    return currentTime;
}

char* when(int start){
    time_t inputTime = getTime() + start;

    struct tm *timeInfo;
    char *buffer = malloc(80 * sizeof(char));

    setlocale(LC_TIME, "fr_FR.UTF-8");  // Définit le locale en français

    timeInfo = localtime(&inputTime);
    if (timeInfo == NULL) {
        perror("Erreur lors de la conversion de la date");
        exit(1);
    }

    strftime(buffer, 80, "%e %B %C %H:%M:%S", timeInfo);



    return buffer;
}

char* concat_args(char* argv[], int argc, Commande* cmd) {
    // Création de la commande
    cmd->num = command_counter;
    cmd->start = getTime() + atoi(argv[1]);
    cmd->period = atoi(argv[2]);

    cmd->argc = argc - 3;

    char** argvs = malloc((cmd->argc + 1) * sizeof(char*));
    if (!argvs) {
        perror("Erreur lors de l'allocation de mémoire");
        exit(1);
    }
    for(int i = 0; i < cmd->argc; ++i){
        argvs[i] = argv[i + 3];
    }
    argvs[cmd->argc] = NULL;
    cmd->argv = argvs;

    int max_len = 2048 + cmd->argc * 50; // Augmenter la taille maximale pour inclure suffisamment d'espace pour les arguments
    char* str = malloc(max_len * sizeof(char));
    if (!str) {
        perror("Erreur lors de l'allocation de mémoire");
        exit(1);
    }
    printf("Start : %d\n",cmd->start);
    snprintf(str, max_len, "%d;%s;%d; ", cmd->num, when(cmd->start), cmd->period);
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

    // Ajouter le caractère de fin de chaîne
    str[max_len - 1] = '\0';

    ++command_counter;
    return str;
}

time_t convertDateToSeconds(const char* date) {
    struct tm timeInfo;
    memset(&timeInfo, 0, sizeof(struct tm));

    // Configuration du format d'entrée
    const char* format = "%e %B %C %T"; 

    printf("Date : %s\n", date);
    // Analyse de la date donnée
    if (strptime(date,format, &timeInfo) == NULL) {
        fprintf(stderr, "Erreur lors de l'analyse de la date\n");
        exit(1);
    }

    printf("Jour : %d %d %d\n",timeInfo.tm_mday,timeInfo.tm_mon,timeInfo.tm_year);
    printf("Time : %d:%d:%d\n",timeInfo.tm_hour,timeInfo.tm_min,timeInfo.tm_sec);
    // Conversion en nombre de secondes
    time_t seconds = mktime(&timeInfo);
    if (seconds == -1) {
        perror("Erreur lors de la conversion en secondes");
        exit(1);
    }

    return seconds;
}

time_t getNextTime(const Commande* cmd) {
    if (cmd->start == NULL) {
        return 0; // Pas de date spécifiée, exécuter immédiatement
    }

    // Convertir la date en secondes depuis Epoch
    time_t dateInSeconds = cmd->start;
    
   

    return dateInSeconds - time(NULL);
}

int getnextTime(Liste *l, int nbCommande){
    time_t currentTime = getTime();
    time_t minDuree = 3600;
    // Parcourir la liste des commandes
    for (int i = 0; i < nbCommande; i++) {
        time_t duree = getNextTime(l->commande[i]);
        
        if(minDuree > duree){
            minDuree = duree;
        }
        
    }
   
    return minDuree;
}

/*************  Handler    *************/

volatile int sigusr1_received = 0;
volatile int alarm_received = 0;

void handler_sigusr1(int sig){
    printf("SIGUSR1 \n");
    sigusr1_received =1;
}

void handler_alarm(int sig){
    printf("ALARM !!! \n");
    alarm_received =1;
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

    int fd = open("/tmp/tasks.fifo", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    Liste *liste;
    liste = initListe();


    while(1){
        int stop = getnextTime(liste,liste->size);
        //printf("Time : %d\n",getnextTime(liste,liste->size));
        alarm(stop);
        
        if(sigusr1_received){
            char **recv_strings;

            // Réception des chaînes de caractères
            recv_strings = recv_argv(fd);
            int size = 0;
            for (int i = 0; recv_strings[i] != NULL; i++) {
                printf("%s ", recv_strings[i]);
                size++;
            }
            printf("\n");


            Commande cmd = {0};

            char *result = concat_args(recv_strings, size , &cmd);

            add_cmd(liste,cmd);
            //print_liste(liste);

            f = fopen("/tmp/tasks.txt", "a");
            fprintf(f,"%s\n",result);
            fclose(f);


            for (int i = 0; recv_strings[i] != NULL; i++) {
                free(recv_strings[i]);
            }

            free(result);
            free(recv_strings);

            sigusr1_received = 0;
        }

        if(alarm_received){

        }
    }

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