/**
 * @file
 * File contentente il modulo handler.
 * Fornisce gli handler chiamati dall'interfaccia grafica al verificarsi degli eventi
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>

#include "handler.h"
#include "struttura_dati.h"
#include "accesso_dati.h"
#include "file_IO.h"
#include "debug.h"

extern GtkBuilder *build;

extern const char DATA_PATH[];

/* Inizio definizioni delle entità private del modulo */

enum tipo_ora { VUOTA, PRENOTATA };	/**< Tipo rapresentante il tipo d'ora nella tabella */

circolo_t *circolo;

const char ESTENSIONE_BACKUP[] = ".abk";
const char *coperture[] = {"INDOOR", "OUTDOOR"};
const char *terreni[] = {"ERBA", "ERBA SINTETICA", "TERRA", "SINTETICO", "CEMENTO"};

const int ORA_APERTURA = 8;
const int ORA_CHIUSURA = 23;

/** Trsforma un intero in una stringa orario.
 * Prende l'intero rappresentate i minuti totali e lo
 * converte in una stringa del formato hh:mm
 * @param[in] int Intero rappresentante i minuti
 * @return Stringa
 */
#define STRINGA_ORARIO(int) g_strdup_printf("%02d:%02d", (int / 60), (int % 60) )

/** Controlla l'estensione del file.
 * Se il nome del file non ha estensione l'aggiunge
 * @param[in,out] file File da controllare
 */
static inline void controllo_estensione(char *&file, const char *estensione)
{
	if ( g_str_has_suffix(file, estensione) )
		return;

	char *tmp = g_strconcat(file, estensione, NULL);
	g_free(file);
	file = tmp;
}

/** Ritorna il nome del file del FileChooser.
 * Sostitutivo della gtk_file_chooser_get_current_name
 * che è presente solo dalla versione 3.10 non presente in
 * tutte le distribuzioni
 * @param[in] chooser FileChooser
 * @return Nome del file
 */
static char *get_current_name(GtkFileChooser *chooser)
{
	char *file = gtk_file_chooser_get_filename(chooser);
	char *name = 0;
	int barra = 0;
	int i;

	for (i = 0; file[i] != '\0'; i++)
		if (file[i] == '/')
			barra = i;

	name = new char[i-barra];
	g_strlcpy(name, &file[barra+1], (i-barra) );

	g_free(file);

	return name;
}

/** Inserisce il giocatore nella ListStore.
 * Fa una copia dei dati del giocatore nella ListStore
 * @param[in] data_ Puntatore al giocatore
 * @param[out] list_ Puntatore alla ListStore
 */
static void insert_list_giocatore(gpointer data_, gpointer list_)
{
	GtkTreeIter iter;
	giocatore_t *data = (giocatore_t *) data_;
	GtkListStore *list = (GtkListStore *) list_;

	gtk_list_store_append(list, &iter);
	gtk_list_store_set(list, &iter, 
				0, data->nome->str,
				1, data->cognome->str,
				2, data->nascita->str,
				3, data->classifica->str,
				4, data->circolo->str,
				5, data->socio,
				6, data->retta,
				7, data,
				-1);
}

/** Inserisce il campo nella ListStore.
 * Fa una copia dei dati del campo nella ListStore
 * @param[in] data_ Puntatore al campo
 * @param[out] list_ Puntatore alla ListStore
 */
static void insert_list_campi(gpointer data_, gpointer list_)
{
	GtkTreeIter iter;
	campo_t *data = (campo_t *) data_;
	GtkListStore *list = (GtkListStore *) list_;

	const char *copertura = coperture[data->copertura];
	const char *terreno = terreni[data->terreno];

	gtk_list_store_append(list, &iter);
	gtk_list_store_set(list, &iter, 
				0, data->numero,
				1, copertura,
				2, terreno,
				3, data,
				-1);
}

/** Aggiunge alla tabella le righe dei campi.
 * Per ogni campo aggiunge una riga alla tabella con il numero del campo
 * @param[in] campo_ Puntatore al campo
 * @param[out] tabella_ Puntatore alla tabella
 */
static void inserisci_righe_campi(gpointer campo_, gpointer tabella_)
{
	campo_t *campo = (campo_t *) campo_;
	GtkGrid *tabella = (GtkGrid *) tabella_;
	char *tmp = g_strdup_printf("%d", campo->numero);
	GtkWidget *etichetta = GTK_WIDGET( gtk_label_new(tmp) );
	
	gtk_grid_insert_row(tabella, 1);
	gtk_grid_attach(tabella, etichetta, 0, 1, 1, 1);

	g_free(tmp);
}

/** Restituisce una stringa con la data.
 * @param[in] calendario Calendario
 * @return Data
 */
static char *get_stringa_data(GtkCalendar *calendario)
{
	unsigned int giorno, mese, anno;

	gtk_calendar_get_date(calendario, &anno, &mese, &giorno);
	mese++;
	char *data = g_strdup_printf("%02d-%02d-%04d", giorno, mese, anno);
	
	return data;
}

/** Controlla se un ora è disponibile.
 * Controlla che la combinazione ora durata non vada in conflitto con ore già prenotate
 * @param[in] campo Campo
 * @param[in] orario Orario
 * @param[in] durata Durata
 * @return Risultato del test
 */
static bool controlla_disponibilita_ora(campo_t *campo, int orario, int durata)
{
	GList *tmp = campo->ore;
	while(tmp != 0){
		ora_t *ora = (ora_t *) tmp->data;
		if ( (orario < ora->orario + ora->durata) && (orario + durata > ora->orario) )
			return false;

		tmp = g_list_next(tmp);
	}

	return true;
}

/** Distrugge la cella del container se non fa parte della struttara esterna.
 * @param[in] widget Cella da eliminare
 * @param[in] container Contenitore della cella
 */
static void distruggi_cella_se_interna(GtkWidget *widget, gpointer container)
{
	int top, left;
	gtk_container_child_get( GTK_CONTAINER(container), widget,
						"top-attach", &top,
						"left-attach", &left,
						NULL);
	
	if (top == 0 || left == 0)
		return;

	gtk_widget_destroy(widget);
}

/** Inserisce ore vuote nella tabella.
 * Inserisce nella tabella ore vuote dall'inizio alla fine della riga del campo
 * @param[in] inizio Inizio dell'inserimento
 * @param[in] fine Fine dell'inserimento
 * @param[in] tabella Tabella dove inserire
 * @param[in] campo Campo dove inserire le ore
 */
static void inserisci_ore_vuote(int inizio, int fine, GtkGrid* tabella, campo_t *campo)
{
	D1(cout<<"inserimento ore vuote"<<endl)
	D2(cout<<"da: "<<inizio<<"a: "<<fine<<endl);	
	
	if (inizio == fine)
		return;

	const char segnale[] = "clicked";
	GtkWidget *etichetta = 0;

	int ora_inizio = inizio + (ORA_APERTURA)*60;
	int scarto_inizio = 60 - (inizio % 60);
	int scarto_fine = fine % 60;
	int ora_scarto_fine = fine - scarto_fine + ORA_APERTURA*60;

	//Di defualt le ore inizia al minuto 0 quindi controllo eventuali scarti
	if (scarto_inizio != 60){
		D1(cout<<"scarto_inizio"<<endl)

		etichetta = GTK_WIDGET( gtk_button_new() );
		g_signal_connect( G_OBJECT(etichetta), segnale, G_CALLBACK(handler_mostra_ora), NULL );
		g_object_set_data( G_OBJECT(etichetta), "campo", campo);
		g_object_set_data( G_OBJECT(etichetta), "tipo_ora", GINT_TO_POINTER( VUOTA ) );
		g_object_set_data( G_OBJECT(etichetta), "ora", GINT_TO_POINTER(ora_inizio) );
		
		gtk_grid_attach(tabella, etichetta, inizio + 60, campo->numero, scarto_inizio, 1);

		inizio += 60;
	}

	if (scarto_fine != 0){
		D1(cout<<"scarto_fine"<<endl)

		etichetta = GTK_WIDGET( gtk_button_new() );
		g_signal_connect( G_OBJECT(etichetta), segnale, G_CALLBACK(handler_mostra_ora), NULL );
		g_object_set_data( G_OBJECT(etichetta), "campo", campo);
		g_object_set_data( G_OBJECT(etichetta), "tipo_ora", GINT_TO_POINTER( VUOTA ) );
		g_object_set_data( G_OBJECT(etichetta), "ora", GINT_TO_POINTER(ora_scarto_fine) );	

		gtk_grid_attach(tabella, etichetta, (fine - scarto_fine + 60), campo->numero, scarto_fine, 1);
	}

	inizio = inizio + scarto_inizio;
	fine = fine - scarto_fine;

	while (inizio <= fine){
		D2(cout<<inizio<<endl)
		ora_inizio = inizio + (ORA_APERTURA - 1)*60;

		etichetta = GTK_WIDGET( gtk_button_new() );
		g_signal_connect( G_OBJECT(etichetta), segnale, G_CALLBACK(handler_mostra_ora), NULL );
		g_object_set_data( G_OBJECT(etichetta), "campo", campo);
		g_object_set_data( G_OBJECT(etichetta), "tipo_ora", GINT_TO_POINTER( VUOTA ) );
		g_object_set_data( G_OBJECT(etichetta), "ora", GINT_TO_POINTER(ora_inizio) );
		
		gtk_grid_attach(tabella, etichetta, inizio, campo->numero, 60, 1);

		inizio += 60;
	}
	
}

/** Converte una stringa rappresentante un orario in intero.
 * In base al formato della stringa restituisce l'intero in minuti
 * @param[in] data Stringa
 * @return minuti rappresentanti l'ora
 */
static int controlla_formato_ora(const char *data)
{	
	int len = 0;
	for (; data[len] != '\0'; len++)
		;

	D2(cout<<"len: "<<len<<endl)
	
	//controllo che caratteri siano numerici 
	switch (len){
		case 5:
			if (data[4] < '0' || data[4] > '9')
				return -1;
		case 4:
			if (data[3] < '0' || data[3] > '9')
				return -1;
		case 2:
			if (data[1] < '0' || data[1] > '9')
				return -1;
		case 1: 
			if (data[0] < '0' || data[0] > '9')
				return -1;
			break;
		default:
			return -1;
	}

	D1(cout<<"Tutti caratteri numerici"<<endl) 

	//formato h
	if (len == 1)
		return (data[0] - '0')*60;

	//formato hh controllo che non sia maggiore di 23
	if (len == 2)
		switch (data[0]-'0'){
			case 0:
			case 1:
				return ((data[0]-'0')*10 + (data[1]-'0'))*60;
				break;
			case 2:
				if ((data[1]-'0') < 3)
					return ((data[0]-'0')*10 + (data[1]-'0'))*60;
			default:
				return -1;
		}

	//formato h:mm
	if (len == 4){
		if (data[2] > '5' )
			return -1;

		return (data[0]-'0')*60 + (data[2]-'0')*10 + (data[3]-'0');
	}

	//formato hh:mm
	if (len == 5){
		if (data[3] > '5')
			return -1;

		switch (data[0]-'0'){
			case 0:
			case 1:
				return ((data[0]-'0')*10 + (data[1]-'0'))*60 + (data[3]-'0')*10 + (data[4]-'0');
				break;
			case 2:
				if ((data[1]-'0') < 3)
					return ((data[0]-'0')*10 + (data[1]-'0'))*60 + (data[3]-'0')*10 + (data[4]-'0');
			default:
				return -1;
		}
	}
	
	return -1;		
}


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

void handler_nuovo_giocatore(GtkMenuItem *item, gpointer giocatore_)
{
	giocatore_t *giocatore = (giocatore_t *) giocatore_;

	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "agg_giocatore") );
	GObject *pulsante = gtk_builder_get_object(build, "giocatore_ok");

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

	if (giocatore != NULL){
		gtk_entry_set_text(entry_nome, giocatore->nome->str);
		gtk_entry_set_text(entry_cognome, giocatore->cognome->str);
		gtk_entry_set_text(entry_nascita, giocatore->nascita->str);
		gtk_entry_set_text(entry_tessera, giocatore->tessera->str);
		gtk_entry_set_text(entry_telefono, giocatore->telefono->str);
		gtk_entry_set_text(entry_email, giocatore->email->str);
		gtk_entry_set_text(entry_classifica, giocatore->classifica->str);
		gtk_entry_set_text(entry_circolo, giocatore->circolo->str);
		gtk_toggle_button_set_active(toggle_socio, giocatore->socio);
		gtk_toggle_button_set_active(toggle_retta, giocatore->retta);
	} 
	else {
		gtk_entry_set_text(entry_nome, "");
		gtk_entry_set_text(entry_cognome, "");
		gtk_entry_set_text(entry_nascita, "");
		gtk_entry_set_text(entry_tessera, "");
		gtk_entry_set_text(entry_telefono, "");
		gtk_entry_set_text(entry_email, "");
		gtk_entry_set_text(entry_classifica, "");
		gtk_entry_set_text(entry_circolo, "");
		gtk_toggle_button_set_active(toggle_socio, false);
		gtk_toggle_button_set_active(toggle_retta, false);

	}

	g_object_set_data(pulsante, "giocatore", giocatore);	

	mostra_finestra(NULL, window);
}

void handler_visualizza_giocatore(GtkButton *button, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET( gtk_widget_get_toplevel( GTK_WIDGET(button) ) );
	
	GtkTreeIter iter;
	GtkTreeModel *model;
	giocatore_t *giocatore_sel = 0;
	GtkTreeView *view = GTK_TREE_VIEW( gtk_builder_get_object(build, "giocatori_view") );
	GtkTreeSelection *selezione = gtk_tree_view_get_selection(view);

	if ( !gtk_tree_selection_get_selected(selezione, &model, &iter) ){
		finestra_errore("Selezionare un elemento");
		return;
	}
	gtk_tree_model_get(model, &iter, 7, &giocatore_sel, -1);
	
	nascondi_finestra(window, NULL, NULL);

	if(giocatore_sel != NULL)
		handler_nuovo_giocatore(NULL, giocatore_sel);
	
}

void handler_aggiungi_giocatore(GtkButton *button, gpointer user_data)
{
	if (circolo == 0){
		finestra_errore("Circolo non inizializzato");
		return;
	}

	GObject *giocatore_ = gtk_builder_get_object(build, "giocatore_ok");
	giocatore_t *vecchio_g = (giocatore_t *) g_object_get_data(giocatore_, "giocatore");

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

	if (nome[0] == '\0' || cognome[0] == '\0'){
		finestra_errore("Inserisci nome e cognome");
		return;
	}

	if (socio)
		giocatore = aggiungi_socio(nome, cognome, nascita, tessera, telefono, email, classifica, retta, vecchio_g, circolo);
	else
		giocatore = aggiungi_giocatore(nome, cognome, nascita, tessera, telefono, email, classifica, circolo_g, vecchio_g,
						circolo);

	if (giocatore == 0){
		finestra_errore("Impossibile aggiungere il giocatore");
		return;
	}

	if ( !salva_giocatore(giocatore, circolo) )
		finestra_errore("Attenzione! non è stato possibile salvare il giocatore su file\n alla chiusura del programma \
				il giocatore non sarà più recuperabile");

	nascondi_finestra( gtk_widget_get_toplevel( GTK_WIDGET(button) ), NULL, NULL);	
}

void handler_inizializza_cir(GtkButton *button, gpointer user_data)
{
	GtkEntry *entry_nome = GTK_ENTRY( gtk_builder_get_object(build, "nome_cir") );
	GtkEntry *entry_indirizzo = GTK_ENTRY( gtk_builder_get_object(build, "indirizzo_cir") );
	GtkEntry *entry_email = GTK_ENTRY( gtk_builder_get_object(build, "indirizzo_cir") );
	GtkEntry *entry_telefono = GTK_ENTRY( gtk_builder_get_object(build, "telefono_cir") );

	const char *nome = gtk_entry_get_text(entry_nome);
	const char *indirizzo = gtk_entry_get_text(entry_indirizzo);
	const char *email = gtk_entry_get_text(entry_email);
	const char *telefono = gtk_entry_get_text(entry_telefono);

	if (nome[0] == '\0'){
		finestra_errore("Inserire un nome per il circolo");
		return;
	}
	
	char *dir_cir = get_dir_circolo(nome);
	if ( g_file_test(dir_cir, G_FILE_TEST_IS_DIR) ){
		finestra_errore("Esiste già un circolo con questo nome");
		g_free(dir_cir);
		return;
	}
	g_free(dir_cir);

	if ( circolo != 0 && !alert("Verrà chiuso il circolo attuale. Continuare?") )
		return;

	if ( circolo != 0 )
		elimina_circolo(circolo);	

	if ( (circolo = inizializza_circolo(nome, indirizzo, email, telefono)) == 0 )
		finestra_errore("Impossibile creare il circolo");

	if ( !salva_circolo(circolo) )
		finestra_errore("Attenzione! non è stato possibile salvare il circolo su file\n alla chiusura del programma \
				il circolo non sarà più recuperabile");

	gtk_widget_set_sensitive( GTK_WIDGET( gtk_builder_get_object(build, "menu_modifica") ), TRUE);
	gtk_widget_set_sensitive( GTK_WIDGET( gtk_builder_get_object(build, "menu_backup") ), TRUE);
	gtk_widget_set_sensitive( GTK_WIDGET( gtk_builder_get_object(build, "menu_visualizza") ), TRUE);	

	nascondi_finestra( gtk_widget_get_toplevel( GTK_WIDGET(button) ), NULL, NULL);
	
	handler_carica_circolo(NULL, (void *) nome);
}

void handler_nuovo_campo(GtkMenuItem *item, gpointer campo_)
{
	campo_t *campo = (campo_t *) campo_;

	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "agg_campo") );
	GObject *pulsante = gtk_builder_get_object(build, "campo_ok");

	GtkSpinButton *entry_numero = GTK_SPIN_BUTTON( gtk_builder_get_object(build, "numero_c") );
	GtkComboBox *entry_copertura = GTK_COMBO_BOX( gtk_builder_get_object(build, "copertura_c") );
	GtkComboBox *entry_terreno = GTK_COMBO_BOX( gtk_builder_get_object(build, "terreno_c") );
	GtkEntry *entry_note = GTK_ENTRY( gtk_builder_get_object(build, "note_c") );
	
	if (campo != NULL){
		gtk_spin_button_set_value(entry_numero, campo->numero);
		gtk_combo_box_set_active(entry_copertura, campo->copertura);
		gtk_combo_box_set_active(entry_terreno, campo->terreno);
		gtk_entry_set_text(entry_note, campo->note->str);
	} 
	else {
		gtk_spin_button_set_value(entry_numero, circolo->n_campi + 1);
		gtk_combo_box_set_active(entry_copertura, 0);
		gtk_combo_box_set_active(entry_terreno, 0);
		gtk_entry_set_text(entry_note, "");
	}

	g_object_set_data(pulsante, "campo", campo);	


	mostra_finestra(NULL, window);
}

void handler_visualizza_campo(GtkButton *button, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "elenco_c") );
	
	GtkTreeIter iter;
	GtkTreeModel *model;
	campo_t *campo_sel = 0;
	GtkTreeView *view = GTK_TREE_VIEW( gtk_builder_get_object(build, "campi_view") );
	GtkTreeSelection *selezione = gtk_tree_view_get_selection(view);

	if ( !gtk_tree_selection_get_selected(selezione, &model, &iter) ){
		finestra_errore("Selezionare un elemento");
		return;
	}
	gtk_tree_model_get(model, &iter, 3, &campo_sel, -1);
	
	nascondi_finestra(window, NULL, NULL);

	if(campo_sel != NULL)
		handler_nuovo_campo(NULL, campo_sel);
	
}

void aggiorna_tabella_ore(GtkCalendar *calendario, gpointer user_data)
{
	if (!calendario)
		calendario = GTK_CALENDAR( gtk_builder_get_object(build, "calendario") );

	GtkContainer *cont_tabella = GTK_CONTAINER( gtk_builder_get_object(build, "tabella_ore") );
	GList *figli = gtk_container_get_children(cont_tabella);
	GtkGrid *tabella = GTK_GRID(figli->data);

	gtk_container_foreach( GTK_CONTAINER(tabella), distruggi_cella_se_interna, tabella);
	
	char *data = get_stringa_data(calendario);
	D1(cout<<"Giorno aquisito"<<endl)	

	GList *tmp_c = circolo->campi;
	while(tmp_c != NULL){
		campo_t *campo = (campo_t *) tmp_c->data;

		int inizio = 0;
		int fine = 0; 
		GList *tmp_o = campo->ore;
		while(tmp_o != NULL){
			ora_t *ora = (ora_t *) tmp_o->data;
			if ( g_strcmp0(ora->data->str, data) == 0 ){

				//Crea un bottone contente dati utili e lo inserisce nella tabella
				const char *nome = get_nome_ora(ora);
				GtkWidget *etichetta = GTK_WIDGET( gtk_button_new_with_label(nome) );
				g_signal_connect(etichetta, "clicked", G_CALLBACK(handler_mostra_ora), ora);
				g_object_set_data( G_OBJECT(etichetta), "campo", campo);
				g_object_set_data( G_OBJECT(etichetta), "tipo_ora", GINT_TO_POINTER(PRENOTATA) );
				g_object_set_data( G_OBJECT(etichetta), "ora", ora);

				fine = ora->orario - (ORA_APERTURA*60);
				inserisci_ore_vuote(inizio, fine, tabella, campo);
				inizio = (ora->orario)+(ora->durata)-(ORA_APERTURA*60);
				
				gtk_grid_attach(tabella, etichetta, (ora->orario-(ORA_APERTURA-1)*60), campo->numero, ora->durata, 1);
			}
			
			tmp_o = g_list_next(tmp_o);
		}
	
		fine = (ORA_CHIUSURA - ORA_APERTURA)*60;
		inserisci_ore_vuote(inizio, fine, tabella, campo);

		tmp_c = g_list_next(tmp_c);
	}

	g_free(data);

	gtk_widget_show_all( GTK_WIDGET(cont_tabella) );
}

void disegna_tabella_ore()
{
	GtkContainer *finestra = GTK_CONTAINER( gtk_builder_get_object(build, "tabella_ore") );
	GList *figli = gtk_container_get_children(finestra);
	GtkWidget *vecchio = GTK_WIDGET(figli->data);

	gtk_widget_destroy(vecchio);
		
	GtkGrid *tabella = GTK_GRID( gtk_grid_new() );

	gtk_container_add(finestra, GTK_WIDGET(tabella) );

	gtk_grid_set_column_spacing(tabella, 1);

	//colonna campi
	gtk_grid_insert_column(tabella, 0);

	//creazione colonne di tutta la giornata
	for (int i = 0; i < ( (ORA_CHIUSURA-ORA_APERTURA-1)*60 ); i++)
		gtk_grid_insert_column(tabella, 0);

	//riga per gli orari
	gtk_grid_insert_row(tabella, 0);
	for (int i = ORA_APERTURA; i <= ORA_CHIUSURA-1; i++){
		char *tmp = g_strdup_printf("%02d:00", i);
		GtkWidget *etichetta = GTK_WIDGET( gtk_label_new(tmp) );
		gtk_grid_attach(tabella, etichetta, ( (i - ORA_APERTURA + 1) * 60 ), 0, 60, 1);
		g_free(tmp);
	}

	//righe dei campi
	g_list_foreach(circolo->campi, inserisci_righe_campi , tabella);

	aggiorna_tabella_ore(NULL, NULL);
	gtk_widget_show_all( GTK_WIDGET(finestra) );
}

void handler_mostra_ora(GtkButton *button, gpointer user_data)
{
	GObject *cella = G_OBJECT(button);
	tipo_ora tipo = (tipo_ora) GPOINTER_TO_INT( g_object_get_data(cella, "tipo_ora") );
	campo_t *campo = (campo_t *) g_object_get_data(cella, "campo");
	gpointer ora_ = g_object_get_data(cella, "ora");

	char *campo_numero = g_strdup_printf("%d", campo->numero);

	GtkCalendar *calendario = GTK_CALENDAR( gtk_builder_get_object(build, "calendario") );
	GtkWidget *box_n = GTK_WIDGET( gtk_builder_get_object(build, "ora_nuova") );
	GtkWidget *box_v = GTK_WIDGET( gtk_builder_get_object(build, "ora_esistente") );

	if (tipo == VUOTA){
		GtkListStore *list = GTK_LIST_STORE( gtk_builder_get_object(build, "giocatori") );
		gtk_list_store_clear(list);
		g_list_foreach(circolo->giocatori, insert_list_giocatore, list);

		GtkComboBox *nome = GTK_COMBO_BOX( gtk_builder_get_object(build, "nome_ora_n") );
		GtkEntry *orario = GTK_ENTRY( gtk_builder_get_object(build, "orario_ora_n") );
		GtkEntry *durata = GTK_ENTRY( gtk_builder_get_object(build, "durata_ora_n") );
		GtkEntry *data = GTK_ENTRY( gtk_builder_get_object(build, "data_ora_n") );
		GtkLabel *campo_label = GTK_LABEL( gtk_builder_get_object(build, "campo_ora_n") );

		char *orario_ = STRINGA_ORARIO( GPOINTER_TO_INT(ora_) );
		char *data_ = get_stringa_data(calendario);

		gtk_combo_box_set_active(nome, 0);
		gtk_entry_set_text(orario, orario_ );
		gtk_entry_set_text(durata, "01:00");
		gtk_entry_set_text(data, data_);
		gtk_label_set_text(campo_label, campo_numero);

		g_object_set_data( G_OBJECT(box_n), "campo", campo);

		g_free(orario_);
		g_free(data_);
		g_free(campo_numero);
		
		gtk_widget_set_visible(box_n, TRUE);
		gtk_widget_set_visible(box_v, FALSE);

		return;
	}
	
	D1(cout<<"ora non vuota"<<endl)

	ora_t *ora = (ora_t *) ora_;

	GtkLabel *nome = GTK_LABEL( gtk_builder_get_object(build, "nome_ora") );
	GtkLabel *orario = GTK_LABEL( gtk_builder_get_object(build, "orario_ora") );
	GtkLabel *data = GTK_LABEL( gtk_builder_get_object(build, "data_ora") );
	GtkLabel *durata = GTK_LABEL( gtk_builder_get_object(build, "durata_ora") );
	GtkLabel *campo_label = GTK_LABEL( gtk_builder_get_object(build, "campo_ora") );

	char *orario_ = STRINGA_ORARIO(ora->orario);
	char *durata_ = STRINGA_ORARIO(ora->durata);

	gtk_label_set_text(nome, get_nome_ora(ora));
	gtk_label_set_text(orario, orario_);
	gtk_label_set_text(data, ora->data->str);
	gtk_label_set_text(durata, durata_);
	gtk_label_set_text(campo_label, campo_numero);

	g_object_set_data( G_OBJECT(box_v), "ora", ora);
	g_object_set_data( G_OBJECT(box_v), "campo", campo);	

	gtk_widget_set_visible(box_v, TRUE);
	gtk_widget_set_visible(box_n, FALSE);

	g_free(orario_);
	g_free(durata_);
	g_free(campo_numero);
}

void handler_prenota_ora(GtkButton *button, gpointer user_data)
{
	D1(cout<<"prenota ora"<<endl)

	GObject *box_n = gtk_builder_get_object(build, "ora_nuova");

	gpointer campo_ = g_object_get_data(box_n, "campo");

	GtkComboBox *selezione = GTK_COMBO_BOX( gtk_builder_get_object(build, "nome_ora_n") );
	GtkEntry *entry_orario = GTK_ENTRY( gtk_builder_get_object(build, "orario_ora_n") );
	GtkEntry *entry_durata = GTK_ENTRY( gtk_builder_get_object(build, "durata_ora_n") );
	GtkEntry *entry_data = GTK_ENTRY( gtk_builder_get_object(build, "data_ora_n") );

	const char *orario_ = gtk_entry_get_text(entry_orario);
	const char *durata_ = gtk_entry_get_text(entry_durata);
	const char *data = gtk_entry_get_text(entry_data);	

	int orario = controlla_formato_ora(orario_);
	int durata = controlla_formato_ora(durata_);

	if (orario < 0 || orario >= ORA_CHIUSURA*60){
		D1(cout<<"orario errato"<<endl)
		finestra_errore("Orario Errato");
		return;
	}

	if (durata < 0 || durata > ORA_CHIUSURA*60 - orario){
		D1(cout<<"durata errata"<<endl)
		finestra_errore("Durata Errata");
		return;
	}

	campo_t *campo = (campo_t *) campo_;

	if ( !controlla_disponibilita_ora(campo, orario, durata) ){
		finestra_errore("Ora non disponibile");
		return;
	}

	giocatore_t *giocatore = 0;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_combo_box_get_model(selezione);
	
	if ( !gtk_combo_box_get_active_iter(selezione, &iter) ){
		finestra_errore("Selezionare un giocatore");
		return;
	}

	gtk_tree_model_get(model, &iter, 7, &giocatore, -1);

	ora_t *ora = aggiungi_ora(orario, data, durata, giocatore, campo);
	salva_ora(ora, campo, circolo);

	aggiorna_tabella_ore(NULL, NULL);	
}

void handler_aggiungi_campo(GtkButton *button, gpointer user_data)
{
	if (circolo == 0){
		finestra_errore("Circolo non inizializzato");
		return;
	}

	GObject *campo_ = gtk_builder_get_object(build, "campo_ok");
	campo_t *vecchio_c = (campo_t *) g_object_get_data(campo_, "campo");

	campo_t *campo = 0;

	GtkSpinButton *entry_numero = GTK_SPIN_BUTTON( gtk_builder_get_object(build, "numero_c") );
	GtkComboBox *entry_copertura = GTK_COMBO_BOX( gtk_builder_get_object(build, "copertura_c") );
	GtkComboBox *entry_terreno = GTK_COMBO_BOX( gtk_builder_get_object(build, "terreno_c") );
	GtkEntry *entry_note = GTK_ENTRY( gtk_builder_get_object(build, "note_c") );

	int numero = gtk_spin_button_get_value_as_int(entry_numero);
	copertura_t copertura = (copertura_t) gtk_combo_box_get_active(entry_copertura);
	terreno_t terreno = (terreno_t) gtk_combo_box_get_active(entry_terreno);
	const char *note = gtk_entry_get_text(entry_note);

	GList *list = cerca_lista_int(circolo->campi, numero, numero, campo_t);

	if (list != NULL){
		finestra_errore("numero campo già prsente");
		return;
	}

	g_list_free(list);

	campo = aggiungi_campo(numero, copertura, terreno, note, vecchio_c, circolo);

	if (campo == 0){
		finestra_errore("Impossibile aggiungere il campo");
		return;
	}

	if ( !salva_campo(campo, circolo) )
		finestra_errore("Attenzione! non è stato possibile salvare il campo su file\n alla chiusura del programma \
				il campo non sarà più recuperabile");

	nascondi_finestra( gtk_widget_get_toplevel( GTK_WIDGET(button) ), NULL, NULL);
	disegna_tabella_ore();
}

void handler_apri_circolo(GtkMenuItem *button, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "carica_circolo") );
	GtkListStore *list = GTK_LIST_STORE( gtk_builder_get_object(build, "circoli") );
	GDir *dir = g_dir_open(DATA_PATH, 0, NULL);
	const char *file = 0;
	char *file_path = 0;
	GtkTreeIter iter;

	gtk_list_store_clear(list);

	while( (file = g_dir_read_name(dir)) ){
		if ( file_nascosto(file) )
			continue;

		file_path = g_build_filename(DATA_PATH, file, NULL);

		if ( !g_file_test(file_path, G_FILE_TEST_IS_DIR) ){
			g_free(file_path);
			continue;
		}
	
		g_free(file_path);

		gtk_list_store_append(list, &iter);
		gtk_list_store_set(list, &iter, 0, file, -1);
	}

	g_dir_close(dir);

	gtk_widget_set_sensitive( GTK_WIDGET( gtk_builder_get_object(build, "menu_modifica") ), TRUE);
	gtk_widget_set_sensitive( GTK_WIDGET( gtk_builder_get_object(build, "menu_backup") ), TRUE);
	gtk_widget_set_sensitive( GTK_WIDGET( gtk_builder_get_object(build, "menu_visualizza") ), TRUE);

	mostra_finestra(NULL, window);
}

void handler_carica_circolo(GtkButton *button, gpointer user_data)
{
	if ( circolo != 0 && user_data == 0 && !alert("Verrà chiuso il circolo attuale. Continuare?") )
		return;

	if ( circolo != 0 )
		elimina_circolo(circolo);
	
	char *circolo_sel = 0;	
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "carica_circolo") );

	if ( user_data != 0 )
		circolo_sel = (char *) user_data;
	else {	

		GtkTreeIter iter;
		GtkTreeModel *model;
		GtkTreeView *view = GTK_TREE_VIEW( gtk_builder_get_object(build, "circolo_view") );
		GtkTreeSelection *selezione = gtk_tree_view_get_selection(view);
	
		if ( !gtk_tree_selection_get_selected(selezione, &model, &iter) ){
			finestra_errore("Selezionare un elemento");
			return;
		}
		gtk_tree_model_get(model, &iter, 0, &circolo_sel, -1);
	
	}
	
	circolo = carica_circolo(circolo_sel);

	if (user_data == 0)
		g_free(circolo_sel);

	nascondi_finestra(window, NULL, NULL);
	disegna_tabella_ore();	
}

void handler_backup(GtkMenuItem *button, gpointer user_data)
{
	gint response;
	bool stato = true;
	char *file = 0;
	GtkFileChooser *scegli_file = GTK_FILE_CHOOSER( gtk_builder_get_object(build, "scegli_file") );
	
	gtk_file_chooser_set_action(scegli_file, GTK_FILE_CHOOSER_ACTION_SAVE);
	gtk_file_chooser_set_do_overwrite_confirmation(scegli_file, TRUE);

	mostra_finestra(NULL, scegli_file);
	
	response = gtk_dialog_run( GTK_DIALOG(scegli_file) );
	if (response == GTK_RESPONSE_YES){
		file = gtk_file_chooser_get_filename(scegli_file);
		D1(cout<<"nome: "<<file<<endl);
		stato = backup(file, circolo);
		g_free(file);
	}

	if (stato == false)
		finestra_errore("Non è stato possibile effetturare il backup");
	
	nascondi_finestra( GTK_WIDGET(scegli_file), NULL, NULL);
}

void handler_ripristina(GtkMenuItem *button, gpointer user_data)
{
	gint response;
	bool stato = false;
	char *file = 0;
	GtkFileChooser *scegli_file = GTK_FILE_CHOOSER( gtk_builder_get_object(build, "scegli_file") );
	
	gtk_file_chooser_set_action(scegli_file, GTK_FILE_CHOOSER_ACTION_OPEN);

	mostra_finestra(NULL, scegli_file);
	
	response = gtk_dialog_run( GTK_DIALOG(scegli_file) );
	if (response == GTK_RESPONSE_YES){
		file = gtk_file_chooser_get_filename(scegli_file);
		D1(cout<<"nome: "<<file<<endl);
		D1(cout<<"nome: "<<file<<endl);
		char *nome_cir = get_nome_backup(file);

		if ( circolo_esistente(nome_cir) ){
			//Circolo già presente
			D1(cout<<"Circolo esistente"<<endl)
			if ( alert("Questo backup fa riferimento ad un circolo presente. Vuoi sovrascriverlo?") ){
				elimina_file_circolo(nome_cir);
			}
			else
			{
				g_free(file);
				g_free(nome_cir);
				return;
			}
		}

		stato = ripristina(file);
		handler_carica_circolo(NULL, nome_cir);

		g_free(file);
		g_free(nome_cir);
	}

	if (stato == false)
		finestra_errore("Non è stato possibile ripristinare il backup");
	
	nascondi_finestra( GTK_WIDGET(scegli_file), NULL, NULL);
}

void handler_file_ok(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(user_data);
	if ( gtk_file_chooser_get_action(chooser) == GTK_FILE_CHOOSER_ACTION_SAVE ){
		char *file = get_current_name(chooser);
		
		controllo_estensione(file, ESTENSIONE_BACKUP);
		D1(cout<<file<<endl);
		gtk_file_chooser_set_current_name(chooser, file);
	}
}

void handler_elenco_giocatori(GtkMenuItem *button, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "elenco_g") );
	GtkTreeView *view = GTK_TREE_VIEW( gtk_builder_get_object(build, "giocatori_view") );
	GtkListStore *list = GTK_LIST_STORE( gtk_builder_get_object(build, "giocatori") );

	gtk_tree_view_set_model(view, GTK_TREE_MODEL(list) );
	
	gtk_list_store_clear(list);

	g_list_foreach(circolo->giocatori, insert_list_giocatore, list);

	gtk_widget_show_all(window);
	
}

void handler_elenco_soci(GtkMenuItem *button, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "elenco_g") );
	GtkTreeView *view = GTK_TREE_VIEW( gtk_builder_get_object(build, "giocatori_view") );
	GtkListStore *list = GTK_LIST_STORE( gtk_builder_get_object(build, "soci") );

	gtk_tree_view_set_model(view, GTK_TREE_MODEL(list) );

	GList *soci = cerca_lista_int(circolo->giocatori, socio, true, giocatore_t);

	gtk_list_store_clear(list);

	g_list_foreach(soci, insert_list_giocatore, list);

	g_list_free(soci);

	gtk_widget_show_all(window);
}

void handler_elenco_campi(GtkMenuItem *button, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "elenco_c") );
	GtkListStore *list = GTK_LIST_STORE( gtk_builder_get_object(build, "campi") );
	
	gtk_list_store_clear(list);

	g_list_foreach(circolo->campi, insert_list_campi, list);

	gtk_widget_show_all(window);
}

void handler_elimina_ora(GtkButton *button, gpointer user_data)
{
	if ( !alert("Sei sicuro di voler eliminare l'ora?") )
		return;
	
	GObject *box_v = gtk_builder_get_object(build, "ora_esistente");

	campo_t *campo = (campo_t *) g_object_get_data(box_v, "campo");
	ora_t *ora = (ora_t *) g_object_get_data(box_v, "ora");

	D1(cout<<campo<<endl);
	D2(cout<<ora<<endl);

	elimina_file_ora(ora, campo, circolo);
	elimina_ora(ora, campo);

	aggiorna_tabella_ore(NULL, NULL);
}

void handler_elimina_campo(GtkButton *button, gpointer user_data)
{
	if ( !alert("Sei sicuro di voler eliminare il campo?") )
		return;
	
	GtkTreeIter iter;
	GtkTreeModel *model;
	campo_t *campo = 0;
	GtkTreeView *view = GTK_TREE_VIEW( gtk_builder_get_object(build, "campi_view") );
	GtkTreeSelection *selezione = gtk_tree_view_get_selection(view);

	if ( !gtk_tree_selection_get_selected(selezione, &model, &iter) ){
		finestra_errore("Selezionare un elemento");
		return;
	}

	gtk_tree_model_get(model, &iter, 3, &campo, -1);

	elimina_file_campo(campo, circolo);
	elimina_campo(campo, circolo);

	handler_elenco_campi(NULL, NULL);
	disegna_tabella_ore();
}

void handler_elimina_giocatore(GtkButton *button, gpointer user_data)
{
	if ( !alert("Sei sicuro di voler eliminare il giocatore?") )
		return;

	GtkTreeIter iter;
	GtkTreeModel *model;
	giocatore_t *giocatore = 0;
	GtkTreeView *view = GTK_TREE_VIEW( gtk_builder_get_object(build, "giocatori_view") );
	GtkTreeSelection *selezione = gtk_tree_view_get_selection(view);

	if ( !gtk_tree_selection_get_selected(selezione, &model, &iter) ){
		finestra_errore("Selezionare un elemento");
		return;
	}

	gtk_tree_model_get(model, &iter, 7, &giocatore, -1);

	elimina_file_giocatore(giocatore, circolo);
	elimina_giocatore(giocatore, circolo);

	handler_elenco_giocatori(NULL, NULL);
	aggiorna_tabella_ore(NULL, NULL);
}

void handler_elimina_circolo(GtkButton *button, gpointer user_data)
{
	char *circolo_sel = 0;
	char *dir_cir = 0;
	
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeView *view = GTK_TREE_VIEW( gtk_builder_get_object(build, "circolo_view") );
	GtkTreeSelection *selezione = gtk_tree_view_get_selection(view);
	
	if ( !gtk_tree_selection_get_selected(selezione, &model, &iter) ){
		finestra_errore("Selezionare un elemento");
		return;
	}

	gtk_tree_model_get(model, &iter, 0, &circolo_sel, -1);

	dir_cir = get_dir_circolo(circolo_sel);

	D2(cout<<circolo_sel<<" "<<circolo->nome->str<<endl)

	if ( circolo != 0 && g_strcmp0(circolo_sel, circolo->nome->str) == 0 ){
		finestra_errore("Il circolo è attualmente in uso. Impossibile eliminarlo");
		return;
	}

	if ( !alert("Sei sicuro di voler eliminare il circolo?") )
		return;	

	elimina_file_circolo(circolo_sel);

	g_free(circolo_sel);
	g_free(dir_cir);

	handler_apri_circolo(NULL, NULL);
}


} // extern "C"

/* Fine definizioni pubbliche */

