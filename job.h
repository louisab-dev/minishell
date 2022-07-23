#ifndef __JOB_H
#define __JOB_H

/** Etat des processus */
enum etat {SUSPENDU, ACTIF};
typedef enum etat etat;

/** Une cellule contenant un processus */
typedef struct Cellule Cellule;

/** La liste des processus */
typedef Cellule* listeProcessus;

/* Cellule contenant un processus */
struct Cellule {
	int id;			// id du processus
	int pid;		// pid du processus
	etat status;			// etat du processus
	char * commande;		// ligne exécutée
    listeProcessus suivante; // processus suivant
};

/** Obtenir une chaine vide de processus
 * @param processus la liste des processus
 */
void initialiser(listeProcessus * processus);

/** Ajouter un élément dans la liste des jobs
 * @param id id du processus
 * @param pid pid du processus
 * @param status etat du processus
 * @param commande ligne exécutée
 * @param processus la liste des processus
 */
void ajouter(int id, int pid, etat status, char * commande, listeProcessus * processus);

/** Supprimer un élément de la liste des jobs à partir de son pid
 * @param pid pid du processus à supprimer
 * @param processus la liste des processus
 */
void supprimer(int pid, listeProcessus * processus);

/** Changer l'état d'un processus de la liste des jobs à partir de son pid
 * @param pid pid du processus à modifier
 * @param status nouvel état du processus
 * @param processus la liste des processus
 */
void modifierEtat(int pid, etat status, listeProcessus * processus);

/** Obtenir le pid d'un processus depuis son id 
 * @param id id du processus
 * @param processus la liste des processus
*/
int idToPid(int id, listeProcessus processus);

/** Obtenir l'id d'un processus depuis son pid 
 * @param pid pid du processus
 * @param processus la liste des processus
*/
int pidToId(int pid, listeProcessus processus);

/** Obtenir la commande d'un processus depuis son pid 
 * @param pid pid du processus
 * @param processus la liste des processus
*/
char* pidToCmd(int pid, listeProcessus processus);

/** Afficher la liste de processus 
 * @param processus la liste des processus
*/
void afficher(listeProcessus processus);

#endif
