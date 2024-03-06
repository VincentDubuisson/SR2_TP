#include <stdio.h>
#include <stdbool.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */


/*
 * Paramètres (en entree):
 *   - paquet :
 */
uint8_t generer_controle(paquet_t paquet) {

    uint8_t somme;

    somme = paquet.type ^ paquet.num_seq ^ paquet.lg_info;

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

    uint8_t somme2 = paquet.type ^ paquet.num_seq ^ paquet.lg_info;

    for (int i = 0; i < MAX_INFO; i++) {
        somme2 ^= paquet.info[i];
    }

    if (somme1 == somme2) {
      return true;
    }
    return false;
}

int inc(int modulo, int numpp) {
    numpp++;
    return numpp % modulo;
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
