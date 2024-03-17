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
    int num_pp = 0; /* numéro du prochain paquet à envoyer */
    int num_ack = 0; /* numéro du prochain ack à recevoir */
    int evt;

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
        p_data.num_seq = num_pp; /* initialisation du numéro de séquence */
        /* initialisation de la somme de controle */
        p_data.somme_ctrl = generer_controle(p_data);

        do {
            /* remise à la couche reseau */
            vers_reseau(&p_data);

            /* départ du temporisateur*/
            depart_temporisateur(TEMPO_TIME_V2);

            /* on attend de recevoir un acquittement ou la fin du timer */
            evt = attendre();

            /* Tant que l'on ne reçoit pas de paquet acquittement ou fin du timer */
            int stop = 0;
            while (evt != -1) {

                /* remise à la couche reseau */
                vers_reseau(&p_data);

                /* départ du temporisateur*/
                depart_temporisateur(TEMPO_TIME_V2);

                /* on attend de recevoir un acquittement ou la fin du timer */
                evt = attendre();

                if (stop == 20) {/* si perte du dernier ack */
                    printf("%s[TRP] Recepteur inactif !%s\n", RED, NRM);
                    return 1;
                }
                stop++;
            }

            /* récupération du paquet d'acquittement */
            de_reseau(&p_ack);

            /* arret du temporisateur */
            arret_temporisateur();

        } while (p_ack.num_seq != num_ack);


        /* incrémentation du numéro de séquence du paquet à émettre avec modulo*/
        num_pp = inc(MODULO_V2, num_pp);
        num_ack = inc(MODULO_V2, num_ack);

        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);


    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
