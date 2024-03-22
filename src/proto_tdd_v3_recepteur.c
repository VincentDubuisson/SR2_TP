/*************************************************************
* proto_tdd_v3 -  récepteur                                  *
* TRANSFERT DE DONNEES  v3                                   *
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
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t p_data, p_ack; /* paquet utilisé par le protocole */
    int fin = 0; /* condition d'arrêt */
    int num_ack = 0; /* numero du prochain ack à envoyer */
    int inf = 0; /* borne inférieur */
    int taille_fe = 1; /* taille de la fenêtre d'emission */


    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        /* récupération du paquet de data */
        de_reseau(&p_data);

        printf("paquet recu = %d\n", p_data.num_seq);

        /* si le paquet reçu est sans erreur */
        if (verifier_controle(p_data)) {

            /* initialisation paquet d'acquittement positif */
            p_ack.type = ACK;

            /* si le paquet reçu est le prochain à recevoir */
            if (dans_fenetre(inf, p_data.num_seq, taille_fe)) {

                /* extraction des donnees du paquet recu */
                for (int i=0; i<p_data.lg_info; i++) {
                    message[i] = p_data.info[i];
                }
                /* remise des données à la couche application */
                fin = vers_application(message, p_data.lg_info);

                inf++;

                p_ack.num_seq = num_ack;

                /* remise à la couche reseau de l'acquittement */
                vers_reseau(&p_ack);

                printf("ack envoyé = %d\n", p_ack.num_seq);

                /* incrémentation du numero du paquet d'acquittement */
                num_ack = inc(MODULO_V3, num_ack);

            /* sinon */
            } else {
                /* remise à la couche reseau de l'acquittement */
                vers_reseau(&p_ack);

                printf("ack envoyé = %d\n", p_ack.num_seq);

                /* Hors séquence donc renvoie du dernier ack */
                printf("%s[TRP] Paquet hors séquence %s\n", RED, NRM);
            }

        /* sinon */
        } else {
            /* Erreur donc aucun acquittement n'est envoyé */
            printf("%s[TRP] Erreur détecté dans la paquet %s\n", RED, NRM);
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
