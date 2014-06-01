/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : ilist.h - Rozhrani modulu ilist.c
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */

#ifndef ILIST_H_
#define ILIST_H_


#include <stdio.h>
#include <stdlib.h>

/**
 * Typy instrukci
 */
typedef enum {
  /* Aritmeticke operace */
  I_ADD, // (dest, op1, op1)
  I_SUB, // (dest, op1, op1)
  I_MUL, // (dest, op1, op1)
  I_DIV, // (dest, op1, op1)

  I_CONCAT, // (dest, op1, op2) konkatenace

  /* relacni operace*/
  I_EQ, // (dest, op1, op1)
  I_NEQ, // (dest, op1, op1)
  I_LE, // (dest, op1, op1)
  I_LEQ, // (dest, op1, op1)
  I_GE, // (dest, op1, op1)
  I_GEQ, // (dest, op1, op1)
  I_NEG, // (dest, op1, - )

  /* logicke operace */
  I_NOT, // (dest, op, - )

  /* prirazeni */
  I_ASSIGN, // (dest, op, - )

  /* instrukce skoku */
  I_JMP, // (adr, - , - ) : skoci na adresu adr (nepodminene)
  I_IFJUMP, // (adr, cond, - ) : skoci na adresu adr, pokud plati podminka cond
  /**
   * Slouzi jako adresa funkce (instrukce call pak musi pristupovat az k dalsi adrese!)
   * Semantika: pokud interpret narazi na instrukci I_FSKIP, preskoci vsechny instrukce
   * az po instrukci I_FSTOP
   */
  I_FSKIP, // (-,-,-) => nema operandy
  I_FSTOP, //zarazka pro instrukci I_FSKIP: (-,-,-)

  /*
   * - Vrati rizeni na instrukci ulozenou v tabulce symbolu,
   * - odsrani tabulku symbolu na vrcholu zasobniku,
   * - do nadrazene TS ulozi navratovou hodnotu
   */
  I_RET, // format (-, -, -)

  /*
   * - Preda rizeni volane funkci
   * - Volani teto instrukce musi predchazet N instrukci I_PUSHPAR, kdy N
   * je pocet parametru volane funkce.
   * - Vyvori novou TS na vrchol zasobniku TS,
   * - Vyhleda funkci v tabulce funkci
   * - (KDO PO SOBE UKLIDI?)
   */
  I_CALL, // format (id_funkce, adresa_kam_se_vynori, - )

  /* priznak navesti urcuje jaky kod nasleduje, pokud se jedna o priznak funkce, je preskocena
   * pri linearnim pruchodu instrukcnim seznamem  */
  I_LABEL, // navesti urcene pro skoky (priznak, -, -)

  /* zasobnik parametru*/
  I_PUSHPAR, // pushne na zasobnik argumentu hodnotu
  I_POPPAR, // odstrani a vrati hodnotu z vrcholu zasobniku
  I_NOP,  // prazdna operace

  I_STOP, //konec programu

  /*
   * Instrunkce pro vestavene funkce, tytot instrukce nemaji zadne operandy
   * tvari se sice jako funkce, ale neprovadi zadny skok, nicmene dodrzuji
   * semantiku funkci (vraci hodnotu na urcene misto dle konvence, ...)
   * Nemaji zadne operandy (neni potreba, ne, argumenty ctou z fronty)
   */
  I_BOOLVAL,
  I_DOUBLEVAL,
  I_INTVAL,
  I_STRVAL,
  I_GETSTR,
  I_PUTSTR,
  I_STRLEN,
  I_GET_SUBSTR,
  I_FIND_STR,
  I_SORT_STR,

} tInstrType;

/**
 * Typ reprezentujici 3AC instrukci
 */
typedef struct tInstr {
  tInstrType type;  // typ instrukce
  void *op1; // adresa 1
  void *op2; // adresa 2
  void *op3; // adresa 3
} tInstr;

/**
 * Polozka seznamu, jejiz datovou slozkou je konkretni instrukce
 */
typedef struct listItem {
  tInstr instr;
  struct listItem *nextItem;
} tListItem;

/**
 * Model reprezentujici seznam instrukci, aktivni prvek predstavuje aktualni instrukci
 */
typedef struct tIList {
  struct listItem *first;  // ukazatel na prvni prvek
  struct listItem *last;   // ukazatel na posledni prvek
  struct listItem *active; // ukazatel na aktivni prvek / aktualni instrukci
} tIlist;

void listInit(tIlist *L);
void listFree(tIlist *L);
int listInsertLast(tIlist *L, tInstr I);
int listInsertLastItem(tIlist *L, tListItem *i);
void listFirst(tIlist *L);
void listNext(tIlist *L);
void listGoto(tIlist *L, void *gotoInstr);
void *listGetPointerLast(tIlist *L);
tInstr *listGetData(tIlist *L);


void printIList(tIlist *L);
void printInstr(tInstr *i);

/**
 * Generuje instrukci na zaklade predany parametru. Nová instrukce je pridana nakonec
 * existujici seznamu.
 * @param L Pracovni seznam
 * @param type Typ instrukce
 * @param op1 Prvni operand insturkce
 * @param op2 Druhy operand insturkce
 * @param op3 Treti operand insturkce
 *
 * @return Zda se pridani povedlo
 */
int igenerate(tIlist *L,tInstrType type, void * op1, void * op2, void * op3);

void * igenerateNoAppend(tInstrType type, void * op1, void * op2, void * op3);
#endif /* ILIST_H_ */
