/**
 * @mainpage ACE gestionale Circolo Tennis.
 * Programma per la gestione di Circoli Tennis.
 * Permette la gestione dei campi, dei tornei, dei corsi.
 *
 * Richede le librerie GTK+-3.0 e Glib-2.0 per essere eseguito
 * Di seguito link al sito ufficiale delle librerie
 *
 * http://www.gtk.org/
 * 
 *
 * @author Filippo Muzzini
 */

/**
 * @file
 * File contenente la funzione ::main
 */

#include <gtk/gtk.h>
#include <glib.h>

#include <iostream>
using namespace std;

#include "struttura_dati.h"

#ifdef DEBUG_MODE
	unsigned char MASK = 1|2;
#endif

/* Definizioni costanti del modulo */

const char INTERFACCIA[] = "interfaccia/interfaccia.glade";
const char TEMA[] = "interfaccia/tema.css";

/* Fine definizioni costanti */

GtkBuilder *build;
circolo_t *circolo;

/** Funzione principale.
 * Carica e inizializza i vari componenti del programma e lancia l'interfaccia grafica
 */
int main(int argc, char *argv[])
{
	GdkScreen *screen = 0;
	GtkCssProvider *stile = 0;  

	gtk_init(&argc, &argv);

	build = gtk_builder_new();
	gtk_builder_add_from_file(build, INTERFACCIA, NULL);
	gtk_builder_connect_signals(build, NULL);

	screen = gdk_screen_get_default();
	stile = gtk_css_provider_new();
	gtk_css_provider_load_from_path(stile, TEMA, NULL);
	gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(stile), GTK_STYLE_PROVIDER_PRIORITY_THEME);

	gtk_main();	
}
