#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "job.h"

/** Obtenir une chaine vide de processus
 * @param listeProcessus la liste de processus
 */
void initialiser(listeProcessus * processus) {
    *processus = NULL;
}

/** Ajouter un élément dans la liste des jobs 
 * @param id id du processus
 * @param pid pid du processus
 * @param status etat du processus
 * @param commande ligne exécutée
 * @param processus la liste des processus
*/
void ajouter(int id, int pid, etat status, char * commande, listeProcessus * processus) {
    
    listeProcessus nouveau;
	nouveau = malloc(sizeof(struct Cellule));

	// Affectation
	nouveau->commande = commande;
	nouveau->pid = pid;
	nouveau->id = id;
    nouveau->status = status;
	nouveau->suivante = *processus;

	// nouvelle liste composée du nouveau maillon
	*processus = nouveau;
}

/** Supprimer un élément de la liste des jobs à partir de son pid
 * @param pid pid du processus à supprimer
 * @param processus la liste des processus
 */
void supprimer(int pid, listeProcessus * processus) {
    listeProcessus courant, precedent;
    courant = *processus;
    precedent = NULL;
    while (courant != NULL && courant->pid != pid) {
        precedent = courant;
        courant = courant->suivante;
    }
    if (courant != NULL) {
        if (precedent == NULL) {
            *processus = courant->suivante;
        } else {
            precedent->suivante = courant->suivante;
        }
        free(courant);
    }
}

/** Changer l'état d'un processus de la liste des jobs à partir de son pid
 * @param pid pid du processus à modifier
 * @param status nouvel état du processus
 * @param processus la liste des processus
 */
void modifierEtat(int pid, etat status, listeProcessus * processus) {
    listeProcessus courant;
    courant = *processus;
    while (courant != NULL && courant->pid != pid) {
        courant = courant->suivante;
    }
    if (courant != NULL) {
        courant->status = status;
    }
}

/** Obtenir le pid d'un processus depuis son id
 * @param id id du processus
 * @param processus la liste des processus
 */
int idToPid(int id, listeProcessus processus) {
    listeProcessus courant;
    courant = processus;
    while (courant != NULL && courant->id != id) {
        courant = courant->suivante;
    }
    if (courant != NULL) {
        return courant->pid;
    }
    return -1;
}


/** Obtenir l'id d'un processus depuis son pid
 * @param pid pid du processus
 * @param processus la liste des processus
 */
int pidToId(int pid, listeProcessus processus) {
    listeProcessus courant;
    courant = processus;
    while (courant != NULL && courant->pid != pid) {
        courant = courant->suivante;
    }
    if (courant != NULL) {
        return courant->id;
    }
    return -1;
}

/** Obtenir la commande utilisée pour lancer le processus
 * @param id id du processus
 * @param processus la liste des processus
 */
char* pidToCmd(int pid, listeProcessus processus) {
    listeProcessus courant;
    courant = processus;
    while (courant != NULL && courant->pid != pid) {
        courant = courant->suivante;
    }
    if (courant != NULL) {
        return courant->commande;
    }
    return NULL;
}

/** Afficher la liste de processus 
 * @param processus la liste des processus
*/
void afficher(listeProcessus processus) {
    listeProcessus courant;
    char *status;
    courant = processus;
    while (courant != NULL) {

        if (processus->status == SUSPENDU) {
            status = "suspendu";
        }
        else {
            status = "actif";
        }
        printf("%d\t%d\t%s\t%s\n", courant->id, courant->pid, status, courant->commande);
        courant = courant->suivante;
    }
}
