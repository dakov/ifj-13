/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : str.h - Rozhrani modulu str.c
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */
#ifndef STRING_H_
#define STRING_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "garbage.h"



typedef struct {
	char *string;
	int length;
	int allocated;
} TString;

/**
 * Provede inicializaci typu TString zapouzdrujici
 * retezce. Alokuje datovy atribut `string` na
 * velikost BUFFINC, delka retezce `length` je nastavena
 * na 0.
 * Jako prvni znak retezce je nastaven znak '\0'
 *
 * @param str Ukazatel na inicializovanou strukturu
 * @return Konstantu STROK, nebo STRERR pri chybe
 */
int strinit(TString *str);


/**
 * Provede inicializaci typu TString zapouzdrujici
 * retezce. Alokuje datovy atribut `string` na
 * velikost BUFFINC, delka retezce `length` je nastavena
 * na 0.
 * A na�te do n�j �et�zec z druh�ho parametru
 *
 * @param str Ukazatel na inicializovanou strukturu
 * @param s �et�zec kter� se zkop�ruje
 *
 * @return Konstantu STROK, nebo STRERR pri chybe
 */
int strInitDefault(TString * str, char * s);

/**
 * Prida jeden znak na konec retezce. Pokud nestaci
 * jit alokovana pamet, pokusi se o jeji rozsireni.
 *
 * @param str Retezec typu TString, k nemuz se ma znak pridat
 * @param c Znak, ktery se ma pridat na konec retezce
 *
 * @return STROK nenastala chyba, jinak STRERR
 */
int strCharAppend(TString *str, char c);

/**
 * Resetuje atributy rezezce. Pokud je jiz alokovana pamet
 * vetsi nez byla pri inicializaci, NEBUDE velikost aktualne
 * alokovane snizena.
 *
 * Nastavi delku retezce na 0 a znak '\0' umistni na index 0.
 */
void strReset(TString *str);

/**
 * Porovna retezce zapouzdrene v strukturach typu TString.
 * Zachovava stejne chovani jako standardni funkce strcmp.
 *
 * @return viz manualove stranky strcmp
 */
int strTStringCmp(const TString *str1, const TString *str2);

/**
 * Porovna retezec zapouzdreny v strukture typu TString s normalnim ceckovskym.
 * Zachovava stejne chovani jako standardni funkce strcmp.
 *
 * @return viz manualove stranky strcmp
 */
int strConstTStringCmp(TString *str1, char* str2);

/**
 * Kopiruje retezec zapouzdrene v strukturach typu TString.
 * Podle standartni funkce strcpy je prvni parametr cil a druhy zdroj.
 *
 * @param str Retezec typu TString do ktereho se ma kopirovat.
 * @param str Retezec typu TString ze ktereho se ma kopirovat.
 *
 * @return Konstrantu STROK nebo STRERR pri chybe
 */
int strTStringCpy(TString *destination, const TString *source);

/**
 * Vrati delku retezce v str
 * Podle standartni funkce strcpy je prvni parametr cil a druhy zdroj.
 *
 * @param str Retezec typu TString
 *
 * @return delka retezece
 */
int strTStringLen(TString *str);

/**
 * Konkatenace retezcu dest = str1.str2 
 *
 * @param str Retezec typu TString do ktereho vleze vysledek.
 * @param str Retezec typu TString .. prvni cast  
 * @param str Retezec typu TString .. druha cast
 *
 * @return Konstrantu STROK nebo STRERR pri chybe
 */
int strTStringCat(TString ** destination, TString * source1, TString * source2);


#endif /* STRING_H_ */
