/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : error.c - modul pro obsluhu chybovych stavu programu
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

#include "error.h"
#include "str.h"

extern TString tkAttr;
extern int tkLine;

extern void printTillLF();

//TODO: pole constantnich retezcu pro tisk zpravy.


/**
 * Vraci konstantu identifikujici lexikalni chybu. Pokud je nastaven debugovaci
 * mod - VERBOSE_MODE v error.h, vytiskne chybovou zpravu.
 *
 * @return Konstantu E_LEX
 */
int lexError(){ //1

#if VERBOSE_MODE
 fprintf( stderr ,"* Radek %d * Neplatny lexem: %s\n", tkLine, tkAttr.string);
 fprintf( stderr,  "* Ukazka * %s" , tkAttr.string);
 printTillLF();
#endif

 return E_LEX;
}

int syntaxError() { //2
#if VERBOSE_MODE
  fprintf( stderr ,"* Radek %d * Syntakticka chyba\n", tkLine );
  fprintf( stderr ,"* Neockavany token * %s\n", tkAttr.string);
  fprintf( stderr,  "* Ukazka * %s" , tkAttr.string);
  printTillLF();
#endif

  return E_SYNTAX;
}


int funSemanticError() { // 3
#if VERBOSE_MODE
  fprintf( stderr ,"* Radek %d * Semanticka chyba 3\n", tkLine );
  fprintf( stderr ,"* ne/redefinovana funkce * %s\n", tkAttr.string);
  fprintf( stderr,  "* Ukazka * %s" , tkAttr.string);
  printTillLF();
#endif

  return E_SEMANTIC;
}

int argumentError(char * var){ //4
#if VERBOSE_MODE
  fprintf( stderr ,"* Funkce ocekava vice argumentu * %s\n", var);
#endif
  return E_ARG;
}

int undeclarVarError(TString * var){ //5
#if VERBOSE_MODE
  fprintf( stderr ,"* Nedeklarovana promenna * %s\n", var->string);
  printTillLF();
#endif
  return E_UNDECLARED;
}

int zeroDivError(){ //10
#if VERBOSE_MODE
  fprintf(stderr, "* Pokus o deleni nulou *\n");
#endif
  return E_ZERO_DIV;
}


int castError() {//11
#if VERBOSE_MODE
  fprintf( stderr ,"* chyba pri pretypovani na cislo * %s\n", tkAttr.string);
#endif
  return E_INT_CAST;
}

int incompatibleTypesError() { //12
#if VERBOSE_MODE
  fprintf(stderr, "* Typová nekompatibilita *\n");
#endif
  return E_TYPE_COMPAT;
}

int otherSemanticError(const char *msg) { // 13
#if VERBOSE_MODE
  fprintf(stderr, "*Jina semanticka/behova chyba*\n");
  fprintf(stderr, "%s\n", msg);
  fprintf(stderr, "\n");
#endif
  return E_RUNTIME_SEMANT;
}


int internalError() { //99
#if VERBOSE_MODE
  fprintf( stderr, "* Vnitrni chyba interpretu!*\n");
#endif
  return E_INTERN;

}

