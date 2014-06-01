/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : interpret.h - Rozhrani modulu interpret.h
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */
#ifndef _INTERPRET_H_
#define _INTERPRET_H_


#include "ilist.h"
#include "builtins.h"
#include "ial.h"
#include "error.h"

/**
 * Provede interpretaci predaneho seznamu instrukci
 * @param ilist Interpretovany seznam instrukci
 * @param funTable Globalni tabulka funkci
 * @param TSstack Globalni zasobnik tabulek symbolu
 */
int interpret(tIlist * ilist, htable_t * funTable, htableStack * TSstack);


#endif
