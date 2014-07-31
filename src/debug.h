/**
 * @file
 * File contente le istruzioni per il debug.
 * Ha effetto solo se è definito DEBUG_MODE
 */

#ifndef DEBUG
#define DEBUG

#ifdef DEBUG_MODE
	extern unsigned char MASK;			/**< Maschera Debug. */
	#define DBG(A, B) {if ((A) & MASK) {B; } }	/**< Controlla la maschera e esegue l'istruzione. */
	#include <iostream>
	using namespace std;
#else
	#define DBG(A, B)				/**< DEBUG_MODE non definito. Non esegue nulla. */
#endif

#define D1(A) DBG(1, A) /**< Mostra i vari passaggi fondamentali */
#define D2(A) DBG(2, A) /**< Mostra stato variabili ed errori veri e propri */

#endif
