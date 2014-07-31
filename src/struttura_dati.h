/**
 * @file
 * Header contenente le strutture dati comuni.
 */

#ifndef STRUTTURA_DATI
#define STRUTTURA_DATI

#include <glib.h>

/* Inizio Header del modulo Struttura dati */

const int LUN_DATA	= 10; /**< Lunghezza della date (gg/mm/aaaa) */

//@{
/** Definizione dei tipi di lista.
 * Le liste del programma si appoggiano alle liste di libreria
 */
typedef GSList *lista_ore, *lista_soci, *lista_campi;	//@}

/** Definizione del tipo stringa.
 * Le stringhe del programma si appoggiano alle stringhe di libreria
 */
typedef GString *stringa;

/** Struttura reppresentante il Circolo.
 * Il Circolo è caratterizzato dai dati (nome, inidirizzo, email, telefono) e
 * da due liste contenenti i campi e i soci;
 * ha anche due contatori per il numero di campi e di soci
 */
struct circolo_t {
	stringa nome;
	stringa indirizzo;
	stringa email;
	int telefono;
	int n_campi;
	int n_soci;
	lista_soci soci;
	lista_campi campi;
};

/** Struttura rappresentante i soci.
 * Ogni socio è costituito da i dati anagrafici, recapiti e un ID che identifica ogni socio
 */
struct socio_t {
	int ID;
	stringa nome;
	stringa cognome;
	char nascita[LUN_DATA];
	int tessera;
	int telefono;
	stringa email;
	float classifica;
};

/** Struttura rappresentante i giocatori.
 * Contiene i dati del giocatore non socio
 */
struct giocatore_t {
	int ID;
	stringa nome;
	stringa cognome;
	char nascita[LUN_DATA];
	int tessera;
	int telefono;
	stringa email;
	float classifica;
	stringa circolo;
};

/** Tipo che rappresenta chi ha prenotato un ora.
 * Può essere un socio, un giocatore non socio oppure il campo può essere usato per corsi o tornei
 */
enum prenotante_t {SOCIO, GIOCATORE, CORSO, TORNEO};

/** Struttura rappresentante le ore prenotate.
 * Ogni ora è caratterizzata dall'orario, la data, durata in minuti, il tipo di prenotante e un puntatore generico;
 * Il puntatore generico punterà a un dato diverso a seconda del tipo di prenotante :
 * se è SOCIO punta ai dati del socio,
 * se è GIOCATORE punta ai dati del giocatore non socio,
 * se è CORSO punta ai dati del gruppo del corso,
 * se è TORNEO punta ai dati del turno del torneo
 */
struct ora_t {
	int orario;
	char data [LUN_DATA];
	int durata;
	prenotante_t tipo;
	void *prenotante;
};

/** Tipo che rappresenta il tipo di copertura del campo.
 * Un capo può essere indoor oppure outdoor
 */
enum copertura_t {INDOOR, OUTDOOR};

/** Tipo che rappresenta il tipo di terreno.
 * Un campo può avere molte superfici, questo è un tipo generico non prevede i dettagli del terreno
 * ma li raggruppa nei tipi più significativi. Eventuali informazioni aggiuntive verranno inserite nelle note del tipo campo
 */
enum terreno_t {ERBA, ERBA_SINTETICA, TERRA, SINTETICO, CEMENTO};

/** Struttura rappresentate i campi.
 * Ogni campo è identificato da un numero ed è caratterizzato dal tipo di terreno e se è coperto o scoperto,
 * inoltre contiene un puntatore alla lista delle ore prenotate e delle note per eventuali informazioni aggiuntive
 */
struct campo_t {
	int numero;
	copertura_t copertura;
	terreno_t terreno;
	stringa note;
	lista_ore ore;
};

/* Fine header del modulo struttura dati */

#endif
