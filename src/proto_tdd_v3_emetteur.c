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
    int curseur = 0; /* curseur de la fenêtre d'emission */
    int num_ack = 0; /* numéro de l'acquittement à recevoir */
    int inf = 0; /* borne inférieur de la fenetre d'emission */
    int taille_fe = 4; /* taille de la fenêtre d'emission par défaut*/
    int fin = 0; /* nombre de réémission consecutive de la fenetre */
    int p_num_ack = 0; /* numero de l'acquittement précedent recu */


    /* vérification des parametres d'entrée du programme */
    if (argc == 2) {
        taille_fe = atoi(argv[1]);

        /* si la taille de la fenetre est valide */
        if (taille_fe <= 0 || taille_fe >= MODULO_V3) {
            printf("%sVous devez rentrer un nombre entre 1 et 15 pour la fenêtre d'emission%s\n", RED, NRM);
            return 1;
        }

    /* sinon message d'utilisation d'appel du programme */
    } else if (argc > 2) {
        printf("%sUsage: ./bin/emetteur [taille_fe] %s\n", RED, NRM);
        return 1;
    }

    /* initialisation du tableau de la fenetre d'emission, il va servir pour
      les réémission */
    paquet_t fenetre[taille_fe];

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer et le dernier ack est recu */
    while ( (taille_msg != 0 || fenetre[0].num_seq != 99) && fin <= FIN_BCL) {

        /* Si le paquet à envoyer est dans la fenêtre d'emission */
        if (dans_fenetre(inf, num_pp, taille_fe) && taille_msg != 0) {

            /* construction paquet */
            for (int i=0; i<taille_msg; i++) {
                p_data.info[i] = message[i];
            }
            p_data.lg_info = taille_msg; /* initialisation de la taille du msg */
            p_data.type = DATA; /* initialisation du type du paquet */
            p_data.num_seq = num_pp; /* initialisation du numéro de séquence */
            /* initialisation de la somme de controle */
            p_data.somme_ctrl = generer_controle(p_data);

            /* initialisation de la fenetre d'emission */
            fenetre[curseur] = p_data;

            /* remise à la couche reseau */
            vers_reseau(&p_data);

            /* Si le paquet à envoyer est le dernier de la fenêtre d'emission ou
            le dernier du fichier */
            if (curseur == taille_fe-1) {
              /* départ du temporisateur*/
              depart_temporisateur(TEMPO_TIME_V3);
            }

            /* lecture des donnees suivantes de la couche application */
            de_application(message, &taille_msg);

            if (taille_msg == 0 && curseur != taille_fe-1) {
                /* départ du temporisateur*/
                depart_temporisateur(TEMPO_TIME_V3);
            }

            /* incrementation du numero de paquet (le curseur de la fenêtre)*/
            num_pp = inc(MODULO_V3, num_pp);
            /* incrementation du curseur de la fenetre d'emission */
            curseur++;
            /* rénitialisation du compteur de réémission */
            fin = 0;

        } else {

            /* on attend de recevoir un acquittement ou la fin du timer */
            int evt = attendre();

            /* si on a reçu un acquittement */
            if (evt == -1) {
                /* récupération du paquet d'acquittement */
                de_reseau(&p_ack);

                /* si l'acquittement reçu est celui attendu */
                if (p_ack.num_seq == num_ack) {

                    /* décrémentation du curseur de la fenetre d'emission */
                    curseur--;

                    /* si l'ack recu n'est pas le dernier à recevoir */
                    if (taille_msg != 0 || num_ack != 14) {
                        /* incrementation du numero du prochain ack à recevoir */
                        num_ack = inc(MODULO_V3, num_ack);
                    }

                    /* décalage de la fenetre d'emission */
                    for (int i = 0; i < taille_fe; i++) {
                        fenetre[i] = fenetre[i+1];
                    }
                    fenetre[taille_fe-1].num_seq = 99;

                    /* incrementation du précedent acquittement */
                    p_num_ack = p_ack.num_seq;

              /* sinon si l'ack reçu correspond à un paquet dans la fe */
              } else if (dans_fenetre(inf, p_ack.num_seq, taille_fe) && p_ack.num_seq != p_num_ack) {

                    /* incrementation du numero du prochain ack à recevoir */
                    if (p_num_ack >= 15) {
                        p_num_ack = 0;
                    }
                    num_ack = p_ack.num_seq;

                    /* décrémentation du curseur */
                    curseur -= (num_ack+16) - ((p_num_ack%MODULO_V3)+16);
                    /* incrementation du précedent acquittement */
                    p_num_ack = num_ack;

                    /* décalage de la fenetre d'emission jusqu'à l'ack recu */
                    while (fenetre[0].num_seq != num_ack) {
                        for (int i = 0; i < taille_fe; i++) {
                            fenetre[i] = fenetre[i+1];
                        }
                        fenetre[taille_fe-1].num_seq = 99;
                    }
                    /* décalage de la fenetre pour le prochain ack à recevoir */
                    for (int i = 0; i < taille_fe; i++) {
                        fenetre[i] = fenetre[i+1];
                    }
                    fenetre[taille_fe-1].num_seq = 99;

                    /* incrementation du numero du prochain ack à recevoir */
                    if (taille_msg != 0 || num_ack != 14) {
                        num_ack = inc(MODULO_V3, num_ack);
                    }
                }
                /* si dernier acquittement de la fenetre recu */
                if (fenetre[0].num_seq > 15 || curseur == 0) {
                     arret_temporisateur(); /* arret du temporisateur */
                     inf = num_pp; /* décalage de la borne inférieur de la fenetre */
                     curseur = 0; /* rénitialisation du curseur */
                }

            /* sinon, le temporisateur s'est arrêté */
            } else {

                /* initialisation borne inférieur au dernier ack recu */
                inf = num_ack;

                /* réémission jusqu'au curseur */
                for (int i = 0; i < curseur; i++) {
                  /* remise à la couche reseau */
                  p_data = fenetre[i];
                  vers_reseau(&p_data);
                  /* si dernier paquet à réemettre */
                  if (i == taille_fe-1 || (taille_msg == 0 && fenetre[i].num_seq == 15)) {
                      depart_temporisateur(TEMPO_TIME_V3);
                      fin++;
                  }
                }
            }
        }
    }

    /* empeche boucle infinie de réémission */
    if (fin > FIN_BCL) {
        printf("%s[TRP] Perte de l'acquittement détectée (ou premier paquet perdu/erroné)%s\n", RED, NRM);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
