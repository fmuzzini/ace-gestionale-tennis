/**
 * @file
 * File contenente l'interfaccia del modulo accesso_dati.cc
 */

#ifndef ACCESSO_DATI
#define ACCESSO_DATI

#include "struttura_dati.h"

/* Inizio interfaccia del modulo accesso_dati */

/** Inizializza il Circolo con i dati della società.
 * Crea una struttura di tipo circolo_t con i dati passati come parametri e 
 * la ritorna
 * @param[in] nome Nome del Circolo
 * @param[in] indirizzo Indirizzo del Circolo
 * @parma[in] email Email del Circolo
 * @param[in] telefono Telefono del Circolo
 * @return Struttura appena creata
 */
circolo_t inizializza_circolo(char nome[], char indirizzo[], char email[], int telefono);

/** Aggiunge un socio al Circolo.
 * Crea un nuovo socio con i dati passati e lo aggancia alla lista soci del circolo
 * @param[in] nome Nome del socio
 * @param[in] cognome Cognome del socio
 * @param[in] nascita Data di nascita del socio nel formato (gg/mm/aaaa)
 * @param[in] tessera Numero di tessera del socio
 * @param[in] telefono Numero di telefono del socio
 * @param[in] email Email del socio
 * @param[in,out] circolo Circolo al quale aggiungere il socio, viene passato per riferimento
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool aggiungi_socio(char nome[], char cognome[], char nascita[], int tessera, int telefono, char email[], circolo_t &circolo);

/** Aggiunge un campo al Circolo.
 * Crea un nuovo campo con i dati passati e lo aggancia alla lista campi del circolo
 * @param[in] numero Numero identificativo del campo
 * @param[in] copertura Tipo di copertura (INDOOR, OUTDOOR)
 * @param[in] terreno Tipo di terreno
 * @param[in] note Eventuali note
 * @param[in,out] circolo Circolo al quale aggiungere il campo;
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool aggiungi_campo(int numero, copertura_t copertura, terreno_t terreno, char note[], circolo_t &circolo);

/** Elimina il socio dal Circolo.
 * Elimina il socio passato come parametro e crea un giocatore con gli stessi dati.
 * @param[in] socio Socio da eliminare (Sotto forma di elemento GSList*)
 * @param[in,out] circolo Circolo al quale eliminare il socio
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool elimina_socio(GSList *socio, circolo_t &circolo);

/* Fine interfaccia del modulo accesso_dati */

#endif
