/**
 * @file File contente l'interfaccia del modulo handler.cc
 */

#ifndef HANDLER
#define HANDLER

#include <gtk/gtk.h>

/* Inizio interfaccia del modulo handler */

extern "C" {

/** Mostra a video la finestra.
 * @param[in] finestra Finestra da mostrare
 */
void mostra_finestra(GtkMenuItem *actived, gpointer finestra);

/** Nasconde la finestra.
 * @param[in] widget Finestra da nascondere.
 */
gboolean nascondi_finestra(GtkWidget *widget, GdkEvent *event, gpointer user_data);

/** Mostra una finestra di errore.
 * @param[in] messaggio Messaggio da mostrare
 */
void finestra_errore(const char messaggio[]);

/** Mostra una finestra di dialogo con i pulsanti SI e NO.
 * @param[in] messaggio Messaggio da mostrare
 * @return Pulsante premuto TRUE = SI, FALSE = NO
 */
bool alert(const char messaggio[]);

/** Modifica i campi in base al bottone.
 * Attiva o disattiva i campi Retta Pagata e Circolo in base allo stato
 * del bottone.
 * @param[in] button Bottone
 */
void socio_toggle(GtkToggleButton *button, gpointer user_data);

/** Esce dal programma.
 */
gboolean handler_esci(GtkWidget *widget, GdkEvent *event, gpointer user_data);

/** Setta sul primo elemento il ComboBox
 * @param[in] widget ComboBox
 */
void set_active(GtkWidget *widget, gpointer user_data);

/** Chiude la finestra.
 */
void handler_annulla(GtkButton *button, gpointer user_data);

/** Setta i campi della finestra agg_giocatore.
 * Setta i campi bianchi se giocatore_ è NULL; con i dati di giocatore_ altrimenti
 * @param[in] giocatore_ Vecchio giocatore
 */
void handler_nuovo_giocatore(GtkMenuItem *item, gpointer giocatore_);

/** Visualizza il giocatore selezionato.
 */
void handler_visualizza_giocatore(GtkButton *button, gpointer user_data);

/** Aggiunge/Modifica con i campi inseriri il giocatore
 * @param[in] user_data Eventuale vecchio giocatore
 */
void handler_aggiungi_giocatore(GtkButton *button, gpointer user_data);

/** Crea un nuovo circolo con i campi inseriti.
 */
void handler_inizializza_cir(GtkButton *button, gpointer user_data);

/** Setta i campi della finestra agg_campo.
 * Setti i campi bianchi se campo_ è NULL; con i dati di campo_ altrimenti
 * @param[in] campo_ Eventuale vecchio campo
 */
void handler_nuovo_campo(GtkMenuItem *item, gpointer campo_);

/** Visualizza il campo selezionato.
 */
void handler_visualizza_campo(GtkButton *button, gpointer user_data);

/** Aggiorna la tabella delle ore.
 * Colloca ogni ora nel giusto orario
 * @param[in] calendario Calandario da cui prendere la data
 */
void aggiorna_tabella_ore(GtkCalendar *calendario, gpointer user_data);

/** Disegna la base della tabella ore.
 * Disegna gli orari e le righe per i campi
 */
void disegna_tabella_ore();

/** Aggiunge/Modifica con i campi inseriri il campo
 * @param[in] user_data Eventuale vecchio campo
 */
void handler_aggiungi_campo(GtkButton *button, gpointer user_data);

/** Crea l'elenco dei circoli caricabili e lo mostra.
 */
void handler_apri_circolo(GtkMenuItem *button, gpointer user_data);

/** Carica il circolo selezionato.
 */
void handler_carica_circolo(GtkButton *button, gpointer user_data);

/** Mostra la finestra per il salvataggio del backup.
 */
void handler_backup(GtkMenuItem *button, gpointer user_data);

/** Mostra la finestra per il ripristino di un backup.
 */
void handler_ripristina(GtkMenuItem *button, gpointer user_data);

/** Controlla il nome del file selezionato per controllarne l'estensione.
 */
void handler_file_ok(GtkWidget *widget, GdkEvent *event, gpointer user_data);

/** Visualizza l'elenco dei giocatori.
 */
void handler_elenco_giocatori(GtkMenuItem *button, gpointer user_data);

/** Visualizza l'elenco dei soci.
 */
void handler_elenco_soci(GtkMenuItem *button, gpointer user_data);

/** Visualizza l'elenco dei campi.
 */
void handler_elenco_campi(GtkMenuItem *button, gpointer user_data);

/** Mostra i dati dell'ora.
 * @param[in] ora_ Ora da visualizzare
 */
void handler_mostra_ora(GtkButton *button, gpointer ora_);

/** Visualizza il prenotante dell'ora.
 * @param[in] ora_ Ora
 */
void handler_visualizza_ora(GtkButton *button, gpointer ora_);

/** Prenota un ora in base ai dati inseriti. 
 */ 
void handler_prenota_ora(GtkButton *button, gpointer user_data);

/** Elimina l'ora identificata dalla cella
 * @param[in] cella_ora Cella dell'ora da eliminare
 */
void handler_elimina_ora(GtkButton *button, gpointer cella_ora);

/** Elimina il campo selezionato.
 */
void handler_elimina_campo(GtkButton *button, gpointer user_data);

/** Elimina il giocatore selezionato.
 */
void handler_elimina_giocatore(GtkButton *button, gpointer user_data);


} // extern "C"


/* Fine interfaccia del modulo handler */


#endif
