/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : error.h - Rozhrani modulu error.c
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral
 ============================================================================
 */
#ifndef ERROR_H_
#define ERROR_H_

#include "str.h"
/* Definovane chybove stavy a navratove kody */

#define EOK                 0 // Bezchybovy stav
#define E_LEX               1 // Chyba pri lexikalni analyze
#define E_SYNTAX            2 // Chyba pri syntakticke analyze
#define E_SEMANTIC          3 // Semanticka chyba
#define E_ARG               4 // Semanticka-behova: chybejici parametr pri volani
#define E_UNDECLARED        5 // Semanticka-behova: Nedeklarovana promenna
#define E_ZERO_DIV         10 // Semanticka-behova: Deleni nulou
#define E_INT_CAST         11 // Semanticka-behova: chyba pri pretypovani na cislo
#define E_TYPE_COMPAT      12 // Semanticka-behova: typova nekomatibilita ve vyrazech
#define E_RUNTIME_SEMANT   13 // Semanticka-behova: ostatni behove chyby
#define E_INTERN           99 // Vnitrni chyba interpretu


// 1 vypisuj informace, 0 bud zticha
#define VERBOSE_MODE 1

/**
 * Vraci konstantu E_LEX, pokud je nastaven priznak VERBOSE_MODE, tiskne take
 * odpovidajici zpravu na stderr
 * @return E_LEX
 */
int lexError();
int syntaxError();
int internalError();
int funSemanticError();
int undeclarVarError(TString *);
int argumentError(char *);
int zeroDivError();
int castError();
int incompatibleTypesError();
int otherSemanticError(const char *);


#endif /* ERROR_H_ */
