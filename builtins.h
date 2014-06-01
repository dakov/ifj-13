/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : builtins.h - Rozhrani modulu builtins.c
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */


#ifndef BUILTINS_H_
#define BUILTINS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>


#include "error.h"
#include "str.h"
#include "garbage.h"
#include "ilist.h"

/**
 * Konstanty identifikujici typ promenne v konkretnim case.
 */
#define T_NULL 0
#define T_INT 1
#define T_REAL 2
#define T_STR 3
#define T_BOOL 4
#define T_FUNC 5

typedef struct TFunc {
  int argc; //pocet parametru
  TString ** argv; //vektor nazvu parametru
  tListItem * start; // prvni instrukce na instrukci pasce
} TFunc;

/**
 * Struktura reprezentucijici "obecny datovy typ",
 * jehoz typ je udan dynamicky dle hodnoty, kterou
 * aktualne nabyva.
 */
typedef union data{
    TString *str;         //string
    double real;           //realne cislo
    int integer;           //cele cislo
    bool boolean;          //bool
    void * null;
    TFunc function;    //funkce
} TData;

/**
 * Typ reprezentujici daty hodnotu s potenciálem měnit svůj typ
 */
typedef struct TVar{
  int type;
  TData data;
} TVar;


/*
 * Funkce pro implictni pretypovani. Ridi se tabulkou
 * konverzi viz. zadani projektu.
 */

/**
 * Vraci hodnotu promenne 'var' konvertovanou na pravdivostni hodnotu.
 *
 * @return Hodnotu typu bool odpovidajici hodnote var po pretypovani.
 */
TVar * boolval(TVar *var, int * result);

/**
 * Vrati hodnotu promenne var, prevedenou na realne cislo typu double.
 *
 * Pri konverzi retezce na double jsou nejdrive oriznuty vsechny
 * uvozujici znaky, dále je z retezce nacteno desetinne cislo dle
 * definice az po prvni nevyhovujici znak (nebo konec retezce).
 * Zbytek retezce (vcetne prvniho nevyhovujiciho znaku) je ignorovan.
 *
 * Neni-li timto zpusoben nacten ani jeden znak, vraci hodnotu 0.0
 * Je-li takto nactena poslounost znaku nevyhovujici definici desetinneho
 * literalu, je vracena chyba E_INT_CAST.
 *
 * @param var Zdrojova promenna.
 * @return Hodnota var konvertovanou na desetinne cislo.
 */
TVar * doubleval(TVar *var, int * result);


/**
 * Vrati hodnotu argumentu var konvertovanou na hodnotu typu int.
 *
 * Pri konverzi retezce na cele cislo jsou nejdrive oriznuty vsechny
 * uvozujici znaky, dále je z retezce nacteno cele cislo - dle
 * definice - az po prvni nevyhovujici znak (nebo konec retezce).
 * Zbytek retezce (vcetne prvniho nevyhovujiciho znaku) je ignorovan.
 *
 * Neni-li timto zpusoben nacten ani jeden znak, vraci 0.
 * Pri chybe vraci E_INT_CAST.
 *
 * @param var Zdrojova promenna.
 * @return Hodnota var konvertovanou na cele cislo.
 */
TVar * intval(TVar *var, int * result);

/**
 * Vrati hodnotu agrumentu var konvertovanou na retezec.
 * Konverzi celeho resp. desetinneho cisla provadi podle
 * standardnich pravidel formatu %d resp. %g
 */
TVar * strval(TVar *var, int * result);

/**
 * Nacte jeden radek ze stdin a vrati jeho hodnotu a vrati jej
 * jako hodnotu typu string.
 *
 * Precteny znak konec radku / konce souboru nejsou soucasti
 * vystupni hodnoty.
 *
 * Pokud jiz na vstupu nejsou zadna data (prvni precteny znak je
 * EOF) vraci prazdny retezec "".
 *
 * @return Hodnotu nacteneho radku jako typ string.
 */
TVar * get_string();

/**
 * Vypise hodnoty parametru str1,str2,..,strN na stdin ihned za sebe,
 * nevklada zadne oddelovace.
 *
 * Vraci pocet skutecne obdrzenych argumentu jako cele cislo.
 */
int put_string();



#endif /* BUILTINS_H_ */
