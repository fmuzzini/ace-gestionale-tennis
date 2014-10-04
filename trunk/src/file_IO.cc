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

const char DATI_CIRCOLO[] = "circolo.txt";		/**< File contenente i dati del circolo */
extern const char DATI_CAMPO[] = "campo.txt";		/**< File contenente i dati del campo */
extern const char DATA_PATH[] = "data";			/**< Directory dove il programma memorizza i dati */
extern const char GIOCATORI_DIR[] = "giocatori";	/**< Cartella dei giocatori */
extern const char CAMPI_DIR[] = "campi";		/**< Cartella dei campi */
const char ARCHIVIO_DIR[] = "archivio";			/**< Cartella delle ore archiviate */
extern const char ORE_DIR[] = "ore";			/**< Cartella delle ore */

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

/** Operatore per la lettura del tipo prenotante_t.
 * Utilizza un intero temporaneo e lo converte
 */
static istream &operator>>(istream &is, prenotante_t &t)
{
	int tmp;
	is>>tmp;
	t = static_cast<prenotante_t>(tmp);
	
	return is;
}

/* Fine definizioni private */

/* Inizio definizioni delle funzioni pubbliche */

bool salva_circolo(const circolo_t *circolo)
{
	const char *nome = circolo->nome->str;
	char *file = 0;
	char *dir = 0;

	if (circolo == 0) return false;

	//Controllo esistenza cartella del circolo, se non esiste la crea
	dir = g_build_filename(DATA_PATH, nome, NULL);
	if ( !controlla_directory(dir) ) return false;

	//Apertura file per la scrittura
	file = g_build_filename(dir, DATI_CIRCOLO, NULL);
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
	circolo_t *circolo = 0;
	char *testo = 0;
	char **campi = 0;
	int size = 0;
	char *file = g_build_filename(DATA_PATH, nome, DATI_CIRCOLO, NULL);
	
	ifstream f1(file);
	if (!f1){
		D1(cout<<"Errore apertura file"<<endl)
		D2(cout<<"File: "<<file<<endl)
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
	circolo = inizializza_circolo(campi[0], campi[1], campi[2], campi[3]);

	//Deallocazione memoria utilizzata
	g_strfreev(campi);
	g_free(file);
	delete[] testo;

	return circolo;
}

bool salva_giocatore(const giocatore_t *giocatore, const circolo_t *circolo)
{
	char *dir = g_build_filename(DATA_PATH, circolo->nome->str, GIOCATORI_DIR, NULL);
	char *file = 0;
	
	//ID sotto forma di stringa
	ostringstream id;
	id<<(giocatore->ID);
	
	if (giocatore == 0) return false;
	if (circolo == 0) return false;

	if ( !controlla_directory(dir) ) return false;

	file = g_strconcat(dir, "/", id.str().c_str(), ".txt", NULL);
	ofstream f1(file);

	g_free(dir);

	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		D2(cout<<"File: "<<file<<endl)		
		g_free(file);		
		return false;
	}

	//Scrittura file
	f1<<id.str().c_str()<<endl;
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
	//numero sotto forma di stringa
	ostringstream numero;
	numero<<(campo->numero);

	char *dir = g_build_filename(DATA_PATH, circolo->nome->str, CAMPI_DIR, numero.str().c_str(), NULL);
	char *file = 0;

	if (campo == 0) return false;
	if (circolo == 0) return false;

	if ( !controlla_directory(dir) ) return false;

	file = g_build_filename(dir, DATI_CAMPO, NULL);
	ofstream f1(file);

	g_free(dir);

	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		D2(cout<<"File: "<<file<<endl)		
		g_free(file);		
		return false;
	}

	//Scrittura file
	f1<<numero.str().c_str()<<endl;
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

bool salva_ora(const ora_t *ora, const campo_t *campo, const circolo_t *circolo, bool archivia)
{
	//numero sotto forma di stringa
	ostringstream numero, orario;
	numero<<(campo->numero);
	orario<<(ora->orario);
	
	int prenotante;
	char *dir;
	char *file = 0;

	if (archivia)	
		dir = g_build_filename(DATA_PATH, circolo->nome->str, CAMPI_DIR, numero.str().c_str(), ARCHIVIO_DIR, NULL);
	else
		dir = g_build_filename(DATA_PATH, circolo->nome->str, CAMPI_DIR, numero.str().c_str(), ORE_DIR, NULL);
	

	
	if (ora == 0) return false;
	if (campo == 0) return false;

	if ( !controlla_directory(dir) ) return false;

	switch (ora->tipo){
		case GIOCATORE:
			prenotante = ((giocatore_t *) ora->prenotante)->ID;
			break;
		case TORNEO:
			break;
		case CORSO:
			break;
	}


	file = g_strconcat(dir, "/", ora->data->str, "_", orario.str().c_str(), ".txt", NULL);
	ofstream f1(file);

	g_free(dir);

	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		D2(cout<<"File: "<<file<<endl)		
		g_free(file);		
		return false;
	}

	//Scrittura file
	f1<<orario.str().c_str()<<endl;
	f1<<ora->data->str<<endl;
	f1<<ora->durata<<endl;
	f1<<ora->tipo<<endl;
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
	ora_t *ora;
	char data[11];
	int orario, durata, id;
	prenotante_t tipo;
	void *prenotante;

	ifstream f1(file);
	if (!f1){
		D1(cout<<"Errore nell'apertura del file"<<endl)
		return 0;
	}

	f1>>orario;
	f1>>data;
	f1>>durata;
	f1>>tipo;
	f1>>id;

	switch (tipo){
		case GIOCATORE:
			prenotante = (cerca_lista_int(circolo->giocatori, ID, id, giocatore_t))->data;
			break;
		case TORNEO:
			break;
		case CORSO:
			break;
	}

	//Chiusura file
	f1.close();

	//Creazione circolo
	ora = aggiungi_ora(orario, data, durata, tipo, prenotante, campo);

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
	
	while( f1.peek() == EOF ){
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
	ifstream f1(file);
	if (!f1){
		D1(cout<<"impossibile aprire il file"<<endl)
		D2(cout<<"file: "<<file<<endl);
		return 0;
	}

	char c;
	while ( (f1>>c) && (c != '=') )
		;

	int inizio = f1.tellg();

	while ( (f1>>c) && (c != '>') )
		;

	int fine = f1.tellg();

	if (!f1){
		D1(cout<<"file corrotto"<<endl)
		return 0;
	}

	char *nome = new char[fine-inizio+1];

	f1.seekg(inizio, f1.beg);
	for (int i = 0; i<(fine-inizio); i++)
		f1>>nome[i];
	nome[fine-inizio+1] = '\0';

	return nome;	
}	

/* Fine definizioni pubbliche */
