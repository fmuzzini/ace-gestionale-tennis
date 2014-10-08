/**
 * @mainpage ACE gestionale Circolo Tennis. <br>
 * Programma per la gestione di Campi da Tennis. <br>
 * Permette la gestione delle prenotazioni dei campi. <br>
 * 
 * Consente di creare un Circolo e di aggiungere ad esso <br>
 * campi e giocatori. <br>
 * E poi possibile gestire la prenotazione di tali campi. <br>
 *
 * Nella cartella esempi è presente il file esempio.abk <br>
 * che contiene il backup di un circolo d'esempio. <br>
 *
 * Richede le librerie GTK+-3.0 e Glib-2.0 per essere eseguito <br>
 * Di seguito link al sito ufficiale delle librerie <br>
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

/* Fine definizioni costanti */

GtkBuilder *build;

/** Funzione principale.
 * Carica e inizializza i vari componenti del programma e lancia l'interfaccia grafica
 */
int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	build = gtk_builder_new();

	g_assert(build);
	
	gtk_builder_add_from_file(build, INTERFACCIA, NULL);
	gtk_builder_connect_signals(build, NULL);

	gtk_main();	

	return 0;
}
