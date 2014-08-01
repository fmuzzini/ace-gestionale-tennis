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
circolo_t *inizializza_circolo(char nome[], char indirizzo[], char email[], int telefono);

/** Aggiunge un giocatore al Circolo.
 * Crea un nuovo socio con i dati passati e lo aggancia alla lista soci del circolo
 * @param[in] nome Nome del giocatore
 * @param[in] cognome Cognome del giocatore
 * @param[in] nascita Data di nascita del giocatore nel formato (gg/mm/aaaa)
 * @param[in] tessera Numero di tessera del giocatore
 * @param[in] telefono Numero di telefono del giocatore
 * @param[in] email Email del giocatore
 * @param[in,out] circolo Circolo al quale aggiungere il giocatore, viene passato per riferimento
 * @return puntatore al giocatore appena creato, 0 in caso di fallimento
 */
giocatore_t *aggiungi_giocatore(char nome[], char cognome[], char nascita[],
				int tessera, int telefono, char email[], float classifica,
				circolo_t *circolo);

/** Aggiunge un socio al Circolo.
 * Crea un nuovo socio con i dati passati e lo aggancia alla lista soci del circolo
 * @param[in] nome Nome del socio
 * @param[in] cognome Cognome del socio
 * @param[in] nascita Data di nascita del socio nel formato (gg/mm/aaaa)
 * @param[in] tessera Numero di tessera del socio
 * @param[in] telefono Numero di telefono del socio
 * @param[in] email Email del socio
 * @param[in] retta Stato del pagamento della retta
 * @param[in,out] circolo Circolo al quale aggiungere il socio, viene passato per riferimento
 * @return successo (TRUE) o fallimento (FALSE)
 */
giocatore_t *aggiungi_socio	(char nome[], char cognome[], char nascita[],
				int tessera, int telefono, char email[], float classifica,
				bool retta, circolo_t *circolo);

/** Aggiunge un campo al Circolo.
 * Crea un nuovo campo con i dati passati e lo aggancia alla lista campi del circolo
 * @param[in] numero Numero identificativo del campo
 * @param[in] copertura Tipo di copertura (INDOOR, OUTDOOR)
 * @param[in] terreno Tipo di terreno
 * @param[in] note Eventuali note
 * @param[in,out] circolo Circolo al quale aggiungere il campo;
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool aggiungi_campo(int numero, copertura_t copertura, terreno_t terreno, char note[], circolo_t *circolo);

/** Elimina il socio dal Circolo.
 * Elimina il socio passato come parametro rendendolo un giocatore normale
 * In altri termini declassa il socio a giocatore
 * @param[in] socio Puntatore al socio da eliminare
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool elimina_socio(giocatore_t *socio);

/** Elimina il giocatore dal Circolo
 * Elimina i giocatore passato come parametro dal circolo
 * @param[in,out] giocatore Giocatore da eliminare
 * @param[in,out] circolo Circolo dal quale eliminare il giocatore
 */
bool elimina_giocatore(giocatore_t *&giocatore, circolo_t *circolo);

/** Elimina il campo dal Circolo.
 * Elimina il campo con tutte le ore a lui associate dal circolo
 * @param[in,out] campo Campo da eliminare
 * @param[in,out] circolo Circolo dal quale eliminare il campo
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool elimina_campo(campo_t *&campo, circolo_t *circolo);

/** Elimina l'ora dal campo.
 * Elimina l'ora passata come parametro dal campo
 * @param[in,out] ora Ora da eliminare
 * @param[in,out] campo Campo dal quale eliminare l'ora
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool elimina_ora(ora_t *&ora, campo_t *campo);

/* Fine interfaccia del modulo accesso_dati */

#endif
