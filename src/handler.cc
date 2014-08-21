/**
 * @file
 * File contentente il modulo file_IO.
 * Fornisce i metodi per il caricamento/salvataggio su/da file
 */

#include "struttura_dati.h"
#include "accesso_dati.h"
#include "file_IO.h"
#include "debug.h"
#include <gtk/gtk.h>

extern GtkBuilder *build;
extern circolo_t *circolo;

/* Inizio definizioni delle entità private del modulo */

/* Fine definizioni private */

/* Inizio definizioni pubbliche */

extern "C"
{


void mostra_finestra(GtkMenuItem *actived, gpointer finestra)
{
	gtk_widget_show_all( GTK_WIDGET(finestra) );
}

gboolean nascondi_finestra(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_hide(widget);
	return TRUE;
}

void finestra_errore(const char messaggio[])
{
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "errore") );
	GtkLabel *errore = GTK_LABEL( gtk_builder_get_object(build, "errore_m") );
	gtk_label_set_text(errore, messaggio);
	gtk_widget_show_all(window);	
}

bool alert(const char messaggio[])
{
	gint response;

	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "alert") );
	GtkLabel *errore = GTK_LABEL( gtk_builder_get_object(build, "alert_m") );
	gtk_label_set_text(errore, messaggio);
	gtk_widget_show_all(window);

	response = gtk_dialog_run( GTK_DIALOG(window) );
	D1(cout<<"Response: "<<response<<endl)
	if (response == GTK_RESPONSE_YES){
		nascondi_finestra(window, NULL, NULL);
		return true;
	}

	nascondi_finestra(window, NULL, NULL);

	return false;
}

void socio_toggle(GtkToggleButton *button, gpointer user_data)
{
	GtkWidget *circolo = GTK_WIDGET(gtk_builder_get_object(build, "circolo_g"));
	GtkToggleButton *retta = GTK_TOGGLE_BUTTON(gtk_builder_get_object(build, "retta_g"));
	bool stato = gtk_toggle_button_get_active(button);
	
	if (stato) {
		gtk_toggle_button_set_inconsistent(retta, false);
		gtk_widget_set_sensitive(circolo, false);
	} else {
		gtk_toggle_button_set_inconsistent(retta, true);
		gtk_widget_set_sensitive(circolo, true);
	}
}

gboolean handler_esci(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_main_quit();
	return TRUE;
}

void set_active(GtkWidget *widget, gpointer user_data)
{
	gtk_combo_box_set_active( GTK_COMBO_BOX(widget), 0);
}

void handler_annulla(GtkButton *button, gpointer user_data)
{
	GtkWidget *window = gtk_widget_get_toplevel( GTK_WIDGET(button) );
	gtk_widget_hide(window);
}

void handler_aggiungi_giocatore(GtkMenuItem *button, gpointer user_data)
{
	if (circolo == 0){
		finestra_errore("Circolo non inizializzato");
		return;
	}

	giocatore_t *giocatore = 0;

	GtkEntry *entry_nome = GTK_ENTRY( gtk_builder_get_object(build, "nome_g") );
	GtkEntry *entry_cognome = GTK_ENTRY( gtk_builder_get_object(build, "cognome_g") );
	GtkEntry *entry_nascita = GTK_ENTRY( gtk_builder_get_object(build, "nascita_g") );
	GtkEntry *entry_tessera = GTK_ENTRY( gtk_builder_get_object(build, "tessera_g") );
	GtkEntry *entry_telefono = GTK_ENTRY( gtk_builder_get_object(build, "telefono_g") );
	GtkEntry *entry_email = GTK_ENTRY( gtk_builder_get_object(build, "email_g") );
	GtkEntry *entry_classifica = GTK_ENTRY( gtk_builder_get_object(build, "classifica_g") );
	GtkEntry *entry_circolo = GTK_ENTRY( gtk_builder_get_object(build, "circolo_g") );
	GtkToggleButton *toggle_socio = GTK_TOGGLE_BUTTON( gtk_builder_get_object(build, "socio_g") );
	GtkToggleButton *toggle_retta = GTK_TOGGLE_BUTTON( gtk_builder_get_object(build, "retta_g") );

	const char *nome = gtk_entry_get_text(entry_nome);
	const char *cognome = gtk_entry_get_text(entry_cognome);
	const char *nascita = gtk_entry_get_text(entry_nascita);
	const char *tessera = gtk_entry_get_text(entry_tessera);
	const char *telefono = gtk_entry_get_text(entry_telefono);
	const char *email = gtk_entry_get_text(entry_email);
	const char *classifica = gtk_entry_get_text(entry_classifica);
	const char *circolo_g = gtk_entry_get_text(entry_circolo);
	bool socio = gtk_toggle_button_get_active(toggle_socio);
	bool retta = gtk_toggle_button_get_active(toggle_retta);

	if (socio)
		giocatore = aggiungi_socio(nome, cognome, nascita, tessera, telefono, email, classifica, retta, circolo);
	else
		giocatore = aggiungi_giocatore(nome, cognome, nascita, tessera, telefono, email, classifica, circolo_g, circolo);

	if (giocatore == 0){
		finestra_errore("Impossibile aggiungere il giocatore");
		return;
	}

	if ( !salva_giocatore(giocatore, circolo) )
		finestra_errore("Attenzione! non è stato possibile salvare il giocatore su file\n alla chiusura del programma \
				il giocatore non sarà più recuperabile");

	nascondi_finestra( gtk_widget_get_toplevel( GTK_WIDGET(button) ), NULL, NULL);	
}

void handler_inizializza_cir(GtkMenuItem *button, gpointer user_data)
{
	if ( circolo != 0 && !alert("Verrà chiuso il circolo attuale. Continuare?") )
		return;	

	GtkEntry *entry_nome = GTK_ENTRY( gtk_builder_get_object(build, "nome_cir") );
	GtkEntry *entry_indirizzo = GTK_ENTRY( gtk_builder_get_object(build, "indirizzo_cir") );
	GtkEntry *entry_email = GTK_ENTRY( gtk_builder_get_object(build, "indirizzo_cir") );
	GtkEntry *entry_telefono = GTK_ENTRY( gtk_builder_get_object(build, "telefono_cir") );

	const char *nome = gtk_entry_get_text(entry_nome);
	const char *indirizzo = gtk_entry_get_text(entry_indirizzo);
	const char *email = gtk_entry_get_text(entry_email);
	const char *telefono = gtk_entry_get_text(entry_telefono);

	if ( (circolo = inizializza_circolo(nome, indirizzo, email, telefono)) == 0 )
		finestra_errore("Impossibile creare il circolo");

	if ( !salva_circolo(circolo) )
		finestra_errore("Attenzione! non è stato possibile salvare il circolo su file\n alla chiusura del programma \
				il circolo non sarà più recuperabile");

	nascondi_finestra( gtk_widget_get_toplevel( GTK_WIDGET(button) ), NULL, NULL);
}

void handler_aggiungi_campo(GtkMenuItem *button, gpointer user_data)
{
	if (circolo == 0){
		finestra_errore("Circolo non inizializzato");
		return;
	}

	campo_t *campo = 0;

	GtkSpinButton *entry_numero = GTK_SPIN_BUTTON( gtk_builder_get_object(build, "numero_c") );
	GtkComboBox *entry_copertura = GTK_COMBO_BOX( gtk_builder_get_object(build, "copertura_c") );
	GtkComboBox *entry_terreno = GTK_COMBO_BOX( gtk_builder_get_object(build, "terreno_c") );
	GtkEntry *entry_note = GTK_ENTRY( gtk_builder_get_object(build, "note_c") );

	int numero = gtk_spin_button_get_value_as_int(entry_numero);
	copertura_t copertura = (copertura_t) gtk_combo_box_get_active(entry_copertura);
	terreno_t terreno = (terreno_t) gtk_combo_box_get_active(entry_terreno);
	const char *note = gtk_entry_get_text(entry_note);

	campo = aggiungi_campo(numero, copertura, terreno, note, circolo);

	if (campo == 0){
		finestra_errore("Impossibile aggiungere il campo");
		return;
	}

	if ( !salva_campo(campo, circolo) )
		finestra_errore("Attenzione! non è stato possibile salvare il campo su file\n alla chiusura del programma \
				il campo non sarà più recuperabile");

	nascondi_finestra( gtk_widget_get_toplevel( GTK_WIDGET(button) ), NULL, NULL);	
}


} // extern "C"

/* Fine definizioni pubbliche */

