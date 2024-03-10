#include <stdio.h>
#include <stdbool.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */


/*
 * Genere la somme de controle du paquet en parametres
 * Paramètres (en entrée):
 *  - paquet : paquet sur lequel on doit faire la somme de controle
 * Renvoie :
 *  - somme : somme de controle generé
 */
uint8_t generer_controle(paquet_t paquet) {

    uint8_t somme;

    /* calcul de la somme de controle sur l'en-tete du paquet */
    somme = paquet.type ^ paquet.num_seq ^ paquet.lg_info;

    /* calcul de la somme de controle sur la data du paquet */
    for (int i = 0; i < MAX_INFO; i++) {
        somme ^= paquet.info[i];
    }

    return somme;
}

/*
 * Verifie la somme de controle sur un paquet
 * Paramètres (en entrée):
 *  - donnees : données à remonter à l'application
 *  - taille_msg : taille des données en octets
 * Renvoie :
 *    -> 1 si le récepteur n'a plus rien à écrire (fichier terminé)
 *    -> 0 sinon
 */
bool verifier_controle(paquet_t paquet) {

    //récupération de la somme de controle
    uint8_t somme1 = paquet.somme_ctrl;

    /* calcul de la somme de controle sur l'en-tete du paquet */
    uint8_t somme2 = paquet.type ^ paquet.num_seq ^ paquet.lg_info;

    /* calcul de la somme de controle sur la data du paquet */
    for (int i = 0; i < MAX_INFO; i++) {
        somme2 ^= paquet.info[i];
    }

    return somme1 == somme2;
}

/*
 * Incremente le numero du prochain paquet en fonction du modulo
 * Paramètres (en entrée):
 *  - modulo : modulo du numero du paquet
 *  - num_pp : numero du paquet actuel
 * Renvoie :
 *    - numero du prochain paquet
 */
int inc(int modulo, int num_pp) {
    num_pp++;
    return num_pp % modulo;
}


/* ===================== Fenêtre d'anticipation ============================= */

/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}
