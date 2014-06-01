/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : builtins.h - Rozhrani modulu garbage.c
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */

#ifndef GARBAGE_H_
#define GARBAGE_H_


#include <stdlib.h>
#include <stdio.h>

/**
 * Struktura reprezentujici zaznam o alokovanem zdroji.
 * Jedna se o polozku jednosmerneho linearniho seznamu,
 * obsahuje tedy ukazatel na nasledujici zaznam. Pokud
 * je hodnota NULL, aktualni polozka je posledni v seznamu.
 *
 * Dale obsahuje ukazatel na data v pameti
 */
typedef struct TGarbageRec {
  void *ptr;
  struct TGarbageRec *next;
}TGarbageRec;


/**
 * Reprezentuje samotny garbage collector. Obsahuje
 * ukazatel na prvni a posledni polozku seznamu
 */
typedef struct TGarbage {
  TGarbageRec *head;
  TGarbageRec *tail;
  int length;
} TGarbage;



/**
 * Funkce inicializuje garbage collector. Volat pouze jednou,
 * na zacatku programu.
 *
 * Manipuluje s globalni promennou `garbage` tohoto modulu.
 *
 * @return 0, pokud vse probehne v poradku, -1 pri chybe
 */
int initGarbageCollector();

/**
 * Projde vsechny polozky garbage collectoru a postupne
 * je vsechny uvolni. nakonec uvolni samotny garbage collector.
 */
void freeGarbage();


/**
 * Alokuje blok pameti o velikosti `size`. Funkce zachovava
 * API standardni funkce malloc(...). Vraci tedy NULL, pokud
 * se alokace nepovede. Jedina odlisnost oproti funkci malloc je,
 * ze reference na nove alokovany blok je ulozena do garbage collectoru.
 * Pamet muze byt tedy uklizena hromadne.
 *
 * Diky faktu, ze je zachovano rozhrani mallocu, neni potreba implementovat
 * specialni funkce pro realokaci pameti.
 *
 * Varovani: Pamet alokovana touto funci nemuze byt uklizena
 * standardni funkci free().
 *
 * @param size velikost alokovaneho bloku
 *
 * @return Adresu v pameti nove alokovaneho bloku typu (void *), NULL pri chybe.
 */
void *gcMalloc(size_t size);

/**
 * Provede realokaci pameti z adresy `ptr` na velikost `size`.
 * Zachovava rozhrani standardni funkce realloc. Navic preda informaci
 * o realokaci take garbage collecotru.
 *
 * @param *ptr ukazatel na blok pameti, urceny k reallokaci
 * @param size konecna velikost
 *
 * @return adresu nove realokovaneho zdroje,
 */
void *gcRealloc( void *ptr, size_t size);

/**
 * Nalezne zaznam v garbage collectoru, jehoz datovy ukazatel
 * odkatuje na adresu `ptr`. Vrati ukazatel na tento zaznam.
 *
 * @param ptr hledaná adresa
 *
 * @return Adresu patricneho zaznamu garbage collectoru | NULL pokud nenalezen
 */
TGarbageRec *gcFind(void *ptr);



///LOKALNI


/**
 * Funkce inicializuje lokalni garbage collector.
 *
 * Manipuluje s promennou `garbage` tohoto modulu.
 *
 * @param *gc ukazatel na garbage collector
 *
 * @return 0, pokud vse probehne v poradku, -1 pri chybe
 */
int initGarbageCollectorLocal(TGarbage** garbage);

/**
 * Projde vsechny polozky garbage collectoru a postupne
 * je vsechny uvolni. nakonec uvolni samotny garbage collector.
 */
void freeGarbageLocal(TGarbage** garbage);

/**
 * Prida novou polozku na konec garbage collecotoru.
 * Nastavi ukazatel na posledni polozku seznamu (garbage->tail)
 * na tento zaznam.
 *
 * Pokud je zaznam pridan jako prvni, jsou reference garbage collectoru
 * head i tail nastaveny na tutuo polozku
 *
 * @param *garbage Ukazatel na garbage collector
 * @param *ptr Ukazatel na polozku, ktera bude pridana na konec seznamu
 */
void garbageAppendLocal(TGarbage** garbage,TGarbageRec *ptr);

/**
 * Alokuje blok pameti o velikosti `size`. Funkce zachovava
 * API standardni funkce malloc(...). Vraci tedy NULL, pokud
 * se alokace nepovede. Jedina odlisnost oproti funkci malloc je,
 * ze reference na nove alokovany blok je ulozena do garbage collectoru.
 * Pamet muze byt tedy uklizena hromadne.
 *
 * Diky faktu, ze je zachovano rozhrani mallocu, neni potreba implementovat
 * specialni funkce pro realokaci pameti.
 *
 * Varovani: Pamet alokovana touto funci nemuze byt uklizena
 * standardni funkci free().
 *
 * @param *garbage Ukazatel na garbage collector
 * @param size velikost alokovaneho bloku
 *
 * @return Adresu v pameti nove alokovaneho bloku typu (void *), NULL pri chybe.
 */
void *gcMallocLocal(TGarbage** garbage,size_t size);

/**
 * Nalezne zaznam v garbage collectoru, jehoz datovy ukazatel
 * odkatuje na adresu `ptr`. Vrati ukazatel na tento zaznam.
 *
 * @param *garbage Ukazatel na garbage collector
 * @param ptr hledaná adresa
 *
 * @return Adresu patricneho zaznamu garbage collectoru | NULL pokud nenalezen
 */
TGarbageRec *gcFindLocal(TGarbage** garbage,void *ptr);

/**
 * Provede realokaci pameti z adresy `ptr` na velikost `size`.
 * Zachovava rozhrani standardni funkce realloc. Navic preda informaci
 * o realokaci take garbage collecotru.
 *
 * @param *garbage Ukazatel na garbage collector
 * @param *ptr ukazatel na blok pameti, urceny k reallokaci
 * @param size konecna velikost
 *
 * @return adresu nove realokovaneho zdroje,
 */
void *gcReallocLocal(TGarbage** garbage, void *ptr, size_t size);




#endif /* GARBAGE_H_ */
