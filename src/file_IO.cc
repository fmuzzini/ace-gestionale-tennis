/**
 * @file
 * File contentente il modulo file_IO.
 * Fornisce i metodi per il caricamento/salvataggio su/da file
 */

#include <glib.h>
#include <glib/gstdio.h>

#include <cstdlib>
#include <fstream>
#include <sstream>
using namespace std;

#include "file_IO.h"
#include "accesso_dati.h"
#include "struttura_dati.h"
#include "debug.h"


/* Inizio definizioni delle entità private del modulo */

extern const char DATA_PATH[] = "data";			/**< Directory dove il programma memorizza i dati */
const char DATI_CIRCOLO[] = "circolo.txt";		/**< File contenente i dati del circolo */
const char DATI_CAMPO[] = "campo.txt";			/**< File contenente i dati del campo */
const char GIOCATORI_DIR[] = "giocatori";		/**< Cartella dei giocatori */
const char CAMPI_DIR[] = "campi";			/**< Cartella dei campi */
const char ARCHIVIO_DIR[] = "archivio";			/**< Cartella delle ore archiviate */
const char ORE_DIR[] = "ore";				/**< Cartella delle ore */
const char FILE_EXT[] = ".txt";				/**< Estensione dei file */

const char ETX = 3;					/**< End of text */

/** Controlla l'esistenza della directory e se non esiste la crea.
 * Se la directory esiste ritorna subito TRUE altrimenti ricostruisce
 * l'albero delle directory per crearla	
 * @param[in] path Indirizzo della directory
 * @return TRUE se la directory esiste o è stata creata, FALSE se non è stato possibile crearla
 */
static bool controlla_directory(const char path[])
{
	if ( g_file_test(path, G_FILE_TEST_IS_DIR) ) return true;

	D1(cout<<"Cartella non presente"<<endl);
	//Se non esiste la crea
	char *file = 0;
	char *file_ = 0;
	char **cartelle = g_strsplit(path, "/", 0);
	int len = g_strv_length(cartelle);
	D2(cout<<"len: "<<len<<endl)
	
	file = g_build_filename(cartelle[0], NULL);
	for (int i = 0; i < len; i++){
		if ( !g_file_test(file, G_FILE_TEST_IS_DIR) ){

			D2(cout<<"Creazione cartella: "<<file<<endl)
			if ( g_mkdir(file, S_IRWXU) != 0 ){
				D1(cout<<"Impossibile creare cartella"<<endl)
				return false;
			}
		}
	
		file_ = g_build_filename(file, cartelle[i+1], NULL);
		g_free(file);
		file = file_;
	}
	
	g_free(file_);
	g_strfreev(cartelle);

	return true;
}

/** Copia il file nello stream.
 * Copia il contenuto del file nello stream
 * @param[in,out] f1 Stream in cui copiare il file
 * @param[in] file File da copiare
 * @return Successo (TRUE) o fallimento (FALSE)
 */
static bool backup_file(ofstream &f1, const char file[])
{
	if (!f1){
		D1(cout<<"Lo stream passato non è valido"<<endl)
		return false;
	}
	
	streambuf *buf = f1.rdbuf();
	ifstream f2(file);
	if (!f2){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		D2(cout<<"File: "<<file<<endl)
		return false;
	}
	
	f1<<"<file "<<file<<">"<<endl;
	
	while( !f2.eof() ){
		f2>>noskipws>>buf;
		f1.flush();
	}

	f1<<ETX<<endl;

	f2.close();

	return true;
}

/** Copia la cartella nello stream.
 * Copia il contenuto della cartella compresi i file nello stream
 * @param[in,out] f1 Stream nel quale copiare la cartella
 * @param[in] cartella Cartella da copiare
 * @return successo (TRUE) o fallimento (FALSE)
 */
static bool backup_dir(ofstream &f1, const char cartella[])
{
	if (!f1){
		D1(cout<<"Lo stream passato non è valido"<<endl)
		return false;
	}

	GDir *dir = g_dir_open(cartella, 0, NULL);
	const char *file = 0;
	char *file_ = 0;
	
	f1<<"<cartella "<<cartella<<">"<<endl;	

	while( (file = g_dir_read_name(dir)) ){
		file_ = g_build_filename(cartella, file, NULL);

		if ( g_file_test(file_, G_FILE_TEST_IS_DIR) )
			backup_dir(f1, file_);
		if ( g_file_test(file_, G_FILE_TEST_IS_REGULAR) )
			backup_file(f1, file_);

		g_free(file_);
	}
		

	return true;
}

/** Operatore per la lettura del tipo copertura_t.
 * Utilizza un intero temporaneo e lo converte
 */
static istream &operator>>(istream &is, copertura_t &c)
{
	int tmp;
	is>>tmp;
	c = static_cast<copertura_t>(tmp);

	return is;
}

/** Operatore per la lettura del tipo terreno_t.
 * Utilizza un intero temporaneo e lo converte
 */
static istream &operator>>(istream &is, terreno_t &t)
{
	int tmp;
	is>>tmp;
	t = static_cast<terreno_t>(tmp);

	return is;
}

/** Ritorna la directory del circolo.
 * @param[in] nome_cir Nome del circolo
 * @return Percorso della directory
 */
static char *get_dir_circolo(const char *nome_cir)
{
	return g_build_filename(DATA_PATH, nome_cir, NULL);
}

/** Ritorna il file del circolo.
 * @param[in] nome_cir Nome del circolo
 * @return Percorso del file
 */
static char *get_file_circolo(const char *nome_cir)
{
	char *dir = get_dir_circolo(nome_cir);
	char *file = g_build_filename(dir, DATI_CIRCOLO, NULL);

	g_free(dir);

	return file;
}

/** Ritorna la directory dell'ora.
 * @param[in] nome_cir Nome del Circolo
 * @param[in] campo Numero del campo
 * @return Percorso della directory
 */
static char *get_dir_ora(const char *nome_cir, int campo)
{
	ostringstream numero;
	numero<<campo;

	return g_build_filename(DATA_PATH, nome_cir, CAMPI_DIR, numero.str().c_str(), ORE_DIR, NULL);
}

/** Ritorna il file dell'ora.
 * @param[in] nome_cir Nome del circolo
 * @param[in] campo Numero del campo
 * @param[in] ora Ora
 * @return Percorso al file dell'ora
 */
static char *get_file_ora(const char *nome_cir, int campo, const ora_t *ora)
{
	D1(cout<<"get_file_ora"<<endl)
	ostringstream file;
	file<<ora->data->str<<"_"<<ora->orario<<FILE_EXT;

	D2(cout<<file<<endl);
	
	char *dir = get_dir_ora(nome_cir, campo);

	char *percorso = g_build_filename(dir, file.str().c_str(), NULL);

	g_free(dir);

	return percorso;
}

/** Ritorna la directory del giocatore.
 * @param[in] nome_cir Nome del circolo
 * @return Percorso della directory
 */
static char *get_dir_giocatore(const char *nome_cir)
{
	return g_build_filename(DATA_PATH, nome_cir, GIOCATORI_DIR, NULL);
}

/** Ritorna il file del giocatore.
 * @param[in] nome_cir Nome del circolo
 * @param[in] ID Id del giocatore
 * @return Percorso del file
 */
static char *get_file_giocatore(const char *nome_cir, int ID)
{
	ostringstream file;
	file<<ID<<FILE_EXT;

	char *dir = get_dir_giocatore(nome_cir);

	char *percorso = g_build_filename(dir, file.str().c_str(), NULL);

	g_free(dir);	

	return percorso;
}

/** Ritorna la directory del campo.
 * @param[in] nome_cir Nome del circolo
 * @param[in] n Numero del campo
 * @return Percorso della directory
 */
static char *get_dir_campo(const char *nome_cir, int n)
{
	ostringstream numero;
	numero<<n;

	char *percorso = g_build_filename(DATA_PATH, nome_cir, CAMPI_DIR, numero.str().c_str(), NULL);
	return percorso;
}

/** Ritorna il file del campo.
 * @param[in] nome_cir Nome Circolo
 * @param[in] n Numero campo
 * @return Percorso del file
 */
static char *get_file_campo(const char *nome_cir, int n)
{
	char *dir = get_dir_campo(nome_cir, n);

	char *file = g_build_filename(dir, DATI_CAMPO, NULL);	
	
	g_free(dir);

	return file;
}

/** Elimina ricorsivamente le sottodirectory della directory passata.
 * @param[in] dir_ Directory alla quale eliminare le sotto directory
 */
static void elimina_sub_directory(const char *dir_)
{
	GDir *dir = g_dir_open(dir_, 0, NULL);
	const char *file = 0;
	char *file_ = 0;

	while( (file = g_dir_read_name(dir)) ){
		file_ = g_build_filename(dir_, file, NULL);

		if ( g_file_test(file_, G_FILE_TEST_IS_DIR) )
			elimina_sub_directory(file_);
		if ( g_file_test(file_, G_FILE_TEST_IS_REGULAR) )
			g_remove(file_);

		g_free(file_);
	}

	g_rmdir(dir_);
	
}

/* Fine definizioni private */

/* Inizio definizioni delle funzioni pubbliche */

bool file_nascosto(const char file[])
{
	if (file[0] == '.')
		return true;
	else
		return false;
}

bool salva_circolo(const circolo_t *circolo)
{
	char *file = get_file_circolo(circolo->nome->str);
	char *dir = get_dir_circolo(circolo->nome->str);

	if (circolo == 0) return false;

	//Controllo esistenza cartella del circolo, se non esiste la crea
	if ( !controlla_directory(dir) ) return false;

	//Apertura file per la scrittura
	ofstream f1(file);

	g_free(dir);

	if(!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		D2(cout<<"File: "<<file<<endl)
		g_free(file);
		return false;
	}

	

	//Scrittura file
	f1<<circolo->nome->str<<endl;
	f1<<circolo->indirizzo->str<<endl;
	f1<<circolo->email->str<<endl;
	f1<<circolo->telefono->str<<endl;

	if(!f1){
		D1(cout<<"Errore in scrittura"<<endl)
		f1.close();
		g_remove(file);
		g_free(file);
		return false;
	}

	f1.close();
	g_free(file);

	return true;
}

circolo_t *carica_circolo(const char nome[])
{
	//Caricamento dati Circolo
	circolo_t *circolo = 0;
	char *testo = 0;
	char **campi_c = 0;
	int size = 0;
	char *file_c = get_file_circolo(nome);
	
	ifstream f1(file_c);
	if (!f1){
		D1(cout<<"Errore apertura file"<<endl)
		D2(cout<<"File: "<<file_c<<endl)
		return 0;
	}

	//Lunghezza file
	f1.seekg(0, f1.end);
	size = f1.tellg();
	f1.seekg(0, f1.beg);

	//Creazione buffer
	testo = new char[size+1];
	
	//Recupero informazioni
	f1.getline(testo, size, EOF);
	campi_c = g_strsplit(testo, "\n", 0);

	//Chiusura file
	f1.close();

	//Creazione circolo
	circolo = inizializza_circolo(campi_c[0], campi_c[1], campi_c[2], campi_c[3]);

	//Deallocazione memoria utilizzata
	g_strfreev(campi_c);
	g_free(file_c);
	delete[] testo;


	//Caricamento giocatori del circolo
	const char *file_g = 0;
	char *n_dir_g = get_dir_giocatore(nome);
	GDir *dir_g = g_dir_open(n_dir_g, 0, NULL);

	if (dir_g != NULL){

		while( (file_g = g_dir_read_name(dir_g)) ){
			if ( file_nascosto(file_g) )
				continue;
			
			char *giocatore = g_build_filename(n_dir_g, file_g, NULL);
	
			carica_giocatore(giocatore, circolo);

			D1(cout<<"Giocatore caricato"<<endl)
			D2(cout<<giocatore<<endl)

			g_free(giocatore);
		}
	
		g_dir_close(dir_g);
	}

	g_free(n_dir_g);


	//Caricamento campi del circolo
	const char *file = 0;
	campo_t *campo_caricato = 0;	
	char *ora = 0;
	char *dati = 0;
	char *campo = 0;
	char *campi = g_build_filename(DATA_PATH, nome, CAMPI_DIR, NULL);
	GDir *dir = g_dir_open(campi, 0, NULL);
	
	if (dir != NULL){

		while( (file = g_dir_read_name(dir)) ){
			if ( file_nascosto(file) )
				continue;
	
			campo = g_build_filename(campi, file, NULL);
	
			if ( !g_file_test(campo, G_FILE_TEST_IS_DIR) ){
				g_free(campo);
				continue;
			}
	
			dati = g_build_filename(campo, DATI_CAMPO, NULL);
	
			campo_caricato = carica_campo(dati, circolo);

			D1(cout<<"Campo caricato"<<endl)
			D2(cout<<dati<<endl)
	
			g_free(dati);
	
			dati = g_build_filename(campo, ORE_DIR, NULL); 

			g_free(campo);

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

				D1(cout<<"Ora caricata"<<endl)
				D2(cout<<ora<<endl)
		
				g_free(ora);
			}
	
			g_dir_close(dir_ore);
			g_free(dati);
	
		}
	
		g_dir_close(dir);

	} // if (dir != NULL)

	g_free(campi);

	return circolo;
}

bool salva_giocatore(const giocatore_t *giocatore, const circolo_t *circolo)
{
	char *dir = get_dir_giocatore(circolo->nome->str);
	char *file = get_file_giocatore(circolo->nome->str, giocatore->ID);
	
	if (giocatore == 0) return false;
	if (circolo == 0) return false;

	if ( !controlla_directory(dir) ) return false;

	g_free(dir);

	ofstream f1(file);

	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		D2(cout<<"File: "<<file<<endl)		
		g_free(file);		
		return false;
	}

	//Scrittura file
	f1<<giocatore->ID<<endl;
	f1<<giocatore->nome->str<<endl;
	f1<<giocatore->cognome->str<<endl;
	f1<<giocatore->nascita->str<<endl;
	f1<<giocatore->tessera->str<<endl;
	f1<<giocatore->telefono->str<<endl;
	f1<<giocatore->email->str<<endl;
	f1<<giocatore->classifica->str<<endl;
	f1<<giocatore->circolo->str<<endl;
	f1<<giocatore->socio<<endl;
	f1<<giocatore->retta<<endl;

	if (!f1){
		D1(cout<<"Errore in scrittura"<<endl)
		f1.close();
		g_remove(file);
		g_free(file);
		return false;		
	}

	f1.close();
	g_free(file);

	return true;	
}

giocatore_t *carica_giocatore(const char file[], circolo_t *circolo)
{
	giocatore_t *giocatore = 0;
	char *testo = 0;
	char **campi = 0;
	int size = 0;

	ifstream f1(file);
	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		return 0;
	}

	//Lunghezza file
	f1.seekg(0, f1.end);
	size = f1.tellg();
	f1.seekg(0, f1.beg);

	//Creazione buffer
	testo = new char[size+1];
	
	//Recupero informazioni
	f1.getline(testo, size, EOF);
	campi = g_strsplit(testo, "\n", 0);

	//Chiusura file
	f1.close();

	//Creazione circolo
	giocatore = aggiungi_giocatore(campi[1], campi[2], campi[3], campi[4], campi[5], campi[6], campi[7], campi[8], NULL, circolo);

	//Ripristino id e socio
	giocatore->ID = atoi(campi[0]);
	giocatore->socio = atoi(campi[9]);
	giocatore->retta = atoi(campi[10]);

	if (giocatore->socio) 
		circolo->n_soci++;

	//Deallocazione memoria utilizzata
	g_strfreev(campi);
	delete[] testo;

	return giocatore;
}

bool salva_campo(const campo_t *campo, const circolo_t *circolo)
{
	char *dir = get_dir_campo(circolo->nome->str, campo->numero);
	char *file = get_file_campo(circolo->nome->str, campo->numero);

	if (campo == 0) return false;
	if (circolo == 0) return false;

	if ( !controlla_directory(dir) ) return false;

	g_free(dir);

	ofstream f1(file);

	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		D2(cout<<"File: "<<file<<endl)		
		g_free(file);		
		return false;
	}

	//Scrittura file
	f1<<campo->numero<<endl;
	f1<<campo->copertura<<endl;
	f1<<campo->terreno<<endl;
	f1<<campo->note->str<<endl;

	if (!f1){
		D1(cout<<"Errore in scrittura"<<endl)
		f1.close();
		g_remove(file);
		g_free(file);
		return false;		
	}

	f1.close();
	g_free(file);

	return true;	
}

campo_t *carica_campo(const char file[], circolo_t *circolo)
{
	campo_t *campo = 0;
	int numero;
	terreno_t terreno;
	copertura_t copertura;
	char *note = 0;

	ifstream f1(file);
	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		return 0;
	}

	f1>>numero;
	f1>>copertura;
	f1>>terreno;

	//Lunghezza file
	f1.ignore();
	int pos = f1.tellg();
	f1.seekg(0, f1.end);
	int size = f1.tellg();
	f1.seekg(pos, f1.beg);
	
	note = new char[size-pos+1];	
	
	//Recupero informazioni
	f1.getline(note, size-pos, EOF);

	//Chiusura file
	f1.close();

	//Creazione circolo
	campo = aggiungi_campo(numero, copertura, terreno, note, NULL, circolo);

	//Deallocazione memoria utilizzata
	delete[] note;

	return campo;
}

bool salva_ora(const ora_t *ora, const campo_t *campo, const circolo_t *circolo)
{
	int prenotante;
	char *dir = get_dir_ora(circolo->nome->str, campo->numero);
	char *file = get_file_ora(circolo->nome->str, campo->numero, ora);
	
	if (ora == 0) return false;
	if (campo == 0) return false;

	if ( !controlla_directory(dir) ) return false;

	prenotante = ora->prenotante->ID;

	ofstream f1(file);

	g_free(dir);

	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		D2(cout<<"File: "<<file<<endl)		
		g_free(file);		
		return false;
	}

	//Scrittura file
	f1<<ora->orario<<endl;
	f1<<ora->data->str<<endl;
	f1<<ora->durata<<endl;
	f1<<prenotante<<endl;

	if (!f1){
		D1(cout<<"Errore in scrittura"<<endl)
		f1.close();
		g_remove(file);
		g_free(file);
		return false;		
	}

	f1.close();
	g_free(file);

	return true;	
}

ora_t *carica_ora(char file[], campo_t *campo, circolo_t *circolo)
{
	D1(cout<<"Carica ora"<<endl)

	ora_t *ora;
	char data[11];
	int orario, durata, id;
	giocatore_t *prenotante;

	ifstream f1(file);
	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		return 0;
	}

	f1>>orario;
	f1>>data;
	f1>>durata;
	f1>>id;

	GList *list = 0;
	list = cerca_lista_int(circolo->giocatori, ID, id, giocatore_t);
	prenotante = (giocatore_t *) list->data;
	g_list_free(list);

	//Chiusura file
	f1.close();

	//Creazione circolo
	ora = aggiungi_ora(orario, data, durata, prenotante, campo);

	return ora;
}

bool backup(const char file[], circolo_t *circolo)
{
	if (circolo == 0)
		return false;

	bool stato;
	ofstream fout(file);
	if (!fout){
		D1(cout<<"Errore nella creazione del file"<<endl)
		D2(cout<<"File: "<<file<<endl)
		return false;
	}

	char *cartella = g_build_filename(DATA_PATH, circolo->nome->str, NULL);

	fout<<"<ACE BACKUP circolo="<<circolo->nome->str<<">"<<endl;
	
	stato = backup_dir(fout, cartella);

	g_free(cartella);
	fout.close();

	return stato;
	
}

bool ripristina(const char file[])
{
	ifstream f1(file);
	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		D2(cout<<"File: "<<file<<endl)
		return false;
	}

	//salto la prima riga del file (contiene intestazione)
	char c;
	f1>>noskipws;
	while( (f1>>c) && (c != '\n') )
		; 
	
	char markup[10];
	char *file_n = 0;
	int inizio, fine;
	ofstream fout;
	
	while( f1.peek() != EOF ){
		f1.getline(markup, 10, ' ');
	
		inizio = f1.tellg();
		while ( (f1>>c) && (c != '>') )
			;
		fine = f1.tellg();
		f1.seekg(inizio, f1.beg);
		file_n = new char[fine-inizio];
		f1.get(file_n, fine-inizio);
		file_n[fine-inizio] = '\0';
	
		D1(cout<<"markup: "<<markup<<endl)
		
		//controllo markup (può essere o "<file" o "<cartella")
		if ( g_strcmp0(markup, "<file") == 0 ){
			fout.open(file_n);
			if (!fout){
				D1(cout<<"Impossibile scrivere il file"<<endl)
				D2(cout<<"file: "<<file_n<<endl)
				return false;
			}
			f1.ignore(); //ignoro il carattere >
			f1.ignore(); //ignoro il carattere \n
			
			while ( (f1>>c) && (c != ETX) )
				fout<<c;
			
			fout.close();
		} 
		else if ( g_strcmp0(markup, "<cartella") == 0){
			if ( !controlla_directory(file_n) ){
				D1(cout<<"impossibile creare la cartella"<<endl)
				D2(cout<<"cartella: "<<file_n<<endl);
				return false;
			}
			f1.ignore(); //ignoro il carattere >
		}
		else {
			D1(cout<<"file corrotto"<<endl)
			D2(cout<<"file: "<<file<<endl)
			return false;
		}
		
		f1.ignore(); //ignore il carattere \n alla fine della riga
		delete[] file_n;
	}
	
	f1.close();
	
	return true;	
}

char *get_nome_backup(const char file[])
{
	D1(cout<<"get nome backup"<<endl)

	ifstream f1(file);
	if (!f1){
		D1(cout<<"impossibile aprire il file"<<endl)
		D2(cout<<"file: "<<file<<endl);
		return 0;
	}

	f1>>noskipws;

	char c;
	while ( (f1>>c) && (c != '=') )
		;

	int inizio = f1.tellg();
	D2(cout<<"inizio: "<<inizio<<endl)

	while ( (f1>>c) && (c != '>') )
		;

	int fine = f1.tellg();
	fine--;
	D2(cout<<"fine: "<<fine<<endl)

	if (!f1){
		D1(cout<<"file corrotto"<<endl)
		return 0;
	}

	char *nome = new char[fine-inizio+1];

	f1.seekg(inizio, f1.beg);
	for (int i = 0; i<(fine-inizio); i++)
		f1>>nome[i];
	nome[fine-inizio] = '\0';

	D2(cout<<"Nome backup: "<<nome<<endl)

	return nome;	
}

bool elimina_file_giocatore(giocatore_t *giocatore, circolo_t *circolo)
{
	D1(cout<<"Elimina file giocatore"<<endl);

	int res;
	char *file = get_file_giocatore(circolo->nome->str, giocatore->ID);
	
	res = g_remove(file);

	g_free(file);

	if (res == -1)
		return false;

	return true;
}

void elimina_file_campo(campo_t *campo, circolo_t *circolo)
{
	char *file = get_dir_campo(circolo->nome->str, campo->numero);
	
	elimina_sub_directory(file);
	g_rmdir(file);

	g_free(file);
}

bool elimina_file_ora(ora_t *ora, campo_t *campo, circolo_t *circolo)
{
	D1(cout<<"Elimina file ora"<<endl)
	
	int res;

	char *file = get_file_ora(circolo->nome->str, 1, ora);

	D2(cout<<file<<endl);
	
	res = g_remove(file);

	g_free(file);

	if (res == -1)
		return false;
	
	return true;
}

void elimina_file_circolo(const char *nome_cir)
{
	char *dir = get_dir_circolo(nome_cir);
	
	elimina_sub_directory(dir);
	g_rmdir(dir);

	g_free(dir);
}

bool circolo_esistente(const char *nome_cir)
{
	char *dir = get_dir_circolo(nome_cir);

	bool stato = g_file_test(dir, G_FILE_TEST_IS_DIR);

	g_free(dir);
	
	return stato;
}	

/* Fine definizioni pubbliche */
