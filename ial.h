/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : ial.h - Rozhrani modulu ial.h
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral
 ============================================================================
 */
#ifndef IAL_H_
#define IAL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "str.h"
#include "ial.h"
#include "builtins.h"
#include "garbage.h"
#include "error.h"


/*
 * -----------------------------------------------------------------------------
 *                   HLAVICKY ADT a ALGORITMU PRO PREDMET IAL
 * -----------------------------------------------------------------------------
 */


/**
 * Vyhleda prvni vyskyt podretezce needle v retezci haystack a vrati
 * jeho pozici (od nuly). Prazdny retezec se vyskytuje v kazdem
 * retezci na pozici 0.
 *
 * Vyhledavani je provádí Knuth-Moris-Prattuv algoritmus.
 *
 * @param haystack Retezec, ve kterem se vyhledava
 * @param needle Hledany prodretezec.
 *
 * @return Pozici podretezce, nebo -1 pokud podretezec neobsahuje.
 */
int find_string(TString *haystack, TString *needle, int * position);

/**
 * Seradi znaky v danem retezci tak, aby znaky s nizsi ordinalni
 * hodnotou predchazely znakum s vyssi ordinalni hodnotou.
 *
 * Pokud je na vstupu prazdny retezec, vraci opet prazdny retezec.
 *
 * Razeni je provedeno algoritmem Shell sort.
 *
 * @param str Razeny algoritmus
 * @return Retezec obsahujici serazene znaky.
 */
int sort_string(TVar* dest, TVar * str);



/** ZACATEK DEKLARACi O HASHOVACI TABULCE */
#define HTABLE_SIZE 100
/**
 * Struktura reprezentujici jedenu polozku v tambulce symbolu,
 * key je identifikator, data obsahuje nas "obecny datovy typ"
 */
typedef struct item htable_listitem;
struct item{
    TString * name;
    TVar * var;
    htable_listitem* next;
};

/**
 * Struktura reprezentujici jeden seznam, head je jeho vrchol
 */
typedef struct{
       htable_listitem* head;
}htable_list;

/**
 * Struktura reprezentujici hashovaci tabulku,
 * v size je pocet seznamu a v ptr jsou pak samotne seznamy
 */
typedef struct{
      unsigned int htable_size;
      htable_list ptr[];
}htable_t;


/**
 * Vrati index do tabulky podle hashe zadanemu klici,
 *
 * cool hashovaci fce podle literatury
 *
 * @param str Klic - identifikator
 * @param htable_size Velikost tabulky
 *
 * @return index v tabulce
 */
int hash_function(const char *str, unsigned int htable_size);

/**
 * Inicializuje tabulku
 *
 * @param int size Velikost tabulky
 *
 * @return htable_t NULL nebo tabulka
 */
htable_t * htable_init(unsigned int size);

/**
 * Vypis obsahu htable
 *
 * @param htable_t t Tabulka, ktera se vypisuje
 */
void htable_print(htable_t * t);

/**
 * Vyhleda polozku
 *
 * @param htable_t t Tabulka, kde se hleda
 * @param TString key Jmeno identifikatoru
 *
 * @return htable_listitem NULL nebo polozka
 */
htable_listitem * htable_find_item(htable_t *t, TString * key);

/**
 * Vlozeni polozky do tabulky
 *
 * @param htable_t t Tabulka, kam se strka
 * @param TString key Jmeno identifikatoru
 * @param TVar var Uzitecny data
 *
 * @return errcode (TODO: nutno synchronizovat)
 */
int htable_insert(htable_t *t, TString * key, TVar *var);

int htable_overwrite(htable_t * t, TString *key, TVar * var);



/*
 * -----------------------------------------------------------------------------
 *         DALSI ADT A POMOCNE FUNKCE - NESOUVISI PRIMO S PREDMETEM IAL
 * -----------------------------------------------------------------------------
 */

/**
 * Prohodi znaky dle jejich indexu.
 *
 * @param input ukazatel na retezec, ve kterem ma prohodit znaky
 * @param i index prvniho znaku
 * @param j index druheho znaku
 */
void swap(char* input, int i, int j);

/**
 * Porovna ordinalni hodnotu vstupnich znaku.
 *
 * @param first prvni znak
 * @param second druhy znak
 * @return bool (prvni>druhy || !(prvni>druhy))
 */
bool less(char first, char second);

int get_substring(TVar * substr, TVar * str, int start, int end);




///zasobnik pro tabulky

typedef struct {
  int top;
  int allocated;
  htable_t **tables;
} htableStack;


/** Alokace a inicializace prázdného zásobníku
 * Příklad použití:
 * htableStack* stack = htable_stack_init();
 *
 * (Všechny následující operace se zásobníkem
 *  jsou automaticky prázdné operace, pokud
 *  není zásobník správně inicializovaný)
 *
 * @return Odkaz na prázný alokovaný zásobník
 */
htableStack *htable_stack_init();

/** Zjistí, zda-li je daný zásobník prázný
 *
 * @param s Odkaz na zásobník
 * @return Nula pokud není, nenulová hodnota pokud je
 */
int htable_stack_nearly_empty(htableStack *s);

/** Zjistí, zda-li v zasobniku zbyva posledni tabulka
 *
 * @param s Odkaz na zásobník
 * @return Nula pokud není, nenulová hodnota pokud je
 */
int htable_stack_empty(htableStack *s);

/** Získání hodnoty na vrcholu zásobníku
 * (Bez jeho odebrání; pouze přečtení)
 * (Pokud je zásobník prázdný, bude vráceno NULL)
 *
 * @param s Odkaz na zásobník
 * @return htable_t - vrchol zásobníku | NULL
 */
htable_t *htable_stack_top(htableStack *s);

/** Odstranení prvku z vrcholu zásobníku
 * (bez jeho přečtení, pouze odebrání)
 * (je-li zásobník prázný, prázdná operace)
 *
 * @param s Odkaz na zásobník
 */
void htable_stack_pop(htableStack *s);

/** Vložení htable_t na vrchol zásobníku
 *
 * @param s Odkaz na zásobník
 * @param d Data ke vložení - htable_t
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit zasobnik
 */
int htable_stack_push(htableStack *s,htable_t *table);


typedef struct{
  TString **queue;
  int top;
  int bottom;
  int allocated;
}TStringQueue;

/** Alokace a inicializace prázdné fronty
 * Příklad použití:
 * TStringQueue* front = TString_queue_init();
 *
 * (Všechny následující operace s frontou
 *  jsou automaticky prázdné operace, pokud
 *  není zásobník správně inicializovaný)
 *
 * @return Odkaz na práznout alokovanou frontu
 */
TStringQueue *TString_queue_init();

/** Zjistí, zda-li je daná fronta  prázndá
 *
 * @param s Odkaz na frontu
 * @return Nula pokud není, nenulová hodnota pokud je
 */
int TString_queue_empty(TStringQueue *f);

/** Získání hodnoty na začátku fronty
 * (Bez jeho odebrání; pouze přečtení)
 * (Pokud je fronta prázdná, bude vráceno NULL)
 *
 * @param s Odkaz na frontu
 * @return TString - začátek fronty | NULL
 */
TString *TString_queue_top(TStringQueue *f);

/** Odstranení prvku z vrcholu fronty
 * (bez jeho přečtení, pouze odebrání)
 * (je-li fronta prázdná, prázdná operace)
 *
 * @param s Odkaz na frontu
 */
void TString_queue_pop(TStringQueue *f);

/** Vložení TString na konec fronty
 *
 * @param s Odkaz na frontu
 * @param d Data ke vložení - TString*
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit frontu
 */
int TString_queue_put(TStringQueue *f,TString **arg);

/** Vyprázdnění fronty
 *
 * @param s Odkaz na frontu
 */
void TString_queue_set_empty(TStringQueue *queue);

void TString_queue_print(TStringQueue *f);



typedef struct{
  void *breakLab;
  void *contLab;
}TLoop;

typedef struct{
  int top;
  int allocated;
  TLoop **stack;
}loopStack;

loopStack *loopStack_init();

int loopStack_empty(loopStack *s);

TLoop *loopStack_top(loopStack *s);

void loopStack_pop(loopStack *s);

int loopStack_push(loopStack *s,void* breakLab, void* contLab);


#endif /* IAL_H_ */
