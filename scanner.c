/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : scanner.c - Provadi lexikalni analyzu zdrojoveho programu
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
#include <string.h>
#include <ctype.h>

#include "scanner.h"
#include "error.h"
#include "garbage.h"
#include "str.h"


/* Delka oteviraci znacky */
#define OPEN_TAG_LENGTH 5
#define OPEN_TAG "<?php"

char* ifj_keywords[] =
  {
      [KW_FUNC] = "function",
      [KW_IF] = "if",
      [KW_ELSEIF] = "elseif",
      [KW_ELSE] = "else",
      [KW_RET] = "return",
      [KW_WHILE] = "while",
      [KW_TRUE] = "true",
      [KW_FALSE] = "false",
      [KW_NULL] = "null",
      [KW_CONT] = "continue",
      [KW_BREAK] = "break",
      [KW_FOR] = "for"
  };


/* otevreny popisovac zdrojoveho souboru */
FILE * source;

/**
 * Zjisti zda zadany identifikator neni klicove slovo.
 * Pokud ano, vrati identifikator prislusneho stavu.
 * Pokud ne, vrati hodnotu vychozihot stavu defaultState.
 *
 * @param word Retezec nacteneho identifikatoru
 * @param defaultState vychozi stav, poku neni klicove slovo
 * @return Identifikator stavu (viz. popis funkce)
 */
tState isKeyword(TString * word, tState defaultState){
  char * strtmp = word->string;

  for ( int i = KW_FUNC; i <= KW_NULL; ++i){
    if(strcmp(strtmp, ifj_keywords[i]) == 0){
      return i;
    }
  }

  return defaultState;
}

int isValidStrMember(char c) {
  int lowestAscii = 31; //nejnizsi mozna platna ascii hodnota

  return ( c > lowestAscii ) && c != '"' && c != '$';
}

int isEscSeqChar(char c) {
  return c == 'n' || c == 't' || c == '\\' || c == '"' || c == '$';
}

int isHexaChar( char c ) {
  return (isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

void setSourceFile(FILE * file) {
  source = file;
}

int tokenInit(tToken * tk){
  tk->data = gcMalloc(sizeof(TString));

  if( tk-> data == NULL){
    //clean()
    return internalError();
  }

  tk->line = 0;
  tk->state = S_START;

  return EOK;
}

int hasValidOpenTag() {
  int result;

  TString str;
  strinit(&str);

  int c;

  for( short i = 0; i < OPEN_TAG_LENGTH; ++i) {
    c = fgetc(source);
    result = strCharAppend( &str, c);
    if (result != EOK) return result;
  }

  //nacti oddelovac oteviraci znacky
  int separator = fgetc(source);

  // pokud je oddelovacem znak konce radku, je potreba tento znak vratit,
  // aby pocitani radku pro token opravdu odpovidalo
  if (separator == '\n') ungetc(separator, source);

  return (strcmp(str.string, OPEN_TAG ) == 0 ) && isspace(separator);

}


int getToken( TString *tokenval,  tState *statex, int *line ) {
  tState state = S_START;
  int result;

  int c; // nacteny znaky
  char hexabuff[2];

  //restartuj token
  strReset(tokenval);


  while (1) {

    c = fgetc(source);


    switch ( state ) {
    /* Definice pravidel pocatecniho stavu */
    case S_START: {
      // definice prechodu (START,c) -> state2
      if      ( c == '$' ) state = S_VAR_DOLLAR;
      else if (isalpha(c) || c == '_') state = S_IDENT;
      else if (isdigit(c)) state = S_DEC;
      else if ( c == '(' ) state = S_L_PARENTH;
      else if ( c == ')' ) state = S_R_PARENTH;
      else if ( c == '{' ) state = S_L_BRACE;
      else if ( c == '}' ) state = S_R_BRACE;
      else if ( c == '=' ) state = S_ASSIGN;
      else if ( c == '+' ) state = S_ADD;
      else if ( c == '-' ) state = S_SUB;
      else if ( c == '*' ) state = S_MUL;
      else if ( c == '.' ) state = S_CONCAT;
      else if ( c == '/' ) state = S_DIV;
      else if ( c == '<' ) state = S_LT;
      else if ( c == '>' ) state = S_GT;
      else if ( c == '!' ) state = S_EXCLAMATION;
      else if ( c == ';' ) state = S_SEMICOLON;
      else if ( c == ',' ) state = S_COMMA;
      else if ( c == EOF ) state = S_EOF;
      else if ( c == '"') {
        state = S_STR_LOAD;
        break;
      }
      // TODO: else if ( c == '"' ) state = ;
      else if (isspace(c)) {
        //zvys pocitadlo radku tokenu
        if (c == '\n' ) (*line)++;

        // vyskoc ze switche pro tento znak,
        // restartuj se do pocatecniho stavu
        state = S_START;
        break;
      } else {
        // Neni definovano pravidlo prechodu!
        // CHYBA - TODO
        ungetc(c, source);
        return E_LEX;
      }

      // Pokud nalezl validni pravidlo pro prechod, pridej
      // znak do "tokenu"
      result = strCharAppend(tokenval, c);
      if (result != EOK) return result;

      break;
    } // case S_START

    /*
     * -----------------------------------------------------------
     * Definice uzlu "prvni urovne": existuje pravidlo pro prechod
     * z pocatecniho uzlu do tohoto stavu pomoci jedne hrany
     * -----------------------------------------------------------
     */

    case S_L_PARENTH:
    case S_R_PARENTH:
    case S_L_BRACE:
    case S_R_BRACE:
    case S_ADD:
    case S_SUB:
    case S_MUL:
    case S_CONCAT:
    case S_COMMA:
    case S_SEMICOLON: {
      ungetc(c, source);
      *statex = state;
      return EOK;
    }
    case S_IDENT: {
      if ( isalnum(c) || c == '_'){
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;
        break;
      }

      ungetc(c, source);
      *statex = isKeyword(tokenval, state);
      return EOK;
    }

    case S_VAR_DOLLAR: {
      if ( isalpha(c) || c == '_' ){
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;
        state = S_VAR_IDENT;
        break;
      }

      ungetc(c, source);
      return E_LEX;
    }

    case S_DEC: {

      if ( isdigit(c) ) {
        state = S_DEC;
        result = strCharAppend(tokenval,c);
        if (result != EOK) return result;
      } else if (c == '.' ) {
        state = S_REAL_DOT;
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;

      } else if (c == 'e' || c == 'E') {
        state = S_REALEXP_E;
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;
      }
      else {
        ungetc(c, source);
        *statex = state;
        return EOK;
        //TODO:DALSI STAVY!!!
      }
      break;
    }
    case S_ASSIGN:{
      if (c != '='){
        ungetc(c, source);
        *statex = state;
        return EOK;
      }

      state = S_EQ_POSSIBLE;
      result = strCharAppend(tokenval, c);
      if (result != EOK) return result;

      break;
    }
    case S_EXCLAMATION:{ // !
      if (c != '='){
        ungetc(c, source);
        *statex = state;
        return E_LEX;
      }

      state = S_NEQ_POSSIBLE;
      result = strCharAppend(tokenval, c);
      if (result != EOK) return result;

      break;
    }

    case S_DIV: {
      if ( c == '/') { // radkovy komentar
        strReset(tokenval);
        state = S_LINECOMM; // zustan ve stavu
      } else if ( c == '*' ) { // je blokovy komentar
        strReset(tokenval);
        state = S_BLOCKCOMM;
      } else { // jiny znak, uzavri token
        ungetc(c, source);
        *statex = state;
        return EOK;
      }

      break;
    }

    /* relacni operatory */
    case S_LT: { // '<'
      if (c == '=' ) {
        state = S_LEQ;
        result=strCharAppend(tokenval, c);
        if (result != EOK) return result;
      }
      else {
        ungetc(c, source);
        *statex = state;
        return EOK;
      }

      break;
    }
    case S_GT: { // '>'
      if (c == '=' ) {
        state = S_GEQ;
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;
      }
      else {
        ungetc(c, source);
        *statex = state;
        return EOK;
      }

      break;
    }


    /*
     * ------------------------------------------
     * Uzly druhe urovne
     * ------------------------------------------
     */
    case S_VAR_IDENT: {
      if (isalnum(c) || c == '_') { //zustan ve stavu
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;
        break;
      }
      ungetc(c, source);
      *statex = state;
      return EOK;
    }
    /* Radkovy komentar */
    case S_LINECOMM: {
      if ( c != '\n' ) ;
      else state = S_START;

      break;
    }
    /* Blokovy komentar */
    case S_BLOCKCOMM: {
      if ( c == '*' ) state = S_BLOCKCOMM_BREAK;
      else if ( c == EOF ) {
        *statex = S_EOF;
        return lexError();
      }
      //pro znaky ruzne od * a EOF zustan ve svem stavu
      break;
    }

    case S_EQ_POSSIBLE: {
      if ( c != '=' ){
        ungetc(c, source);
        *statex = state;
        return E_LEX;
      }
      state = S_EQ;
      strCharAppend(tokenval, c);
      break;
    }

    case S_NEQ_POSSIBLE: {
      if ( c != '=' ){
        ungetc(c, source);
        *statex = state;
        return E_LEX;
      }
      state = S_NEQ;
      result = strCharAppend(tokenval, c);
      if (result != EOK) return result;
      break;
    }
    /*
     * -----------------------------------------
     * Uzly treti urovne
     * -----------------------------------------
     */

    case S_BLOCKCOMM_BREAK: {

      if ( c == '/' ) state = S_START; //uzavreni blokoveho komentare
      else if ( c == EOF ) return lexError();
      else state = S_BLOCKCOMM; // jiny je stale soucast komentare

      break;
    }

    case S_GEQ: {
      ungetc(c, source);
      *statex = state;
      return EOK;
    }
    case S_LEQ: {
      ungetc(c, source);
      *statex = state;
      return EOK;
    }

    case S_EQ: {
      ungetc(c, source);
      *statex = state;
      return EOK;
    }

    case S_NEQ: {
      ungetc(c, source);
      *statex = state;
      return EOK;
        }
    /*
     * -----------------------------------------
     * Dalsi uzly
     * -----------------------------------------
     */
    case S_REAL_DOT: {
      if ( isdigit(c)){
        state = S_REAL;
        result = strCharAppend(tokenval,c);
        if (result != EOK) return result;
      } else {
        ungetc(c, source);
        return E_LEX;
      }
      break;
    }

    case S_REAL: {
      if ( isdigit(c)){
        state = S_REAL;
        result = strCharAppend(tokenval,c);
        if (result != EOK) return result;
      } else if (c == 'e' || c == 'E') {
        state = S_REALEXP_E;
        result = strCharAppend(tokenval,c);
        if (result != EOK) return result;
      }
      else {
        ungetc(c, source);
        *statex = state;
        return EOK;
      }
      break;
    }

    case S_REALEXP: {
          if ( isdigit(c)){
            state = S_REALEXP;
            result = strCharAppend(tokenval,c);
            if (result != EOK) return result;
          }
          else {
            ungetc(c, source);
            *statex = state;
            return EOK;
          }
          break;
        }


    case S_REALEXP_E: {
      if ( isdigit(c)) {
        state = S_REALEXP;
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;

      } else if (c == '+' || c == '-'){
        state = S_EXPREAL_SIGN;
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;

      }else {
        ungetc(c, source);
        *statex = state;
        return E_LEX;
      }
      break;
    }

    case S_EXPREAL_SIGN: {
        if( isdigit(c) ) {
          state = S_REALEXP;
          result = strCharAppend(tokenval, c);
          if (result != EOK) return result;
        } else {
          ungetc(c, source);
          *statex = state;
          return E_LEX;
        }
        break;
    }

    case S_EOF: {
      *statex = state;
      strReset(tokenval);
      return EOK;
    }

    case S_STR_LOAD:{
      if ( c == '\\' ) {
        state = S_STR_ESC;
      }
      else if ( c == '"') {
        state = S_STRING;
      }
      else if ( isValidStrMember(c)){
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;
      } else {
        *statex = state;
        ungetc(c, source);
        return E_LEX;
      }
      break;
    }

    case S_STRING: {
      *statex = state;
      ungetc(c, source);
      return EOK;
    }

    case S_STR_ESC: { // nacetl zpetne lomitko
      //nasleduje za \ platny escape znak?
      if ( c == 'n' ) {
        result = strCharAppend(tokenval, '\n');
        if (result != EOK) return result;
      }
      else if (c == 't') {
        result = strCharAppend(tokenval, '\t');
        if (result != EOK) return result;
      }
      else if ( c == '\\' ) {
        result = strCharAppend(tokenval, '\\');
        if (result != EOK) return result;
      }
      else if ( c == '"' ) {
        result =  strCharAppend(tokenval, '"');
        if (result != EOK) return result;
      }
      else if (c == '$' ) {
        result = strCharAppend(tokenval, '$');
        if (result != EOK) return result;
      }
      else if (c == 'x') { //hexa escape sekvence
        state = S_STR_HEXA;
        break;
      } else if ( ! isValidStrMember(c) ){
        ungetc(c, source);
        return E_LEX;
      } else { //zbyle validni znaky
        result = strCharAppend(tokenval, '\\');
        result = strCharAppend(tokenval, c);
        if (result != EOK) return result;
      }

      state = S_STR_LOAD;

      break;
    }

    case S_STR_HEXA: {
      if ( isHexaChar(c) ){
        state = S_STR_HEXA_1;
        hexabuff[0] = c;
        break;
      }

      result=strCharAppend(tokenval, '\\');
      result=strCharAppend(tokenval, 'x');
      if (result != EOK) return result;
      ungetc(c, source);
      state = S_STR_LOAD;

      break;
    }

    case S_STR_HEXA_1: {
      if ( isHexaChar(c) ){
        state = S_STR_HEXA_2;
        hexabuff[1] = c;
        break;
      }

      result=strCharAppend(tokenval, '\\');
      result=strCharAppend(tokenval, 'x');
      result=strCharAppend(tokenval, hexabuff[0]);
      if (result != EOK) return result;
      ungetc(c, source);
      state = S_STR_LOAD;


      break;
    }

    case S_STR_HEXA_2: {

      int convbase = 16; //soustava, z jake prevadime

      char hex[5];
      hex[0] = '0';
      hex[1] = 'x';
      hex[2] = hexabuff[0];
      hex[3] = hexabuff[1];
      hex[4] = '\0';

      int res = strtol(hex, NULL, convbase);
      result=strCharAppend(tokenval, res);

      ungetc(c, source);
      state = S_STR_LOAD;

      break;
    }

    default:
      fprintf(stderr, "CHYBA VE SCANNERU! NEOSETRENA VARIANTA!\n");
      break;

    } // switch (state)
  }

  return c;

}


void printStateLabel(tState state) {
  const char * labels[] = {
    [S_START] = "START",
    [S_IDENT] = "IDENT",
    [S_VAR_IDENT] = "VAR-IDENT",
    [S_VAR_DOLLAR] = "-",
    [S_L_PARENTH]  = "L-PARENTH",
    [S_R_PARENTH]  = "R-PARENTH",
    [S_L_BRACE]  = "L-BRACE",
    [S_R_BRACE]  = "R-BRACE",
    [S_ASSIGN]   = "ASSIGN" ,
    [S_EQ_POSSIBLE] = "-",
    [S_NEQ_POSSIBLE] = "-",
    [S_EQ] = "EQ",
    [S_NEQ] = "NEQ",
    [S_ADD] = "ADD",
    [S_SUB] = "SUB",
    [S_MUL] = "MUL",
    [S_CONCAT] = "CONCAT",
    [S_DIV]  = "DIV",
    [S_LT]   = "LT",
    [S_LEQ]  = "LEQ",
    [S_GT] = "GT",
    [S_GEQ] = "GEQ",
    [S_DEC] = "DECIMAL",
    [S_REAL] = "REAL",
    [S_REAL_DOT] = "-",
    [S_REALEXP_E] = "-",
    [S_EXPREAL_SIGN] = "-",
    [S_LINECOMM] = "-",
    [S_BLOCKCOMM] = "-" ,
    [S_BLOCKCOMM_BREAK]  ="-",
    [S_SEMICOLON] = "SEMICOLON",
    [S_EXCLAMATION] =  "!",
    [S_EOF] = "EOF",
    [S_COMMA] = "COMMA",
    [S_REALEXP] = "REAL-EXP",
    [S_STR_LOAD] = "STR-LOAD",
    [S_STR_ESC] = "STR-ESC",
    [S_STRING] = "STRING",
    [S_STR_HEXA] = "STR-HEXA",
    [S_STR_HEXA_1] = "STR-HEXA-1",
    [S_STR_HEXA_2] = "STR-HEXA-2",

    /* Rezervovana slova */
    [KW_FUNC] = "KW-FUNC", // pri uprave ponechat jako prvni!
    [KW_IF] = "KW-IF",
    [KW_ELSE] = "KW-ELSE",
    [KW_RET] = "KW-RET",
    [KW_WHILE] = "KW-WHILE",
    [KW_FALSE] = "KW-FALSE",
    [KW_TRUE] = "KW-TRUE",
    [KW_NULL] = "KW-NULL", // pri uprave ponechat jako posledni!
  };

  fprintf(stderr,"%s", labels[state]);
}

/**
 * Funkce vytiskne vsechny zbyvajici tokeny az do konce radku.
 * Tiskne na standardni chybovy vystup.
 */
void printTillLF(){

  int c;

  while ( (c = fgetc(source)) != -1 && c != '\n' )
    fprintf(stderr, "%c", c);

  fprintf(stderr, "%c", '\n');
}
