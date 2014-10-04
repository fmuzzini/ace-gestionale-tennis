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
extern circolo_t *circolo;

extern const char DATA_PATH[];
extern const char CAMPI_DIR[];
extern const char DATI_CAMPO[];
extern const char ORE_DIR[];
extern const char GIOCATORI_DIR[];

/* Inizio definizioni delle entità private del modulo */

const char ESTENSIONE_BACKUP[] = ".abk";
const char *coperture[] = {"INDOOR", "OUTDOOR"};
const char *terreni[] = {"ERBA", "ERBA_SINTETICA", "TERRA", "SINTETICO", "CEMENTO"};

const int ORA_APERTURA = 8;
const int ORA_CHIUSURA = 24;

/** Restituisce l'id dell'handler collegato alla funzione.
 * @param[in] ist Istanza
 * @param[in] func Nome della funzione associata
 * @return Id dell'handler
 */
#define GET_HANDLER_ID(ist, func) g_signal_handler_find(ist, G_SIGNAL_MATCH_FUNC, 0, 0, 0, (gpointer) func, 0)

/** Trsforma un intero in una stringa orario.
 * Prende l'intero rappresentate i minuti totali e lo
 * converte in una stringa del formato hh:mm
 * @param[in] int Intero rappresentante i minuti
 * @return Stringa
 */
#define STRINGA_ORARIO(int) g_strdup_printf("%02d:%02d", (int / 60), (int % 60) )

/** Controlla se un file è nascosto.
 * Esamina il nome del file e stabilisce se è nascosto;
 * Funziona solo su linux
 * @param[in] file File da esaminare
 * @return TRUE se è nascosto, FALSE altrimenti
 */
static inline bool file_nascosto(const char file[])
{
	if (file[0] == '.')
		return true;
	else
		return false;
}

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

static void setta_stile_tabella(GtkWidget *tabella)
{
	gtk_widget_set_name(tabella, "ore");
}

static void setta_stile_scheletro(GtkWidget *widget)
{
	GtkStyleContext *stile = gtk_widget_get_style_context(widget);

	gtk_style_context_add_class(stile, "scheletro");
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
	setta_stile_scheletro(etichetta);
	
	gtk_grid_insert_row(tabella, 1);
	gtk_grid_attach(tabella, etichetta, 0, 1, 1, 1);

	g_free(tmp);
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

static void inserisci_ore_vuote(int inizio, int fine, GtkGrid* tabella, campo_t *campo)
{
	D1(cout<<"inserimento ore vuote"<<endl)
	D2(cout<<"da: "<<inizio<<"a: "<<fine<<endl);	
	
	if (inizio == fine)
		return;

	const char segnale[] = "clicked";
	GtkWidget *etichetta = 0;

	int ora_inizio = inizio + (ORA_APERTURA - 1)*60;
	int scarto_inizio = 60 - (inizio % 60);
	int scarto_fine = fine % 60;
	int ora_scarto_fine = fine - scarto_fine + ORA_APERTURA*60;

	if (scarto_inizio != 60){
		D1(cout<<"scarto_inizio"<<endl)

		etichetta = GTK_WIDGET( gtk_button_new() );
		g_signal_connect( G_OBJECT(etichetta), segnale, G_CALLBACK(handler_mostra_ora), GINT_TO_POINTER(-ora_inizio) );
		g_signal_connect(etichetta, segnale, G_CALLBACK(handler_imposta_campo), campo);
		
		gtk_grid_attach(tabella, etichetta, inizio, campo->numero, scarto_inizio, 1);
	}

	if (scarto_fine != 0){
		D1(cout<<"scarto_fine"<<endl)

		etichetta = GTK_WIDGET( gtk_button_new() );
		g_signal_connect( G_OBJECT(etichetta), segnale, G_CALLBACK(handler_mostra_ora), GINT_TO_POINTER(-ora_scarto_fine) );
		g_signal_connect(etichetta, segnale, G_CALLBACK(handler_imposta_campo), campo);		

		gtk_grid_attach(tabella, etichetta, (fine - scarto_fine), campo->numero, scarto_fine, 1);
	}

	inizio = inizio + scarto_inizio;
	fine = fine - scarto_fine;

	while (inizio <= fine){
		ora_inizio = inizio + (ORA_APERTURA - 1)*60;

		etichetta = GTK_WIDGET( gtk_button_new_with_label("a") );
		g_signal_connect( G_OBJECT(etichetta), segnale, G_CALLBACK(handler_mostra_ora), GINT_TO_POINTER(-ora_inizio) );
		g_signal_connect(etichetta, segnale, G_CALLBACK(handler_imposta_campo), campo);
		
		gtk_grid_attach(tabella, etichetta, inizio, campo->numero, 60, 1);

		inizio += 60;
	}
	
}

/** Controna di rosso il widget
 * @param[in] widget Widget da contornare
 */
static void contorno_rosso(GtkWidget *widget)
{
	
}

static int controlla_formato_ora(const char *data)
{	
	int len = 0;
	for (; data[len] != '\0'; len++)
		;

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
	if (len == 4)
		return (data[0]-'0')*60 + (data[2]-'0')*10 + (data[3]-'0');

	//formato hh:mm
	if (len == 5)
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

	if (giocatore_ != NULL){
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

	gulong handler_id = GET_HANDLER_ID(pulsante, handler_aggiungi_giocatore);
	g_signal_handler_disconnect(pulsante, handler_id);
	g_signal_connect(pulsante, "clicked", G_CALLBACK(handler_aggiungi_giocatore), giocatore_);	

	mostra_finestra(NULL, window);
}

void handler_visualizza_giocatore(GtkButton *button, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "elenco_g") );
	
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
	
	giocatore_t *vecchio_g = (giocatore_t *) user_data;
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

	gtk_widget_set_sensitive( GTK_WIDGET( gtk_builder_get_object(build, "menu_modifica") ), TRUE);
	gtk_widget_set_sensitive( GTK_WIDGET( gtk_builder_get_object(build, "menu_backup") ), TRUE);
	gtk_widget_set_sensitive( GTK_WIDGET( gtk_builder_get_object(build, "menu_visualizza") ), TRUE);	

	nascondi_finestra( gtk_widget_get_toplevel( GTK_WIDGET(button) ), NULL, NULL);
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
	
	if (campo_ != NULL){
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

	gulong handler_id = GET_HANDLER_ID(pulsante, handler_aggiungi_campo);
	g_signal_handler_disconnect(pulsante, handler_id);
	g_signal_connect(pulsante, "clicked", G_CALLBACK(handler_aggiungi_campo), campo_);	


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
	
	unsigned int giorno, mese, anno;

	gtk_calendar_get_date(calendario, &anno, &mese, &giorno);
	char *data = g_strdup_printf("%02d-%02d-%04d", giorno, mese, anno);
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
				const char *nome = get_nome_ora(ora);
				GtkWidget *etichetta = GTK_WIDGET( gtk_button_new_with_label(nome) );
				g_signal_connect(etichetta, "clicked", G_CALLBACK(handler_imposta_campo), campo);
				g_signal_connect(etichetta, "clicked", G_CALLBACK(handler_mostra_ora), ora);

				fine = ora->orario - (ORA_APERTURA*60);
				inserisci_ore_vuote(inizio, fine, tabella, campo);
				inizio = (ora->orario)+(ora->durata)-(ORA_APERTURA*60);
				
				gtk_grid_attach(tabella, etichetta, (ora->orario-(ORA_APERTURA-1)*60), campo->numero, ora->durata, 1);
			}
			
			tmp_o = g_list_next(tmp_o);
		}
	
		fine = (ORA_CHIUSURA - ORA_APERTURA + 2)*60;
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
	setta_stile_tabella( GTK_WIDGET(tabella) );

	//colonna campi
	gtk_grid_insert_column(tabella, 0);

	//creazione colonne di tutta la giornata
	for (int i = 0; i < ( (ORA_CHIUSURA-ORA_APERTURA)*60 ); i++)
		gtk_grid_insert_column(tabella, 0);

	//riga per gli orari
	gtk_grid_insert_row(tabella, 0);
	for (int i = ORA_APERTURA; i <= ORA_CHIUSURA; i++){
		char *tmp = g_strdup_printf("%02d:00", i);
		GtkWidget *etichetta = GTK_WIDGET( gtk_label_new(tmp) );
		setta_stile_scheletro(etichetta);
		gtk_grid_attach(tabella, etichetta, ( (i - ORA_APERTURA + 1) * 60 ), 0, 60, 1);
		g_free(tmp);
	}

	//righe dei campi
	g_list_foreach(circolo->campi, inserisci_righe_campi , tabella);

	aggiorna_tabella_ore(NULL, NULL);
	gtk_widget_show_all( GTK_WIDGET(finestra) );
}

void handler_imposta_campo(GtkButton *button, gpointer campo_)
{
	D1(cout<<"imposta campo"<<endl)

	campo_t *campo = (campo_t *) campo_;

	GtkLabel *label_1 = GTK_LABEL( gtk_builder_get_object(build, "campo_ora") );
	GtkLabel *label_2 = GTK_LABEL( gtk_builder_get_object(build, "campo_ora_n") );

	char *etichetta = g_strdup_printf("%d", campo->numero);
	
	gtk_label_set_text(label_1, etichetta);
	gtk_label_set_text(label_2, etichetta);

	g_free(etichetta);

	GObject *pulsante = gtk_builder_get_object(build, "prenota");
	gulong handler_id = GET_HANDLER_ID(pulsante, handler_prenota_ora);
	g_signal_handler_disconnect(pulsante, handler_id);
	g_signal_connect(pulsante, "clicked", G_CALLBACK(handler_prenota_ora), campo_);	
}

void handler_visualizza_ora(GtkButton *button, gpointer ora_)
{
	ora_t *ora = (ora_t *) ora_;
	
	switch(ora->tipo){
		case GIOCATORE:
			handler_visualizza_giocatore(NULL, ora);
			break;
		case CORSO:
		
		case TORNEO:
	
		default: return;
	}
}

void handler_mostra_ora(GtkButton *button, gpointer ora_)
{
	GtkCalendar *calendario = GTK_CALENDAR( gtk_builder_get_object(build, "calendario") );
	GtkWidget *box_n = GTK_WIDGET( gtk_builder_get_object(build, "ora_nuova") );
	GtkWidget *box_v = GTK_WIDGET( gtk_builder_get_object(build, "ora_esistente") );

	if ( GPOINTER_TO_INT(ora_) < 0){
		GtkListStore *list = GTK_LIST_STORE( gtk_builder_get_object(build, "giocatori") );
		gtk_list_store_clear(list);
		g_list_foreach(circolo->giocatori, insert_list_giocatore, list);

		unsigned int giorno, mese, anno;
		gtk_calendar_get_date(calendario, &anno, &mese, &giorno);

		GtkComboBox *nome = GTK_COMBO_BOX( gtk_builder_get_object(build, "nome_ora_n") );
		GtkEntry *orario = GTK_ENTRY( gtk_builder_get_object(build, "orario_ora_n") );
		GtkEntry *durata = GTK_ENTRY( gtk_builder_get_object(build, "durata_ora_n") );
		GtkEntry *data = GTK_ENTRY( gtk_builder_get_object(build, "data_ora_n") );

		char *orario_ = STRINGA_ORARIO( -GPOINTER_TO_INT(ora_) );
		char *data_ = g_strdup_printf("%02d-%02d-%04d", giorno, mese, anno);

		gtk_combo_box_set_active(nome, 0);
		gtk_entry_set_text(orario, orario_ );
		gtk_entry_set_text(durata, "01:00");
		gtk_entry_set_text(data, data_);

		g_free(orario_);
		g_free(data_);
		
		gtk_widget_set_visible(box_n, TRUE);
		gtk_widget_set_visible(box_v, FALSE);

		return;
	}
	
	D1(cout<<"ora non vuota"<<endl)

	ora_t *ora = (ora_t *) ora_;
	
	GObject *visualizza = gtk_builder_get_object(build, "visualizza_ora");

	GtkLabel *nome = GTK_LABEL( gtk_builder_get_object(build, "nome_ora") );
	GtkLabel *orario = GTK_LABEL( gtk_builder_get_object(build, "orario_ora") );
	GtkLabel *data = GTK_LABEL( gtk_builder_get_object(build, "data_ora") );
	GtkLabel *durata = GTK_LABEL( gtk_builder_get_object(build, "durata_ora") );

	char *orario_ = STRINGA_ORARIO(ora->orario);
	char *durata_ = STRINGA_ORARIO(ora->durata);

	gtk_label_set_text(nome, get_nome_ora(ora));
	gtk_label_set_text(orario, orario_);
	gtk_label_set_text(data, ora->data->str);
	gtk_label_set_text(durata, durata_);

	gulong handler_id = GET_HANDLER_ID(visualizza, handler_visualizza_ora);
	g_signal_handler_disconnect(visualizza, handler_id);
	g_signal_connect(visualizza, "clicked", G_CALLBACK(handler_visualizza_ora), ora_);	

	gtk_widget_set_visible(box_v, TRUE);
	gtk_widget_set_visible(box_n, FALSE);

	g_free(orario_);
	g_free(durata_);
}

void handler_prenota_ora(GtkButton *button, gpointer campo_)
{
	D1(cout<<"prenota ora"<<endl)

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
		contorno_rosso( GTK_WIDGET(entry_orario) );
		return;
	}

	if (durata < 0 || durata > ORA_CHIUSURA*60 - orario){
		D1(cout<<"durata errata"<<endl)
		contorno_rosso( GTK_WIDGET(entry_durata) );
		return;
	}

	campo_t *campo = (campo_t *) campo_;

	giocatore_t *giocatore = 0;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_combo_box_get_model(selezione);
	gtk_combo_box_get_active_iter(selezione, &iter);
	gtk_tree_model_get(model, &iter, 7, &giocatore);

	ora_t *ora = aggiungi_ora(orario, data, durata, GIOCATORE, giocatore, campo);
	D2(cout<<ora<<endl)
	salva_ora(ora, campo, circolo);
	
}

void handler_aggiungi_campo(GtkButton *button, gpointer user_data)
{
	if (circolo == 0){
		finestra_errore("Circolo non inizializzato");
		return;
	}

	campo_t *campo = 0;
	campo_t *vecchio_c = (campo_t *) user_data;

	GtkSpinButton *entry_numero = GTK_SPIN_BUTTON( gtk_builder_get_object(build, "numero_c") );
	GtkComboBox *entry_copertura = GTK_COMBO_BOX( gtk_builder_get_object(build, "copertura_c") );
	GtkComboBox *entry_terreno = GTK_COMBO_BOX( gtk_builder_get_object(build, "terreno_c") );
	GtkEntry *entry_note = GTK_ENTRY( gtk_builder_get_object(build, "note_c") );

	int numero = gtk_spin_button_get_value_as_int(entry_numero);
	copertura_t copertura = (copertura_t) gtk_combo_box_get_active(entry_copertura);
	terreno_t terreno = (terreno_t) gtk_combo_box_get_active(entry_terreno);
	const char *note = gtk_entry_get_text(entry_note);

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
	if ( circolo != 0 && !alert("Verrà chiuso il circolo attuale. Continuare?") )
		return;

	GtkTreeIter iter;
	GtkTreeModel *model;
	GDir *dir;
	char *circolo_sel = 0;
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "carica_circolo") );
	GtkTreeView *view = GTK_TREE_VIEW( gtk_builder_get_object(build, "circolo_view") );
	GtkTreeSelection *selezione = gtk_tree_view_get_selection(view);

	if ( !gtk_tree_selection_get_selected(selezione, &model, &iter) ){
		finestra_errore("Selezionare un elemento");
		return;
	}
	gtk_tree_model_get(model, &iter, 0, &circolo_sel, -1);
	
	circolo = carica_circolo(circolo_sel);
	
	if (circolo == 0){
		finestra_errore("Impossibile caricare il circolo");
		g_free(circolo_sel);
		return;
	}

	const char *file = 0;
	char *giocatore = 0;
	char *giocatori = g_build_filename(DATA_PATH, circolo_sel, GIOCATORI_DIR, NULL);
	dir = g_dir_open(giocatori, 0, NULL);

	if (dir != NULL){

		while( (file = g_dir_read_name(dir)) ){
			if ( file_nascosto(file) )
				continue;
			
			giocatore = g_build_filename(giocatori, file, NULL);
	
			carica_giocatore(giocatore, circolo);
		}
	
		g_dir_close(dir);
	}
	
	g_free(giocatori);
	g_free(giocatore);

	campo_t *campo_caricato = 0;	
	char *ora = 0;
	char *dati = 0;
	char *campo = 0;
	char *campi = g_build_filename(DATA_PATH, circolo_sel, CAMPI_DIR, NULL);
	dir = g_dir_open(campi, 0, NULL);
	
	if (dir != NULL){

		while( (file = g_dir_read_name(dir)) ){
			if ( file_nascosto(file) )
				continue;
	
			campo = g_build_filename(campi, file, NULL);
	
			if ( !g_file_test(campo, G_FILE_TEST_IS_DIR) )
				continue;
	
			dati = g_build_filename(campo, DATI_CAMPO, NULL);
	
			campo_caricato = carica_campo(dati, circolo);
			
			if( campo_caricato == 0){
				finestra_errore("Impossibile caricare un campo");
			}
	
			g_free(dati);
	
			dati = g_build_filename(campo, ORE_DIR, NULL); 
			GDir *dir_ore = g_dir_open(dati, 0, NULL);
	
			if (dir_ore == NULL){
				g_free(dati);
				continue;
			}
			
			while( (file = g_dir_read_name(dir_ore)) ){
				if ( file_nascosto(file) )
					continue;
	
				ora = g_build_filename(dati, file, NULL);
				
				carica_ora(ora, campo_caricato, circolo);
		
				g_free(ora);
			}
	
			g_dir_close(dir_ore);
			g_free(dati);
	
		}
	
		g_dir_close(dir);

	} // if (dir != NULL)

	g_free(circolo_sel);
	g_free(campo);
	g_free(campi);
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
	bool stato = true;
	char *file = 0;
	GtkFileChooser *scegli_file = GTK_FILE_CHOOSER( gtk_builder_get_object(build, "scegli_file") );
	
	gtk_file_chooser_set_action(scegli_file, GTK_FILE_CHOOSER_ACTION_OPEN);

	mostra_finestra(NULL, scegli_file);
	
	response = gtk_dialog_run( GTK_DIALOG(scegli_file) );
	if (response == GTK_RESPONSE_YES){
		file = gtk_file_chooser_get_filename(scegli_file);
		D1(cout<<"nome: "<<file<<endl);
		
		stato = ripristina(file);
		g_free(file);
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
	GtkListStore *list = GTK_LIST_STORE( gtk_builder_get_object(build, "giocatori") );
	
	gtk_list_store_clear(list);

	g_list_foreach(circolo->giocatori, insert_list_giocatore, list);

	gtk_widget_show_all(window);
	
}

void handler_elenco_soci(GtkMenuItem *button, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET( gtk_builder_get_object(build, "elenco_s") );
	GtkListStore *list = GTK_LIST_STORE( gtk_builder_get_object(build, "giocatori") );

	GList *soci = cerca_lista_int(circolo->giocatori, socio, true, giocatore_t);

	gtk_list_store_clear(list);

	g_list_foreach(soci, insert_list_giocatore, list);

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




} // extern "C"

/* Fine definizioni pubbliche */

