/*************************************************************
* proto_tdd_v1 -  récepteur                                  *
* TRANSFERT DE DONNEES  v1                                   *
*                                                            *
* Protocole « Stop-and-Wait » avec acquittement négatif,     *
*  mode non connecté                                         *
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

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&p_data);

        /* si pas d'erreur dans la paquet reçu */
        if (verifier_controle(p_data)) {

            /* extraction des donnees du paquet recu */
            for (int i=0; i<p_data.lg_info; i++) {
                message[i] = p_data.info[i];
            }
            /* remise des données à la couche application */
            fin = vers_application(message, p_data.lg_info);

            /* initialisation paquet d'acquittement positif */
            p_ack.type = ACK;

        /* sinon */
        } else {
            /* Erreur donc aucun acquittement n'est envoyé */
            printf("%s[TRP] Erreur détecté dans la paquet.%s\n", RED, NRM);
            /* initialisation paquet d'acquittement négatif */
            p_ack.type = NACK;
        }

        /* remise à la couche reseau de l'acquittement */
        vers_reseau(&p_ack);
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
