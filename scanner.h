/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : scanner.h - Rozhrani modulu scanner.c
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */
#ifndef SCANNER_H_
#define SCANNER_H_


#include <stdio.h>
#include <stdlib.h>

#include "str.h"

/**
 * Stavy konecneho automatu
 */
typedef enum {
  S_START,                 /* 00 Pocatecni stav */
  S_IDENT,                 /* 01 Identifikator */
  S_VAR_IDENT,             /* 02 Identifikator promenne */
  S_VAR_DOLLAR,            /* 03 Byl prijat dolar - neni koncovy */
  S_L_PARENTH,             /* 04 Leva kulata zavorka ( */
  S_R_PARENTH,             /* 05 Prava kulata zavorka ) */
  S_L_BRACE,               /* 06Leva slozena zavorka { */
  S_R_BRACE,               /* 07 Prava slozena zavorka } */
  S_ASSIGN,                /* 08 Operator prirazeni '=' */
  S_EQ_POSSIBLE,           /* 09 Byly prijaty dve '=' - neni koncovy */
  S_NEQ_POSSIBLE,          /* 10 Byl prijat '!' a '=' */
  S_EQ,                    /* 11 Porovnani === */
  S_NEQ,                   /* 12 Nerovnost !== */
  S_ADD,                   /* 13 Scitani */
  S_SUB,                   /* 14 Odcitani */
  S_MUL,                   /* 15 Nasobeni */
  S_CONCAT,                /* 16 Konkatenace */
  S_DIV,                   /* 17 Deleni */
  S_LT,                    /* 18 <  */
  S_LEQ,                   /* 19 <= */
  S_GT,                    /* 20 >  */
  S_GEQ,                   /* 21 >= */
  S_DEC,                   /* 22 Cele cislo */
  S_REAL,                  /* 23 Realne cislo */
  S_REAL_DOT,              /* 24 Po celem cisle prisla tecka - neni koncovy */
  S_REALEXP_E,             /* 25 V DEC | REAL prisel znak exponentu e,E - NK*/
  S_EXPREAL_SIGN,          /* 27 Po exponentu priselo volitelne znaminko +,- NK */
  S_LINECOMM,              /* 29 Radkovy komentar - NK */
  S_BLOCKCOMM,             /* 30 Blokovy komentar - NK */
  S_BLOCKCOMM_BREAK,       /* 31 V blokovem komentari prisla '*' - NK */
  S_SEMICOLON,             /* 32 Strednik */
  S_EXCLAMATION,           /* 33 Vykricnik */
  S_EOF,                   /* 34 Konec souboru */
  S_COMMA,
  S_REALEXP, /* Destinny literal zapsany v exponencialnim tvaru */

  S_STR_LOAD,
  S_STR_ESC,
  S_STRING,
  S_STR_HEXA,
  S_STR_HEXA_1,
  S_STR_HEXA_2,

  /* Rezervovana slova */
  KW_FUNC, // pri uprave ponechat jako prvni!
  KW_IF,
  KW_ELSEIF,
  KW_ELSE,
  KW_RET,
  KW_WHILE,
  KW_FALSE,
  KW_TRUE,
  KW_FOR,
  KW_CONT,
  KW_BREAK,
  KW_NULL, // pri uprave ponechat jako posledni!

}tState;


typedef struct tToken {
  TString * data;  // hodnota tokenu
  tState state;  // stav tokenu
  int line;     //radek tokenu
} tToken;



/**
 * Nastavi hodnoty reprezentujici token na patricne hodnoty.
 * Vraci konstanty EOK, pokud byl token nacten spravne, chybovou
 * konstantu E_LEX, pokud nastala chyba v rámci lexikální nalýzy
 *
 * @param tokenval Hodnota tokenu
 * @param state Typ tokenu
 * @return EOK | E_LEX | EOF
 */
int getToken( TString *tokenval, tState * state, int * line);

/**
 * Nastavi zdrojovy soubor
 *
 * @param file Popisovac souboru
 */
void setSourceFile(FILE * file);

/**
 * Nacte prvnich N znaku (kde N je delka oteviraci znacky) ze zdrojoveho
 * souboru, potrebnych pro overeni, zda soubor obsahuje validni oteviraci
 * znacku '<?php' nasledovanou prazdnou mezerou. Tuto sekvenci nesmi
 * predchazet zadny jiny znak!
 *
 * NOTE: Celkove nacte tedy prvnich N+1 znaku!
 *
 * @return TRUE pokud je zdrojovy soubor spravne otevren, jinak FALSE
 */
int hasValidOpenTag();

void printStateLabel(tState state);

void printTillLF();

#endif /* SCANNER_H_ */
