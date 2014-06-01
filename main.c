/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : main.c
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

//importy vlastnich modulu
#include "garbage.h"
#include "str.h"
#include "error.h"
#include "scanner.h"
#include "ial.h"
#include "builtins.h"
#include "parser.h"
#include "ilist.h"
#include "interpret.h"

int main(int argc, char** argv) {
  if (argc != 2) { // ocekavame PRAVE JEDEN argument (+ nazev souboru)
    fprintf(stderr, "Nespravny pocet argumentu testovaciho programu.\n");
    return -1; //!! predelat na spravnou konstantu
  }

  FILE * f = fopen(argv[1], "r");
  if (f == NULL ) {
    fprintf(stderr, "Testovacimu souboru se nepodarilo otevrit soubor s aplikaci.\n");
    return internalError();
  }

  initGarbageCollector();

  //nastav soubor, ze ktereho scanner cte
  setSourceFile(f);

  htable_t* fT;
  tIlist iL;
  listInit(&iL);
  fT = htable_init(HTABLE_SIZE);

  htableStack *sS = gcMalloc(sizeof(htableStack));
  if (sS == NULL) return internalError();
  sS = htable_stack_init();

  htable_t *mainTable ;
  mainTable = htable_init(HTABLE_SIZE);
  htable_stack_push(sS,mainTable);

  int result = parse(fT, &iL,sS);

  if (result != EOK)
    {
      fclose(f);
      freeGarbage();
      return result;
      // jinak probehlo vse v poradku, muzeme provadet kod
    }


  result = (interpret(&iL,fT,sS));

  freeGarbage();
  fclose(f);

  if(result != EOK){
    return result;
    }


  return EOK;
}
