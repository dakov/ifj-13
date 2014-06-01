/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : expr.h - Rozhrani modulu expr.c
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "builtins.h"

/**
 * Indexy do precedencni tabulky. Kazdy token je na preveden na tuto hodnotu.
 */
typedef enum {
  PT_MUL,      // * 0
  PT_DIV,      // / 1
  PT_ADD,      // + 2
  PT_SUB,      // - 3
  PT_CONCAT,   // . 4
  PT_LT,       // < 5
  PT_GT,       // > 6
  PT_LEQ,      // <= 7
  PT_GEQ,      // >= 8
  PT_EQ,       // === 9
  PT_NEQ,      // !== 10
  PT_LPARENTH, // ( 11
  PT_I,        // i 12
  PT_RPARENTH, // ) 13
  PT_DELIM,    // $ 14
  PT_USUB,       //u- 15
  /* dalsi symboly / pomocne informace */
  EX_ERROR, // První prvek enum je prvek naznačující chybu (tento prvek bude vrácen např. při operaci top() nad prázdným zásobníkem)
  /* symboly v precedencni tabulce */
  PTL,        // < 17
  PTR,        // > 18
  PTE,        // = 19
  PTX,        // X 20 blank field
  PT_E,          //E 21 neterminal
} TExpr;

/**
 * Polozka zasobniku precedencni analyzy
 */
typedef struct{
  TData value;
  TExpr data;
  tState type;
}TStackItem;

/**
 * Zasobnik precedencni syntakticke analyzy
 */
typedef struct {
  TStackItem *data;
  int top;
  int allocated;
} TExprStack;




/**
 * Provadi syntaktickou analyzu zalozenou na precedencni tabulce.
 * Pred vstupem ocekava, ze ma jiz jeden token nacteny.
 *
 * @return EOK pokud probehla analyza vporadku, jinak patricnou chybu
 */
int precSA();

/** Alokace a inicializace prázdného zásobníku
 * Příklad použití:
 * TExprStack* stack = StackInit();
 *
 * (Všechny následující operace se zásobníkem
 *  jsou automaticky prázdné operace, pokud
 *  není zásobník správně inicializovaný)
 *
 * @return Odkaz na prázný alokovaný zásobník
 */
TExprStack* stackInit();

/** Zjistí, zda-li je daný zásobník prázný
 *
 * @param s Odkaz na zásobník
 * @return Nula pokud není, nenulová hodnota pokud je
 */
int stackEmpty(TExprStack* s);

/** Získání hodnoty na vrcholu zásobníku
 * (Bez jeho odebrání; pouze přečtení)
 * (Pokud je zásobník prázdný, bude vráceno EX_ERROR)
 *
 * @param s Odkaz na zásobník
 * @return Prvek enum TExpr - vrchol zásobníku | EX_ERROR
 */
TExpr stackTop(TExprStack* s);

/** Získání hodnoty na vrcholu zásobníku
 * (Bez jeho odebrání; pouze přečtení)
 * (Pokud je zásobník prázdný, bude vráceno EX_ERROR)
 *
 * @param s Odkaz na zásobník
 * @return Prvek enum TStackItem - vrchol zásobníku | EX_ERROR
 */
TStackItem stackTopItem(TExprStack* s);

/** Odstranení prvku z vrcholu zásobníku
 * (bez jeho přečtení, pouze odebrání)
 * (je-li zásobník prázný, prázdná operace)
 *
 * @param s Odkaz na zásobník
 */
void stackPop(TExprStack* s);

/** Vložení prvku enum TExpr na vrchol zásobníku
 *
 * @param s Odkaz na zásobník
 * @param d Data ke vložení - enum TExpr
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit zasobnik
 */
int stackPush(TExprStack* s, TExpr d);

/** Vložení prvku TStackItem na vrchol zásobníku
 *
 * @param s Odkaz na zásobník
 * @param d Data ke vložení - TStackItem
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit zasobnik
 */
int stackPushItem(TExprStack* s, TStackItem d);

/** Provedeni operace mezi dvemi E
 *
 * @param x  prvni operand a zaroveni destinatinou
 * @param y  druhy operand
 * @param op  operator
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit zasobnik
 */
int doTheMath(TStackItem *x, TStackItem *y, TExpr op);


/** generuje jedinecne nazvy identifikatoru
 *  nazev se sklada ze znaku # nasledovanym cislem
 * postupne se tu generuji prirozena cisla a do nazvu promenne se ukladaji
 * v reverzovanem poradi - na funkcnost to nema vliv, ale je jednodussi implementace
 *
 * @param var TString do ktereho se ulozi
*/
void generateVariable(TString *var);

/** prevede konstantu na promennou s unikatnim nazvem
 *
 * @param state je stav tokenu tState
 * @param string je hodnota tokenu TString
 *
 * @return TVar * na novou promennou, NULL kdyz byla interni chyba
*/
TVar * constantToVar(tState * state,TString * string);

int tokenToData(TData *data,tState * state,TString * string);

int generateTrue();

int generateFalse();
