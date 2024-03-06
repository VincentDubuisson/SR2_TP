/*************************************************************
* proto_tdd_v1 -  émetteur                                   *
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
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t p_data, p_ack; /* paquet utilisé par le protocole */

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
        /* initialisation de la somme de controle */
        p_data.somme_ctrl = generer_controle(p_data);

        /* faire */
        do {

            /* remise à la couche reseau */
            vers_reseau(&p_data);

            /* récupération du paquet d'acquittement */
            de_reseau(&p_ack);

      /* tant que l'acquittement est négatif */
      } while ((p_ack.type == NACK));

      /* lecture des donnees suivantes de la couche application */
      de_application(message, &taille_msg);

    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
