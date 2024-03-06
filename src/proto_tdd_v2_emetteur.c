/*************************************************************
* proto_tdd_v2 -  émetteur                                   *
* TRANSFERT DE DONNEES  v2                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* V. Dubuisson - Univ. de Toulouse III - Paul Sabatier       *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t p_data, p_ack; /* paquet utilisé par le protocole */
    int numpp = 0; /* numéro du prochain paquet à envoyer */
    int evt;
    int tempo_time = 500; /* temps du temporisateur en ms*/

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            p_data.info[i] = message[i];
        }
        p_data.lg_info = taille_msg; /* initialisation de la taille du msg */
        p_data.type = DATA; /* initialisation du type du paquet */
        p_data.num_seq = numpp; /* initialisation du numéro de séquence */
        /* initialisation de la somme de controle */
        p_data.somme_ctrl = generer_controle(p_data);

        /* remise à la couche reseau */
        vers_reseau(&p_data);

        /* départ du temporisateur*/
        depart_temporisateur(tempo_time);

        /* on attend de recevoir un acquittement ou la fin du timer */
        evt = attendre();

        /* Tant que l'on ne reçoit pas de paquet acquittement */
        while (evt != -1) {

            /* remise à la couche reseau */
            vers_reseau(&p_data);

            /* départ du temporisateur*/
            depart_temporisateur(tempo_time);

            /* on attend de recevoir un acquittement ou la fin du timer */
            evt = attendre();
        }

        /* récupération du paquet d'acquittement */
        de_reseau(&p_ack);

        /* arret du temporisateur */
        arret_temporisateur();

        /* incrémentation du numéro de séquence du paquet à émettre */
        numpp = inc(2, numpp);

        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);

    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
