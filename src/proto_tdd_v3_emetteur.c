/*************************************************************
* proto_tdd_v3 -  émetteur                                   *
* TRANSFERT DE DONNEES  v3                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* V. Dubuisson - Univ. de Toulouse III - Paul Sabatier       *
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
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
    int inf = 0; /* borne inférieur */
    int taille_fe = 7; /* taille de la fenêtre d'emission */
    int evt;

    /* TODO finir */
    /*if (argc == 2) {
        taille_fe = atoi(argv[1]);
        if ((int)argv[1] != taille_fe || taille_fe <= 0 || taille_fe >= MODULO_V3) {
            printf("%sVous devez rentrer un nombre entre 1 et 15 pour la fenêtre d'emission%s\n", RED, NRM);
            return 1;
        }

    } else if (argc > 2) {
        printf("%sUsage: ./bin/emetteur (int)taille_fe %s\n", RED, NRM);
        return 1;
    }*/

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

        /* Si le paquet à envoyer est dans la fenêtre d'emission */
        if (dans_fenetre(inf, num_pp, taille_fe)) {
            /* remise à la couche reseau */
            vers_reseau(&p_data);

            /* Si le paquet à envoyer est le premier de la fenêtre d'emission */
            if (inf == num_pp) {
              /* départ du temporisateur*/
              depart_temporisateur(TEMPO_TIME_V3);
            }
            /* incrementation du numero de paquet (le curseur de la fenêtre)*/
            num_pp = inc(MODULO_V3, num_pp);

        /* Si le paquet à envoyer n'est pas dans la fenêtre d'emission */
        } else {
            /* on attend de recevoir un acquittement ou la fin du timer */
            evt = attendre();

            /* si on a reçu un acquittement */
            if (evt == -1) {
                /* récupération du paquet d'acquittement */
                de_reseau(&p_ack);

                /* si l'acquittement reçu est celui attendu */
                if (p_ack.num_seq == num_ack) {
                    /* incrementation du numero de ack à recevoir */
                    num_ack = inc(MODULO_V3, num_ack);
                    /* décalage de la fenêtre d'emission */
                    inf = num_ack;

                /* sinon si l'ack reçu correspond à un paquet dans la fe */
                } else if (p_ack.num_seq > num_ack
                  && dans_fenetre(inf, num_ack, taille_fe)) {
                    /* incrementation du numero du prochain ack à recevoir */
                    num_ack = inc(MODULO_V3, p_ack.num_seq);
                    /* décalage de la fenêtre d'emission */
                    inf = num_ack;
                }

            /* sinon, le temporisateur s'est arrêté */
            } else {
                /* TODO réémission jusqu'au curseur voir feuille TD */

            }





        }


        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
