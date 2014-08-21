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

/** Chiama la funzione per ogni elemento della lista.
 * A differenza di g_list_foreach passa proprio l'elemento della lista (GList*)
 * e non i dati veri e propri (GList*->data)
 * viene utilizzata per eliminare tutti i campi o giocatori dal circolo
 * @param[in] list Lista
 * @param[in] fun Funzione da chiamare
 * @param[in] data Dati da passare alla funzione insieme all'elemento
 */
static void foreach_link(GList *list, bool(*fun)(GList *&, circolo_t *), circolo_t *data)
{
	GList *tmp;
	while(list){
		tmp = list;
		fun(list, data);
		list = g_list_next(tmp);
	}
}

/** Funzione usata per deallocare le ore.
 * Utile anche per la deallocazione della lista ore quando si elimina un campo
 * utilizzando la funzione g_list_free_full()
 */
static inline void dealloca_ora(void *ora_)
{
	ora_t *ora = (ora_t *) ora_;
	delete ora;
}

/* Fine definizioni private */

/* Inizio definizioni delle funzioni pubbliche del modulo */

circolo_t *inizializza_circolo(const char nome[], const char indirizzo[], const char email[], const char telefono[])
{
	circolo_t *circolo = g_try_new(circolo_t, 1);
	if (circolo == 0) return 0;
	D1(cout<<"Memoria per circolo allocata correttamente"<<endl)

	circolo->nome = g_string_new(nome);
	circolo->indirizzo = g_string_new(indirizzo);
	circolo->email = g_string_new(email);
	circolo->telefono = g_string_new(telefono);
	circolo->n_campi = 0;
	circolo->n_soci = 0;
	circolo->giocatori = 0;
	circolo->campi = 0;
	circolo->pros_id = 0;

	return circolo;
}

giocatore_t *aggiungi_giocatore(const char nome[], const char cognome[], const char nascita[],
				const char tessera[], const char telefono[], const char email[], const char classifica[],
				const char circolo_g[], circolo_t *circolo)
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
	giocatore->nascita = g_string_new(nascita);
	giocatore->tessera = g_string_new(tessera);
	giocatore->telefono = g_string_new(telefono);
	giocatore->classifica = g_string_new(classifica);
	giocatore->circolo = g_string_new(circolo_g);
	giocatore->ID = ++(circolo->pros_id);
	D1(cout<<"Informazioni giocatore create"<<endl)

	//Aggancio al Circolo
	circolo->giocatori = g_list_append(circolo->giocatori, giocatore);
	D1(cout<<"giocatore agganciato"<<endl)

	return giocatore;
}

giocatore_t *aggiungi_socio	(const char nome[], const char cognome[], const char nascita[],
				const char tessera[], const char telefono[], const char email[], const char classifica[],
				bool retta, circolo_t *circolo)
{
	//Creazione giocatore
	giocatore_t *socio = aggiungi_giocatore(nome, cognome, nascita, tessera, telefono, email, classifica, 
						circolo->nome->str, circolo);
	if (socio == 0) return 0;

	socio->socio = true;
	socio->retta = retta;

	circolo->n_soci++;
	
	return socio;
}

campo_t *aggiungi_campo(int numero, copertura_t copertura, terreno_t terreno, const char note[], circolo_t *circolo)
{
	//Controllo validità circolo
	if (circolo == 0) return 0;

	//Creazione campo
	campo_t *campo = g_try_new(campo_t, 1);
	if (campo == 0) return 0;
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
	circolo->campi = g_list_append(circolo->campi, campo);
	D1(cout<<"Campo agganciato"<<endl)

	return campo;
}

ora_t *aggiungi_ora(int orario, char data[], int durata, prenotante_t tipo, void *prenotante, campo_t *campo)
{
	//Controllo validità campo
	if (campo == 0) return 0;

	//Creazione ora
	ora_t *ora = g_try_new(ora_t, 1);
	if (ora == 0) return 0;
	D1(cout<<"Memoria per campo allocata correttamente"<<endl)

	//Inserimento informazioni
	ora->orario = orario;
	ora->data = g_string_new(data);
	ora->durata = durata;
	ora->tipo = tipo;
	ora->prenotante = prenotante;
	D1(cout<<"Informazioni inserite"<<endl)

	//Aggancio al campo
	campo->ore = g_list_append(campo->ore, ora);
	D1(cout<<"Ora agganciata"<<endl)

	return ora;
}

bool elimina_socio(giocatore_t *socio, circolo_t *circolo)
{
	//Controllo validità socio
	if (socio == 0) return false;
	if (socio->socio == false) return false;

	socio->socio = false;
	circolo->n_soci--;	

	return true;
	
}

bool elimina_giocatore(lista_giocatori &giocatore, circolo_t *circolo)
{
	if (circolo == 0) return false;
	if (giocatore == 0) return false;

	giocatore_t *giocatore_ = (giocatore_t *) giocatore->data;
	
	g_string_free(giocatore_->nome, true);
	g_string_free(giocatore_->cognome, true);
	g_string_free(giocatore_->email, true);

	delete giocatore_;

	circolo->giocatori = g_list_delete_link(circolo->giocatori, giocatore);
	giocatore = 0;

	return true;
	
	
}

bool elimina_campo(lista_campi &campo, circolo_t *circolo)
{
	if (circolo == 0) return false;
	if (campo == 0) return false;

	campo_t *campo_ = (campo_t *) campo->data;

	g_string_free(campo_->note, true);
	g_list_free_full(campo_->ore, dealloca_ora);

	delete campo_;

	circolo->campi = g_list_delete_link(circolo->campi, campo);
	circolo->n_campi--;
	campo = 0;
	
	return true;
}

bool elimina_ora(lista_ore &ora, campo_t *campo)
{
	if (campo == 0) return false;
	if (ora == 0) return false;

	dealloca_ora(ora->data);
	campo->ore = g_list_delete_link(campo->ore, ora);
	ora = 0;

	return true;
	
}

bool elimina_circolo(circolo_t *&circolo)
{
	if (circolo == 0) return false;

	g_string_free(circolo->nome, true);
	g_string_free(circolo->indirizzo, true);
	g_string_free(circolo->email, true);
	g_string_free(circolo->telefono, true);
	foreach_link(circolo->campi, elimina_campo, circolo);
	foreach_link(circolo->giocatori, elimina_giocatore, circolo);
	g_list_free(circolo->campi);
	g_list_free(circolo->giocatori);

	delete circolo;
	circolo = 0;

	return true;	
}

/* Fine definizione pubbliche */
