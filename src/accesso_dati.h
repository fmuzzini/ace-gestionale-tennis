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
circolo_t *inizializza_circolo(const char nome[], const char indirizzo[], const char email[], const char telefono[]);

/** Aggiunge o modifica un giocatore al Circolo.
 * Crea un nuovo socio con i dati passati e lo aggancia alla lista soci del circolo
 * se gli si passa un vecchio giocatore giocatore modifica i dati di quest'ultimo
 * @param[in] nome Nome del giocatore
 * @param[in] cognome Cognome del giocatore
 * @param[in] nascita Data di nascita del giocatore nel formato (gg/mm/aaaa)
 * @param[in] tessera Numero di tessera del giocatore
 * @param[in] telefono Numero di telefono del giocatore
 * @param[in] email Email del giocatore
 * @param[in] classifica Classifica del giocatore
 * @param[in] circolo_g Circolo di appartenenza del giocatore
 * @param[in,out] vecchio Vecchio giocatore/socio da modificare
 * @param[in,out] circolo Circolo al quale aggiungere il giocatore, viene passato per riferimento
 * @return puntatore al giocatore appena creato, 0 in caso di fallimento
 */
giocatore_t *aggiungi_giocatore(const char nome[], const char cognome[], const char nascita[],
				const char tessera[], const char telefono[], const char email[], const char classifica[],
				const char circolo_g[], giocatore_t *vecchio, circolo_t *circolo);

/** Aggiunge o modifica un socio al Circolo.
 * Crea un nuovo socio con i dati passati e lo aggancia alla lista soci del circolo,
 * se gli si passa un vecchio socio modifica i dati di quest'ultimo
 * @param[in] nome Nome del socio
 * @param[in] cognome Cognome del socio
 * @param[in] nascita Data di nascita del socio nel formato (gg/mm/aaaa)
 * @param[in] tessera Numero di tessera del socio
 * @param[in] telefono Numero di telefono del socio
 * @param[in] email Email del socio
 * @param[in] classifica Classifica del socio
 * @param[in] retta Stato del pagamento della retta
 * @param[in, out] vecchio Vecchio socio/giocatore da modificare
 * @param[in,out] circolo Circolo al quale aggiungere il socio, viene passato per riferimento
 * @return successo (TRUE) o fallimento (FALSE)
 */
giocatore_t *aggiungi_socio	(const char nome[], const char cognome[], const char nascita[],
				const char tessera[], const char telefono[], const char email[], const char classifica[],
				bool retta, giocatore_t *vecchio, circolo_t *circolo);
/** Aggiunge un campo al Circolo.
 * Crea un nuovo campo con i dati passati e lo aggancia alla lista campi del circolo
 * @param[in] numero Numero identificativo del campo
 * @param[in] copertura Tipo di copertura (INDOOR, OUTDOOR)
 * @param[in] terreno Tipo di terreno
 * @param[in] note Eventuali note
 * @param[in,out] circolo Circolo al quale aggiungere il campo;
 * @return successo (TRUE) o fallimento (FALSE)
 */
campo_t *aggiungi_campo(int numero, copertura_t copertura, terreno_t terreno, const char note[], campo_t *vecchio, circolo_t *circolo);

/** Aggiunge un ora al campo.
 * Crea una nuova ora e la aggancia al campo
 * @param orario Orario dell'ora
 * @param data Data dell'ora
 * @param durata Durata dell'ora
 * @param tipo Tipo prenotante
 * @param prenotante Puntatore al prenotante
 * @param campo Campo al quale aggiungere l'ora
 * @return Puntatore all'ora appena creata
 */
ora_t *aggiungi_ora(int orario, const char data[], int durata, prenotante_t tipo, void *prenotante, campo_t *campo);

/** Restituisce il nome associato all'ora.
 * Il nome varia a seconda di che tipo è il prenotante
 * @param[in] ora Puntatore all'ora
 * @return Nome dell'ora
 */
const char *get_nome_ora(ora_t *ora);

/** Elimina il socio dal Circolo.
 * Elimina il socio passato come parametro rendendolo un giocatore normale
 * In altri termini declassa il socio a giocatore
 * @param[in] socio Puntatore al socio da eliminare
 * @param[in] circolo Circolo dal quale eliminare
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool elimina_socio(giocatore_t *socio, circolo_t *circolo);

/** Elimina il giocatore dal Circolo
 * Elimina i giocatore passato come parametro dal circolo
 * @param[in,out] giocatore Giocatore da eliminare
 * @param[in,out] circolo Circolo dal quale eliminare il giocatore
 */
bool elimina_giocatore(lista_giocatori &giocatore, circolo_t *circolo);

/** Elimina il campo dal Circolo.
 * Elimina il campo con tutte le ore a lui associate dal circolo
 * @param[in,out] campo Campo da eliminare
 * @param[in,out] circolo Circolo dal quale eliminare il campo
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool elimina_campo(lista_campi &campo, circolo_t *circolo);

/** Elimina l'ora dal campo.
 * Elimina l'ora passata come parametro dal campo
 * @param[in,out] ora Ora da eliminare
 * @param[in,out] campo Campo dal quale eliminare l'ora
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool elimina_ora(lista_ore &ora, campo_t *campo);

/** Elimina il circolo dalla memoria.
 * Elimina dalla memoria i tutto il circolo
 * @param[in,out] circolo Circolo da eliminare
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool elimina_circolo(circolo_t *&circolo);

/** Cerca all'interno della lista gli elementi corrispondeti ai dati passati, il confronto è tra stringhe.
 * Restituisce una lista con gli elementi corrispondenti alla ricerca
 * @param[in] lista Lista in cui cercare
 * @param[in] campo Campo della struttuara in cui cercare
 * @param[in] dati Dato di confronto
 * @param[in] tipo Tipo della struttura
 * @return Lista con gli elementi trovati
 */
#define cerca_lista_stringa(lista, campo, dati, tipo) 		\
({								\
	GList *res = 0;						\
	GList *tmp = lista;					\
	while(tmp){						\
		tipo *elemento = (tipo *) tmp->data;		\
		if ( g_strcmp0( elemento->campo, dati ) )	\
			res = g_list_append(res, elemento);	\
		tmp = g_list_next(tmp);				\
	}							\
	res;							\
})

/** Cerca all'interno della lista gli elementi corrispondeti ai dati passati, il confronto è tra interi.
 * Restituisce una lista con gli elementi corrispondenti alla ricerca
 * @param[in] lista Lista in cui cercare
 * @param[in] campo Campo della struttuara in cui cercare
 * @param[in] dati Dato di confronto
 * @param[in] tipo Tipo della struttura
 * @return Lista con gli elementi trovati
 */
#define cerca_lista_int(lista, campo, dati, tipo) 		\
({								\
	GList *res = 0;						\
	GList *tmp = lista;					\
	while(tmp){						\
		tipo *elemento = (tipo *) tmp->data;		\
		if ( elemento->campo == dati )			\
			res = g_list_append(res, elemento);	\
		tmp = g_list_next(tmp);				\
	}							\
	res;							\
})

/* Fine interfaccia del modulo accesso_dati */

#endif
