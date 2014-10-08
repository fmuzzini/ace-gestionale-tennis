/**
 * @file
 * File contenente il modulo accesso_dati.
 * Fornisce i metodi per l'accesso e la modifica dei dati
 */

#include <glib.h>
#include "accesso_dati.h"
#include "struttura_dati.h"
#include "file_IO.h"
#include "debug.h"

/* Inizio definizioni delle entità private del modulo */


/** Funzione usata per deallocare le ore.
 * Utile anche per la deallocazione della lista ore quando si elimina un campo
 * utilizzando la funzione g_list_free_full()
 */
static inline void dealloca_ora(gpointer ora_)
{
	ora_t *ora = (ora_t *) ora_;
	delete ora;
}

/** Funzione di comparazione per l'inserimento in ordine dei campi.
 * L'ordinamento avviene dal maggiore al minore per numero,
 * tale ordine è comodo per la visualizzazione della tabella ore
 * @param[in] a Primo campo da comparare
 * @param[in] b Secondo campo da comparare
 * @return -1 se b < a; 1 altrimenti
 */
static int inserisci_in_ordine_campo(gconstpointer a, gconstpointer b)
{
	const campo_t *campo_a = (campo_t *) a;
	const campo_t *campo_b = (campo_t *) b;

	if (campo_b->numero < campo_a->numero)
		return -1;

	return 1;
}

/** Confronta le date passete e ne ritorna l'ordine.
 * Confronta le stringhe interpetandole come date;
 * per confrontarle calcola la differenza sulle varie cifre
 * prima dell'anno, poi del mese e infine del girono
 * @param[in] a Prima data
 * @param[in] b Seconda data
 * @return negativo se a < b, positivo se a > b, 0 se a = b
 */
static int confronta_data(const char *a, const char *b)
{
	for (int i = 0; i < 4; i++)
		if ( a[6+i] != b[6+i] )
			return ( a[6+i] - b[6+i] );

	for (int i = 0; i < 2; i++)
		if ( a[3+i] != b[3+i] )
			return ( a[3+i] - b[3+i] );

	for (int i = 0; i < 2; i++)
		if ( a[i] != b[i] )
			return ( a[6+i] - b[6+i] );
	return 0;
}

/** Funzione di comparazione per l'inserimento in ordine delle ore.
 * Le ore vengono inserite in ordine cronologico
 * @param[in] a Prima ora da comparare
 * @param[in] b Seconda ora da comparare
 * @return -1 se a < b, 0 se a = b, 1 se a > b
 */
static int inserisci_in_ordine_ora(gconstpointer a, gconstpointer b)
{
	const ora_t *ora_a = (ora_t *) a;
	const ora_t *ora_b = (ora_t *) b;

	int data = confronta_data(ora_a->data->str, ora_b->data->str);

	if (data != 0)
		return data;

	if (ora_a->orario < ora_b->orario)
		return -1;
	
	if (ora_a->orario == ora_b->orario)
		return 0;

	return 1;
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
				const char circolo_g[], giocatore_t *vecchio, circolo_t *circolo)
{
	//Controllo validità Cricolo
	if (circolo == 0) return 0;

	//Creazione giocatore
	giocatore_t *giocatore = 0;
	
	if (vecchio == NULL){
		giocatore = g_try_new(giocatore_t, 1);
		if (giocatore == 0) return 0;	
		giocatore->ID = ++(circolo->pros_id);
		D1(cout<<"Memoria per giocatore allocata correttamente"<<endl)
	} else {
		//rimozione vecchie informazioni
		giocatore = vecchio;

		g_string_free(giocatore->nome, TRUE);
		g_string_free(giocatore->cognome, TRUE);
		g_string_free(giocatore->email, TRUE);
		g_string_free(giocatore->nascita, TRUE);
		g_string_free(giocatore->tessera, TRUE);
		g_string_free(giocatore->telefono, TRUE);
		g_string_free(giocatore->classifica, TRUE);
		g_string_free(giocatore->circolo, TRUE);
	}
	
	//Inserimento informazioni
	giocatore->nome = g_string_new(nome);
	giocatore->cognome = g_string_new(cognome);
	giocatore->email = g_string_new(email);
	giocatore->nascita = g_string_new(nascita);
	giocatore->tessera = g_string_new(tessera);
	giocatore->telefono = g_string_new(telefono);
	giocatore->classifica = g_string_new(classifica);
	giocatore->circolo = g_string_new(circolo_g);
	giocatore->socio = false;
	giocatore->retta = false;
	D1(cout<<"Informazioni giocatore create"<<endl)

	if (vecchio == NULL){
		//Aggancio al Circolo
		circolo->giocatori = g_list_append(circolo->giocatori, giocatore);
		D1(cout<<"giocatore agganciato"<<endl)
	}

	return giocatore;
}

giocatore_t *aggiungi_socio	(const char nome[], const char cognome[], const char nascita[],
				const char tessera[], const char telefono[], const char email[], const char classifica[],
				bool retta, giocatore_t *vecchio, circolo_t *circolo)
{
	//Creazione giocatore
	giocatore_t *socio = aggiungi_giocatore(nome, cognome, nascita, tessera, telefono, email, classifica, 
						circolo->nome->str, vecchio, circolo);
	if (socio == 0) return 0;

	socio->socio = true;
	socio->retta = retta;

	circolo->n_soci++;
	
	return socio;
}

campo_t *aggiungi_campo(int numero, copertura_t copertura, terreno_t terreno, const char note[], campo_t *vecchio, circolo_t *circolo)
{
	//Controllo validità circolo
	if (circolo == 0) return 0;

	campo_t *campo = 0;

	if (vecchio == NULL){
		//Creazione campo
		campo = g_try_new(campo_t, 1);
		if (campo == 0) return 0;
		campo->ore = 0;
		D1(cout<<"Memoria per campo allocata correttamente"<<endl)
	} else {
		//rimozione vecchie informazioni
		campo = vecchio;

		g_string_free(campo->note, TRUE);
	}

	//Inserimento informazioni
	campo->note = g_string_new(note);
	campo->numero = numero;
	campo->copertura = copertura;
	campo->terreno = terreno;
	D1(cout<<"Informazioni campo create"<<endl)

	if (vecchio == NULL){
		//Aggancio al Circolo
		circolo->n_campi++;
		circolo->campi = g_list_insert_sorted(circolo->campi, campo, inserisci_in_ordine_campo);
		D1(cout<<"Campo agganciato"<<endl)
	}

	return campo;
}

ora_t *aggiungi_ora(int orario, const char data[], int durata, giocatore_t *prenotante, campo_t *campo)
{
	//Controllo validità campo
	if (campo == 0) return 0;

	//Creazione ora
	ora_t *ora = g_try_new(ora_t, 1);
	if (ora == 0) return 0;
	D1(cout<<"Memoria per ora allocata correttamente"<<endl)

	//Inserimento informazioni
	ora->orario = orario;
	ora->data = g_string_new(data);
	ora->durata = durata;
	ora->prenotante = prenotante;
	D1(cout<<"Informazioni inserite"<<endl)

	//Aggancio al campo
	campo->ore = g_list_insert_sorted(campo->ore, ora, inserisci_in_ordine_ora);
	D1(cout<<"Ora agganciata"<<endl)

	return ora;
}

const char *get_nome_ora(ora_t *ora)
{
	if (ora == NULL)
		return 0;

	return ora->prenotante->cognome->str;
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

bool elimina_giocatore(giocatore_t *giocatore, circolo_t *circolo)
{
	D1(cout<<"elimina giocatore"<<endl)

	if (circolo == 0) return false;
	if (giocatore == 0) return false;

	//elimino ora associate al giocatore
	GList *tmp = circolo->campi;
	while(tmp != NULL){
		campo_t *campo = (campo_t *) tmp->data;
		GList *list = cerca_lista_int(campo->ore, prenotante, (int) giocatore, ora_t);
		GList *tmp_ = list;
		while(tmp_ != NULL){
			ora_t *ora = (ora_t *) tmp_->data;
			elimina_file_ora(ora, campo, circolo);
			elimina_ora(ora, campo);

			tmp_ = g_list_next(tmp_);	
		}
		g_list_free(list);

		tmp = g_list_next(tmp);	
	}

	D1(cout<<"Ore associate eliminate"<<endl)
	
	g_string_free(giocatore->nome, true);
	g_string_free(giocatore->cognome, true);
	g_string_free(giocatore->email, true);

	delete giocatore;

	D1(cout<<"giocatore deallocato"<<endl)

	circolo->giocatori = g_list_remove(circolo->giocatori, giocatore);

	D1(cout<<"giocatore eliminato dalla lista"<<endl)

	return true;	
}

bool elimina_campo(campo_t *campo, circolo_t *circolo)
{
	D1(cout<<"Elimina campo"<<endl)

	if (circolo == 0) return false;
	if (campo == 0) return false;

	g_string_free(campo->note, true);
	g_list_free_full(campo->ore, dealloca_ora);

	delete campo;

	circolo->campi = g_list_remove(circolo->campi, campo);
	circolo->n_campi--;
	
	return true;
}

bool elimina_ora(ora_t *ora, campo_t *campo)
{
	D1(cout<<"Elimina ora"<<endl)

	if (campo == 0) return false;
	if (ora == 0) return false;

	dealloca_ora(ora);
	campo->ore = g_list_remove(campo->ore, ora);

	return true;
	
}

bool elimina_circolo(circolo_t *&circolo)
{
	if (circolo == 0) return false;

	g_string_free(circolo->nome, true);
	g_string_free(circolo->indirizzo, true);
	g_string_free(circolo->email, true);
	g_string_free(circolo->telefono, true);
	g_list_foreach(circolo->campi, (GFunc) elimina_campo, circolo);
	g_list_foreach(circolo->giocatori, (GFunc) elimina_giocatore, circolo);
	g_list_free(circolo->campi);
	g_list_free(circolo->giocatori);

	delete circolo;
	circolo = 0;

	return true;	
}

/* Fine definizione pubbliche */
