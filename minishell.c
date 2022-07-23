#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include "readcmd.h"
#include <signal.h>
#include <errno.h>
#include "job.h"

listeProcessus lprocessus;
int pidFils = -1;

/** Le signal handler permettant le suivi des processus fils
 * @param sig le signal envoyé
 */
void handleSIGCHLD(int sig) {

    int etat;
    int pid;

    do {

        // On utilise WNOHANG pour ne pas bloquer le processus parent
        // On utilise WUNTRACED pour ne pas attendre la fin du processus fils
        // On utilise WCONTINUED pour ne pas bloquer le processus parent si le processus fils est en pause
        pid = (int) waitpid(-1, &etat, WNOHANG | WUNTRACED | WCONTINUED);

        // Si waitpid renvoie -1 et que errno ne vaut pas ECHILD, c'est qu'il y a eu une erreur
        if (pid == -1 && errno != ECHILD) {

            perror("waitpid");
            exit(EXIT_FAILURE);

        }
        else if (pid > 0) {

            if (WIFSTOPPED(etat)) {

                printf("SIGSTOP reçu pour le processus %d\n", pid);

                int id = pidToId(pid, lprocessus);
                char* cmd = pidToCmd(id, lprocessus);
                modifierEtat(pid, SUSPENDU, &lprocessus);
                printf("[%d] + %d suspendu  %s\n", id, pid, cmd);

            } 
            else if (WIFCONTINUED(etat)) {

                int id = pidToId(pid, lprocessus);
                char* cmd = pidToCmd(id, lprocessus);
                modifierEtat(pid, ACTIF, &lprocessus);
                printf("[%d] + %d relancé  %s\n", id, pid, cmd);

            }
            else if (WIFEXITED(etat)) {
                
                supprimer(pid, &lprocessus);

            }
            else if (WIFSIGNALED(etat)) {

                int id = pidToId(pid, lprocessus);
                char* cmd = pidToCmd(id, lprocessus);
                supprimer(pid, &lprocessus);
                printf("[%d] + %d terminé  %s\n", id, pid, cmd);

            }

        }
    } while (pid > 0);
}

/** Le signal handler pour les envois de CTRL+C
 * @param sig le signal envoyé
 */
void handleSIGINT(int sig) {
    if (lprocessus != NULL && pidFils != 1) {
        kill(pidFils, SIGKILL);
    }
}

/** Le signal handler pour les envois de CTRL+Z
 * @param sig le signal envoyé
 */
void handleSIGTSTP(int sig) {
    if (lprocessus != NULL && pidFils != 1) {
        kill(pidFils, SIGSTOP);
        printf("SIGSTOP sent to %d\n", pidFils);
    }
}

/** Attend la fin du processus fils.
 * @param pid PID du processus fils.
 */
void waitForegroundedProcess(int pid) {

    int etat;

    waitpid(pid, &etat, 0);

    if (WIFEXITED(etat)) {
        supprimer(pid, &lprocessus);
    }
    else if (WIFSIGNALED(etat)) {
        supprimer(pid, &lprocessus);
    }
    else if (WIFSTOPPED(etat)) {
        modifierEtat(pid, SUSPENDU, &lprocessus);
    }
    else if (WIFCONTINUED(etat)) {
        modifierEtat(pid, ACTIF, &lprocessus);
    }

}

/** Met en avant plan le processus d'id id
 * si le processus n'est pas trouvé
 * @param id ID du processus.
 */
void setForeground(int id) {
    int pid = idToPid(id, lprocessus);
    if (pid == -1) {
        printf("Erreur : aucun processus ne porte l'id demandé \n");
    }
    else {
        kill(pid, SIGCONT);
        waitForegroundedProcess(pid);
    }
}

/** Met en arrière plan le processus d'id id
 * @param id ID du processus.
 */
void setBackground(int id) {
    int pid = idToPid(id, lprocessus);
    if (pid == -1) {
        printf("Erreur : aucun processus ne porte l'id demandé \n");
    }
    else {
        kill(pid, SIGCONT);
    }
}

/** Stop le processus d'id id
 * @param id ID du processus.
 */
void stop(int id) {
    int pid = idToPid(id, lprocessus);
    if (pid == -1) {
        printf("Erreur : aucun processus ne porte l'id demandé \n");
    }
    else {
        kill(pid, SIGSTOP);
    }
}

/** Affiche une ligne conviviale pour demander une commande
 */
void afficherPrompt() {
    char* user = getenv("USER");
    char* host = getenv("HOSTNAME");
    char* cwd = getcwd(NULL, 0);
    if (host == NULL) {
        host = "";
    }
    printf("%s@%s | %s$ ", user, host, cwd);
}

char* seqToCmd(char** seq) {
    int nbChar = 0;
    for (int i = 0; seq[i] != NULL; i++) {
        nbChar += strlen(seq[i]) + 1;
    }
    char* cmd = malloc(sizeof(char) * nbChar);

    int j = 0;
    for (int i = 0; seq[i] != NULL; i++) {
        for (int k = 0; seq[i][k] != '\0'; k++) {
            cmd[j] = seq[i][k];
            j++;
        }
        cmd[j] = ' ';
        j++;
    }
    cmd[j - 1] = '\0';
    return cmd;
}

/** Le programme principal.
 * @param argc Nombre d'arguments.
 * @param argv Tableau des arguments.
 */
int main(int argc, char *argv[]) {

    /* Sortie de la boucle */
    bool sortie = false;

    /* La ligne de commande */
    struct cmdline *commande;

    /* Le contrôle des jobs */
    int nbJob = 0;
    initialiser(&lprocessus);

    /* Les signal handler */
    signal(SIGINT, handleSIGINT);
    signal(SIGTSTP, handleSIGTSTP);
    signal(SIGCHLD, handleSIGCHLD);

    /* Les signaux à bloquer pour le fils principal */
    sigset_t ens;
	sigemptyset(&ens);
	sigaddset(&ens, SIGINT);
	sigaddset(&ens, SIGTSTP);

    do {

        afficherPrompt();
        commande = readcmd();

        // On vérifie si on doit quitter
        if (commande == NULL || strcmp(commande->seq[0][0], "exit") == 0) {
            sortie = true;
        }

        // Commande interne cd : permet de changer de répertoire
        else if (strcmp(commande->seq[0][0], "cd") == 0) {
            if (commande->seq[0][1] == NULL) {
                chdir(getenv("HOME"));
            }
            else if (chdir(commande->seq[0][1]) == -1) {
                perror("Erreur de changement de répertoire");
            }
        }

        // Commande interne lj : permet de lister les processus lancés depuis le minishell et non terminés
        else if (strcmp(commande->seq[0][0], "lj") == 0) {
            afficher(lprocessus);
        }

        // Commande interne sj : permet de suspendre un job
        else if (strcmp(commande->seq[0][0], "sj") == 0) {
            int id;
            if (commande->seq[0][1] == NULL) {
                printf("Un identifiant de processus est requis \n");
            }
            else {
                id = atoi(commande->seq[0][1]);
                stop(id);
            }
        }

        // Commande interne bg : permet de reprendre en arrière-plan un job suspendu
        else if (strcmp(commande->seq[0][0], "bg") == 0) {
            int id;
            if (commande->seq[0][1] == NULL) {
                printf("Un identifiant de processus est requis \n");
            }
            else {
                id = atoi(commande->seq[0][1]);
                setBackground(id);
            }
        }

        // Commande interne fg : permet de poursuivre en avant-plan un job suspendu ou en arrière-plan
        else if (strcmp(commande->seq[0][0], "fg") == 0) {
            int id;
            if (commande->seq[0][1] == NULL) {
                printf("Un identifiant de processus est requis \n");
            }
            else {
                id = atoi(commande->seq[0][1]);
                setForeground(id);
            }
        }

        // Les commandes externes
        else {

            pidFils = fork();
            if (pidFils < 0) {

                perror("fork");
                exit(EXIT_FAILURE);

            }
            if (pidFils == 0) {

                // Blocage des signaux pour le fils 
                // On considère que le fils ne démasque pas les signaux
                sigprocmask(SIG_BLOCK, &ens, NULL);

                // Exectuer la commande
                execvp(commande->seq[0][0], commande->seq[0]);
                perror("execvp");
                exit(EXIT_FAILURE);

            }

            else {

                nbJob++;
                char* cmd = seqToCmd(commande->seq[0]);
                ajouter(nbJob, pidFils, ACTIF, cmd, &lprocessus);

                if (commande->backgrounded == NULL) {
                    waitForegroundedProcess(pidFils);
                }

            }
        }
    } while (!sortie);
    exit(EXIT_SUCCESS);
}
