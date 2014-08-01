/**
 * @file
 * File contenente il modulo accesso_dati.
 * Fornisce i metodi per l'accesso e la modifica dei dati
 */

#include <glib.h>
#include "accesso_dati.h"
#include "struttura_dati.h"
#include "debug.h"

/* Inizio definizioni delle entità private del modulo */

/** Struttura utilizzata per le ricerche in liste.
 * Gli elementi delle liste possono essere strutture
 * e la ricerca può avvenire in base a un solo campo della struttura;
 * Questa struttura contiene i dati necessari alla ricerca in base ad un campo.
 */
struct parametro_t {
	int offset;			/**< Distanza in byte del campo dall'inizio della struttura */
	int size;			/**< Grandezza del campo in byte */
	void *data;			/**< Puntatore al dato da confrontare, il dato puntato deve avere grandezza size */
	int(*fun)(void *, void *);	/**< Puntatore alla funzione di confronto */
};

/** Funzione usata per eliminare le ore quando si elimina un campo.
 * La funzione g_slist_foreach richiede come parametro una funzione
 * del tipo void(void*, void*), ma elimina_ora è bool(ora_t*,campo_t*)
 * quindi serve una funzione intermedia per poterla chiamare.
 */
static inline void elimina_ora_(void *ora_, void *campo_)
{
	ora_t *ora = (ora_t *) ora_;
	campo_t *campo = (campo_t *) campo_;
	elimina_ora(ora, campo);
}

/** Funzione per la ricerca nelle liste.
 * questa funzione consente di selezionare da una lista gli elementi
 * in base ad 
 */
static int confronta_parametro(parametro_t *parametro, void *elemento)
{
	elemento = elemento + parametro->offset;
	return parametro->fun(elemento, parametro->data);
}

/* Fine definizioni private */

/* Inizio definizioni delle funzioni pubbliche del modulo */

circolo_t *inizializza_circolo(char nome[], char indirizzo[], char email[], int telefono)
{
	circolo_t *circolo = g_try_new(circolo_t, 1);
	if (circolo == 0) return 0;
	D1(cout<<"Memoria per circolo allocata correttamente"<<endl)

	circolo->nome = g_string_new(nome);
	circolo->indirizzo = g_string_new(indirizzo);
	circolo->email = g_string_new(email);
	circolo->telefono = telefono;
	circolo->n_campi = 0;
	circolo->n_soci = 0;
	circolo->giocatori = 0;
	circolo->campi = 0;

	return circolo;
}

giocatore_t *aggiungi_giocatore(char nome[], char cognome[], char nascita[],
				int tessera, int telefono, char email[], float classifica,
				circolo_t *circolo)
{
	//Controllo validità Cricolo
	if (circolo == 0) return 0;

	//Creazione giocatore
	giocatore_t *giocatore = g_try_new(giocatore_t, 1);
	if (giocatore == 0) return 0;
	D1(cout<<"Memoria per giocatore allocata correttamente"<<endl)
	
	//Inserimento informazioni
	giocatore->nome = g_string_new(nome);
	giocatore->cognome = g_string_new(cognome);
	giocatore->email = g_string_new(email);
	g_stpcpy(giocatore->nascita, nascita);
	giocatore->tessera = tessera;
	giocatore->telefono = telefono;
	giocatore->ID = circolo->n_soci + 1;
	D1(cout<<"Informazioni giocatore create"<<endl)

	//Aggancio al Circolo
	circolo->n_soci++;
	circolo->giocatori = g_slist_append(circolo->giocatori, giocatore);
	D1(cout<<"giocatore agganciato"<<endl)

	return giocatore;
}

giocatore_t *aggiungi_socio	(char nome[], char cognome[], char nascita[], 
				int tessera, int telefono, char email[], float classifica,
			 	bool retta, circolo_t *circolo)
{
	//Creazione giocatore
	giocatore_t *socio = aggiungi_giocatore(nome, cognome, nascita, tessera, telefono, email, classifica, circolo);
	if (socio == 0) return 0;

	socio->socio = true;
	socio->retta = retta;
	
	return socio;
}

bool aggiungi_campo(int numero, copertura_t copertura, terreno_t terreno, char note[], circolo_t *circolo)
{
	//Controllo validità circolo
	if (circolo == 0) return false;

	//Creazione campo
	campo_t *campo = g_try_new(campo_t, 1);
	if (campo == 0) return false;
	D1(cout<<"Memoria per campo allocata correttamente"<<endl)

	//Inserimento informazioni
	campo->note = g_string_new(note);
	campo->numero = numero;
	campo->copertura = copertura;
	campo->terreno = terreno;
	campo->ore = 0;
	D1(cout<<"Informazioni campo create"<<endl)

	//Aggancio al Circolo
	circolo->n_campi++;
	circolo->campi = g_slist_append(circolo->campi, campo);
	D1(cout<<"Campo agganciato"<<endl)

	return true;
};

bool elimina_socio(giocatore_t *socio)
{
	//Controllo validità socio
	if (socio == 0) return false;
	if (socio->socio == false) return false;

	socio->socio = false;	

	return true;
	
}

bool elimina_giocatore(giocatore_t *&giocatore, circolo_t *circolo)
{
	if (circolo == 0) return false;
	if (giocatore == 0) return false;
	
	g_string_free(giocatore->nome, true);
	g_string_free(giocatore->cognome, true);
	g_string_free(giocatore->email, true);

	circolo->giocatori = g_slist_remove(circolo->giocatori, giocatore);
	giocatore = 0;

	return true;
	
	
}

bool elimina_campo(campo_t *&campo, circolo_t *circolo)
{
	if (circolo == 0) return false;
	if (campo == 0) return false;

	g_string_free(campo->note, true);
	g_slist_foreach(campo->ore, elimina_ora_, campo);

	circolo->campi = g_slist_remove(circolo->campi, campo);	
	campo = 0;
	
	return true;
}

bool elimina_ora(ora_t *&ora, campo_t *campo)
{
	if (campo == 0) return false;
	if (ora == 0) return false;

	campo->ore = g_slist_remove(campo->ore, ora);
	ora = 0;

	return false;
	
}

/* Fine definizione pubbliche */
