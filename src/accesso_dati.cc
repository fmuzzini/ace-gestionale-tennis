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



/* Fine definizioni private */

/* Inizio definizioni delle funzioni pubbliche del modulo */

circolo_t inizializza_circolo(char nome[], char indirizzo[], char email[], int telefono)
{
	circolo_t circolo;

	circolo.nome = g_string_new(nome);
	circolo.indirizzo = g_string_new(indirizzo);
	circolo.email = g_string_new(email);
	circolo.telefono = telefono;
	circolo.n_campi = 0;
	circolo.n_soci = 0;
	circolo.soci = 0;
	circolo.campi = 0;

	return circolo;
}

bool aggiungi_socio	(char nome[], char cognome[], char nascita[], int tessera, int telefono, char email[], float classifica,
			 circolo_t &circolo)
{
	//Controllo validità Cricolo
	if (&circolo == 0) return false;

	//Creazione socio
	socio_t *socio = g_try_new(socio_t, 1);
	if (socio == 0) return false;
	D1(cout<<"Memoria per socio allocata correttamente"<<endl)
	
	//Inserimento informazioni
	socio->nome = g_string_new(nome);
	socio->cognome = g_string_new(cognome);
	socio->email = g_string_new(email);
	g_stpcpy(socio->nascita, nascita);
	socio->tessera = tessera;
	socio->telefono = telefono;
	socio->ID = circolo.n_soci + 1;
	D1(cout<<"Informazioni socio create"<<endl)

	//Aggancio al Circolo
	circolo.n_soci++;
	circolo.soci = g_slist_append(circolo.soci, socio);
	D1(cout<<"Socio agganciato"<<endl)

	return true;
}

bool aggiungi_campo(int numero, copertura_t copertura, terreno_t terreno, char note[], circolo_t &circolo)
{
	//Controllo validità circolo
	if (&circolo == 0) return false;

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
	circolo.n_campi++;
	circolo.campi = g_slist_append(circolo.campi, campo);
	D1(cout<<"Campo agganciato"<<endl)

	return true;
};

bool elimina_socio(GSList *socio, circolo_t &circolo)
{
	//Sgancio socio dalla lista
	circolo.soci = g_slist_remove_link(circolo.soci, socio);
	D1(cout<<"Socio sganciato"<<endl)

	//Recupero dati socio e ne creo un giocatore
	socio_t *giocatore = (socio_t*) socio->data;	
	aggiungi_giocatore	(giocatore->nome->str, giocatore->cognome->str, giocatore->nascita, giocatore->tessera,
				giocatore->telefono, giocatore->email->str, giocatore->classifica, circolo);
	D1(cout<<"Giocatore creato"<<endl)
	
	//Rimuovo vecchio socio (ricordarsi le stringhe!)
	g_string_free(giocatore->nome, true);
	g_string_free(giocatore->cognome, true);
	g_string_free(giocatore->email, true);
	delete giocatore;
	delete socio;
	D1("Vecchio socio distrutto"<<endl)

	return true;
	
}

/* Fine definizione pubbliche */
