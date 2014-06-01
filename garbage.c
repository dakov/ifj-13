/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : garbage.c - Modul pro praci s dynamicky alokovanou pameti
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include "garbage.h"

/**
 * Promenna uchovavajici ukazatel na garbage collector.
 * Tento ukazatel by nemel byt pristupny mimo tento modul.
 */
TGarbage *garbage;

/**
 * Funkce inicializuje garbage collector. Volat pouze jednou,
 * na zacatku programu.
 *
 * Manipuluje s globalni promennou `garbage` tohoto modulu.
 *
 * @return 0, pokud vse probehne v poradku, -1 pri chybe
 */
int initGarbageCollector() {
  return initGarbageCollectorLocal(&garbage);
}

/**
 * Projde vsechny polozky garbage collectoru a postupne
 * je vsechny uvolni. nakonec uvolni samotny garbage collector.
 */
void freeGarbage() {
  freeGarbageLocal(&garbage);
}

/**
 * Prida novou polozku na konec garbage collecotoru.
 * Nastavi ukazatel na posledni polozku seznamu (garbage->tail)
 * na tento zaznam.
 *
 * Pokud je zaznam pridan jako prvni, jsou reference garbage collectoru
 * head i tail nastaveny na tutuo polozku
 *
 * @param *ptr Ukazatel na polozku, ktera bude pridana na konec seznamu
 */
void garbageAppend(TGarbageRec *ptr){
  garbageAppendLocal(&garbage,ptr);
}

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
void *gcMalloc(size_t size){
  return gcMallocLocal(&garbage,size);
}
/**
 * Nalezne zaznam v garbage collectoru, jehoz datovy ukazatel
 * odkatuje na adresu `ptr`. Vrati ukazatel na tento zaznam.
 *
 * @param ptr hledaná adresa
 *
 * @return Adresu patricneho zaznamu garbage collectoru | NULL pokud nenalezen
 */
TGarbageRec *gcFind(void *ptr) {
  return gcFindLocal(&garbage,ptr);
}

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
void *gcRealloc( void *ptr, size_t size) {
  return gcReallocLocal(&garbage,ptr,size);
}




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
int initGarbageCollectorLocal(TGarbage** garbage) {
  // alokuje strukturu collectoru
  if ( ((*garbage) = malloc(sizeof(TGarbage))) == NULL ) {
    return -1;
  }

  //inicializace hodnot
  (*garbage)->head = NULL;
  (*garbage)->tail = NULL;
  (*garbage)->length = 0;
  return 0;

}

/**
 * Projde vsechny polozky garbage collectoru a postupne
 * je vsechny uvolni. nakonec uvolni samotny garbage collector.
 */
void freeGarbageLocal(TGarbage** garbage) {
  TGarbageRec *rec = (*garbage)->head;

  // projde cely linearni seznam
  while (rec != NULL) {
    //uvolni data
    free(rec->ptr);
    rec->ptr = NULL;

    // zalohuj ukazatel na dalsi prvek
    TGarbageRec *tmpNext = rec->next;

    //uvolni celou strukturu zaznamu
    free(rec);

    // nastav zaznam pro dalsi iteraci
    rec = tmpNext;
  }

  //uvolni cely garbage collector
  free((*garbage));
}

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
void garbageAppendLocal(TGarbage** garbage,TGarbageRec *ptr){
  // projistotu vynuluj ukazatel na nasledujici polozku
  ptr->next = NULL;
  //je tohle prvni zaznam v seznamu?
  if ((*garbage)->head == NULL){
    (*garbage)->head = ptr;
    (*garbage)->tail = ptr;
  } else { // neni prvni -> pridej na konec
    (*garbage)->tail->next = ptr;
    (*garbage)->tail = ptr;
  }

}

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
void *gcMallocLocal(TGarbage** garbage,size_t size){

  void *ptr = malloc(size);

  if (ptr == NULL) {
    return NULL;
  }

  // alokuj garbage record
  TGarbageRec *pGarRec = malloc(sizeof(TGarbageRec));

  if (pGarRec == NULL) {
    // uvolni jiz alokovany zdroj
    free(ptr);
    return NULL;
  }

  pGarRec->ptr = ptr;
  pGarRec->next = NULL;

  garbageAppendLocal(garbage,pGarRec);

  return ptr;
}
/**
 * Nalezne zaznam v garbage collectoru, jehoz datovy ukazatel
 * odkatuje na adresu `ptr`. Vrati ukazatel na tento zaznam.
 *
 * @param *garbage Ukazatel na garbage collector
 * @param ptr hledaná adresa
 *
 * @return Adresu patricneho zaznamu garbage collectoru | NULL pokud nenalezen
 */
TGarbageRec *gcFindLocal(TGarbage** garbage,void *ptr) {
  TGarbageRec *rec = (*garbage)->head;

  // projde cely linearni seznam
  while (rec != NULL) {
    // je aktualni polozka hledana polozka?
    if(ptr == rec->ptr)
      return rec;

    rec = rec->next;
  }
  // nenalezeno

  return NULL;
}

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
void *gcReallocLocal(TGarbage** garbage, void *ptr, size_t size) {

  //realokuje pamet
  void *nptr = realloc(ptr, size);

  //najdi odpovidajici zaznam v GC
  TGarbageRec *gcrec = gcFindLocal(garbage,ptr);

  //nastala chyba
  if (nptr == NULL || gcrec == NULL)
    return NULL;

  // zmen adresu take v zaznamu garbage collectoru
  gcrec->ptr = nptr;

  // vrat novou adresu
  return nptr;
}
