/*************************************************************
* proto_tdd_v2 -  récepteur                                  *
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
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t p_data, p_ack; /* paquet utilisé par le protocole */
    int fin = 0; /* condition d'arrêt */
    int num_pp = 0; /* numero du prochain paquet à recevoir */
    int num_p_ack = 0; /* numero du prochain ack à envoyer */


    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        /* récupération du paquet de data */
        de_reseau(&p_data);

        /* si pas d'erreur dans la paquet reçu */
        if (verifier_controle(p_data)) {

            /* initialisation paquet d'acquittement positif */
            p_ack.type = ACK;

            /* si le paquet reçu est le prochain à recevoir */
            if (p_data.num_seq == num_pp) {

                /* extraction des donnees du paquet recu */
                for (int i=0; i<p_data.lg_info; i++) {
                    message[i] = p_data.info[i];
                }
                /* remise des données à la couche application */
                fin = vers_application(message, p_data.lg_info);

                p_ack.num_seq = num_p_ack;
                num_p_ack = inc(MODULO, num_p_ack);

            /* sinon */
            } else {
                /* Le paquet reçu est dupliqué */
                printf("%s[TRP] Paquet dupliqué.%s\n", RED, NRM);
            }

            /* remise à la couche reseau de l'acquittement */
            vers_reseau(&p_ack);

            /* incrémentation du numéro de séquence du prochain paquet à recevoir */
            num_pp = inc(MODULO, num_pp);

        /* sinon */
        } else {
            /* Erreur donc aucun acquittement n'est envoyé */
            printf("%s[TRP] Erreur détecté dans la paquet.%s\n", RED, NRM);
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
