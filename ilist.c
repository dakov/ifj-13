/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : ilist.h - Modul pro generovani a uchovavani 3AC
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral
 ============================================================================
 */

#include <stdio.h>
#include <malloc.h>
#include "ilist.h"
#include "error.h"
#include "garbage.h"

void listInit(tIlist *L) {
  L->first  = NULL;
  L->last   = NULL;
  L->active = NULL;
}


int listInsertLast(tIlist *L, tInstr i) {
  tListItem *newItem;
  newItem = gcMalloc(sizeof (tListItem));

  if(newItem == NULL) return internalError();

  newItem->instr = i;
  newItem->nextItem = NULL;
  if (L->first == NULL)
     L->first = newItem;
  else
     L->last->nextItem=newItem;
  L->last=newItem;

  return EOK;
}

int listInsertLastItem(tIlist *L, tListItem *i) {
  if (L->first == NULL)
     L->first = i;
  else
     L->last->nextItem=i;
  L->last=i;

  return EOK;
}

void listFirst(tIlist *L) {
  L->active = L->first;
}


void listNext(tIlist *L) {
  if (L->active != NULL)
  L->active = L->active->nextItem;
}

/**
 * Vnuti seznamu aktivni polozku
 * @param L Pracovni seznam
 * @parama gotoInstr Ukazatel na polozku, ktera bude aktivni
 */
void listGoto(tIlist *L, void *gotoInstr) {
  L->active = (tListItem*) gotoInstr;
}

/**
 * Vraci ukazatel na posledni polozku seznamu
 * @param L Pracovni seznam
 */
void *listGetPointerLast(tIlist *L) {
  return (void*) L->last;
}

/**
 * @param L Pracovni seznam
 * @return Ukazatel na aktivni polozku ( aktualni instrukci), NULL pokud L neni aktivni
 */
tInstr *listGetData(tIlist *L) {

  if (L->active == NULL)
  {
    return NULL;
  }
  else return &(L->active->instr);
}

void printInstr(tInstr *i){
  tInstrType itype = i->type;

  switch(itype){
  case I_ADD: printf("I_ADD");break;
  case I_SUB: printf("I_SUB");break;
  case I_MUL: printf("I_MULL");break;
  case I_DIV: printf("I_DIV");break;
  case I_CONCAT: printf("I_CONCAT");break;
  case I_EQ: printf("I_EQ");break;
  case I_NEQ: printf("I_NEQ");break;
  case I_LE: printf("I_LE");break;
  case I_LEQ: printf("I_LEQ");break;
  case I_GE: printf("I_GE");break;
  case I_GEQ: printf("I_GEQ");break;
  case I_NOT: printf("I_NOT");break;
  case I_ASSIGN: printf("I_ASSIGN");break;
  case I_JMP: printf("I_JMP");break;
  case I_IFJUMP: printf("I_IFJUMP");break;
  case I_RET: printf("I_RET");break;
  case I_CALL: printf("I_CALL");break;
  case I_LABEL: printf("I_LABEL");break;
  case I_PUSHPAR: printf("I_PUSHPAR");break;
  case I_POPPAR: printf("I_POPPAR");break;
  case I_NOP: printf("I_NOP"); break;
  case I_STOP: printf("I_STOP"); break;

  case I_FSKIP: printf("I_FSKIP");break;
  case I_FSTOP: printf("I_FSTOP"); break;
  case I_BOOLVAL: printf("I_BOOLVAL"); break;
  case I_DOUBLEVAL: printf("I_DOUBLEVAL"); break;
  case I_INTVAL: printf("I_INTVAL"); break;
  case I_STRVAL: printf("I_STRVAL"); break;
  case I_GETSTR: printf("I_GETSTR"); break;
  case I_PUTSTR: printf("I_PUTSTR"); break;
  case I_STRLEN: printf("I_STRLEN"); break;
  case I_GET_SUBSTR: printf("I_GET_SUBSTR"); break;
  case I_FIND_STR: printf("I_FIND_STR"); break;
  case I_SORT_STR: printf("I_SORT_STR"); break;
  case I_NEG: printf("I_NEG"); break;

  }

  // tisk operandu
  printf(" (%p,%p,%p)\n", i->op1, i->op2, i->op3);
}


void printIList(tIlist * L){

  listFirst(L);

  while (L->active != NULL){
    printf("%p: ", (void *) L->active);
    printInstr(&L->active->instr);

    listNext(L);
  }

}

int igenerate(tIlist *L,tInstrType type, void * op1, void * op2, void * op3){
  int result = EOK;

  tInstr i = {type,op1,op2,op3};

  result = listInsertLast(L, i);

  return result;
}



void * igenerateNoAppend(tInstrType type, void * op1, void * op2, void * op3){

  tInstr * i = gcMalloc(sizeof(tInstr));
  if( i == NULL ) return NULL;
   

  i->type = type;
  i->op1 = op1;
  i->op2 = op2;
  i->op3 = op3;

  tListItem * item = gcMalloc(sizeof(tListItem));
  if( item == NULL ) return NULL; 
   
  item->instr = *i;
  item->nextItem = NULL;

  return item;
}

