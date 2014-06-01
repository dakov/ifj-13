/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : parser.h - Rozhrani modulu parser.c
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */
#ifndef PARSER_H_
#define PARSER_H_


#include "error.h"
#include "scanner.h"
#include "ilist.h"
#include "ial.h"

/**
 * Provede syntaktickou analyzu zdrojoveho programu shora dolu metodou
 * rekurzivniho sestupu kombinovanou (pro kontrolu správnosti struktury
 * zapsaneho programu) kombinovanou s analyzou zdola nahoru precedencni
 * syntaktickou analyzou (pouze pro vyrazy).
 *
 * @param fT Globalni tabulka funkci do ktere se generuji funkce
 * @param iL Globalni seznam do ktereho se budou generovat instrukce
 *
 * return Chybovou konstantu podle toho, zda analyza probehla spravne.
 */
int parse(htable_t *fT, tIlist *iL,htableStack *sS);







#endif /* PARSER_H_ */
