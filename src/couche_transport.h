#ifndef __COUCHE_TRANSPORT_H__
#define __COUCHE_TRANSPORT_H__

#include <stdint.h> /* uint8_t */
#include <stdbool.h>
#define MAX_INFO 96
#define RED "\x1B[31m"
#define NRM "\x1B[0m"
#define MODULO_V2 3
#define MODULO_V3 16
/* en milliseconde */
#define TEMPO_TIME_V2 200
#define TEMPO_TIME_V3 1000

/*************************
* Structure d'un paquet *
*************************/

typedef struct paquet_s {
    uint8_t type;         /* type de paquet, cf. ci-dessous */
    uint8_t num_seq;      /* numéro de séquence */
    uint8_t lg_info;      /* longueur du champ info */
    uint8_t somme_ctrl;   /* somme de contrôle */
    unsigned char info[MAX_INFO];  /* données utiles du paquet */
} paquet_t;

/******************
* Types de paquet *
******************/
#define DATA          1  /* données de l'application */
#define ACK           2  /* accusé de réception des données */
#define NACK          3  /* accusé de réception négatif */
#define CON_REQ       4  /* demande d'établissement de connexion */
#define CON_ACCEPT    5  /* acceptation de connexion */
#define CON_REFUSE    6  /* refus d'établissement de connexion */
#define CON_CLOSE     7  /* notification de déconnexion */
#define CON_CLOSE_ACK 8  /* accusé de réception de la déconnexion */
#define OTHER         9  /* extensions */


/* Capacite de numerotation */
#define SEQ_NUM_SIZE 8

/* ************************************** */
/* Fonctions utilitaires couche transport */
/* ************************************** */

/*
 * Genere la somme de controle du paquet en parametres
 * Paramètres (en entrée):
 *  - paquet : paquet sur lequel on doit faire la somme de controle
 * Renvoie :
 *  - somme : somme de controle generé
 */
uint8_t generer_controle(paquet_t paquet);

/*
 * Verifie la somme de controle sur un paquet
 * Paramètres (en entrée):
 *  - donnees : données à remonter à l'application
 *  - taille_msg : taille des données en octets
 * Renvoie :
 *    -> 1 si le récepteur n'a plus rien à écrire (fichier terminé)
 *    -> 0 sinon
 */
bool verifier_controle(paquet_t paquet);

/*
 * Incremente le numero du prochain paquet en fonction du modulo
 * Paramètres (en entrée):
 *  - modulo : modulo du numero du paquet
 *  - num_pp : numero du paquet actuel
 * Renvoie :
 *    - numero du prochain paquet
 */
int inc(int modulo, int numpp);

/*--------------------------------------*
* Fonction d'inclusion dans la fenetre *
*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille);

#endif
