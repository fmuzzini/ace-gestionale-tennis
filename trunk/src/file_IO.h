/**
 * @file
 * File contenete l'interfaccia del modulo file_IO.cc
 */

#ifndef FILE_IO
#define FILE_IO

#include "struttura_dati.h"

/* Inizio interfaccia del modulo file_IO */

/** Salva su file i dati del circolo.
 * Salva nella directory del programma i dati del circolo
 * in formato testuale
 * @param[in] circolo Circolo da salvare
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool salva_circolo(const circolo_t *circolo);

/** Carica da file i dati del circolo
 * Carica dalla directory del programma i dati del circolo
 * @param[in] nome Nome del circolo da caricare
 * @return puntatore al circolo caricato
 */
circolo_t *carica_circolo(const char nome[]);

/** Carica un giocatore da file
 * Carica i dati del giocatore dal file e aggancia il giocatore
 * al circolo
 * @param[in] file File del giocatore
 * @param[in,out] circolo Circolo al quale agganciare il giocatore
 * @return Puntatore al giocatore appena creato
 */
giocatore_t *carica_giocatore(const char file[], circolo_t *circolo);

/** Salva il giocatore su file.
 * Salva il giocatore su un file nella directory del circolo
 * @param[in] giocatore Giocatore da salvare
 * @param[in] circolo Circolo del giocatore
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool salva_giocatore(const giocatore_t *giocatore, const circolo_t *circolo);

/** Salva il campo su file
 * Salva il campo su un file nella directory del circolo
 * @param[in] campo Campo da salvare
 * @param[in] circolo Circolo del campo
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool salva_campo(const campo_t *campo, const circolo_t *circolo);

/** Carica il campo da file.
 * Carica il campo dal file e lo aggancia al circolo
 * @param[in] file File del campo
 * @param[in,out] circolo Circolo al quale agganciare il campo
 * @return Puntatore al campo appena creato
 */
campo_t *carica_campo(const char file[], circolo_t *circolo);

/** Carica l'ora dal file.
 * Carica l'ora dal file selezionato e la aggancia al campo
 * @param[in] file File dell'ora
 * @param[in,out] campo Campo al quale agganciare
 * @param[in] circolo Circolo del campo
 * @return Puntatore all'ora creata
 */
ora_t *carica_ora(char file[], campo_t *campo, circolo_t *circolo);

/** Salva l'ora su file.
 * Salva l'ora su file, se archivia è true allora l'ora viene
 * salvata nell'archivio
 * @param[in] ora Ora da salvare
 * @param[in] campo Campo dell'ora
 * @param[in] circolo Circolo dell'ora
 * @param[in] archivia Indica se l'ora deve essere archiviata
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool salva_ora(const ora_t *ora, const campo_t *campo, const circolo_t *circolo, bool archivia = false);

/** Crea un backup del circolo.
 * Crea un backup del circolo e lo salva sul file
 * @param[in] file File nel quale salvare il backup
 * @param[in] circolo Circolo da salvare
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool backup(const char file[], circolo_t *circolo);

/** Ripristina un backup.
 * Ripristina l'albero di directory rappresentante un circolo da
 * un file di backup
 * @param[in] file File di backup
 * @return successo (TRUE) o fallimento (FALSE)
 */
bool ripristina(const char file[]);

/** Ritorna il nome del circolo ai cui fa riferimento
 * il file di backup
 * @param[in] file File di backup
 * @return Nome del circolo, 0 in caso di errori
 */
char *get_nome_backup(char file[]);

/* Fine interfaccia del modulo file_IO */

#endif
