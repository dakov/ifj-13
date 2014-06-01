/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : expr.c - Modul ridici precedencni SA analyzu vyrazu
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
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "error.h"
#include "garbage.h"
#include "parser.h"
#include "expr.h"
#include "str.h"
#include "ilist.h"

extern tState tkState;
extern TString tkAttr;
extern int tkLine;

/**
 * Globalni tabulka funkci
 */
htable_t *funTable;

/**
 * Seznam instrukci
 */
tIlist *iList;

/**
 * Globalni zasobnik tabulek symbolu
 */
htableStack * symbolStack;

/**
 * Ciselnik pr ogenerovani unikatnich nazvu docasnych promennych
 */
int counterVar = 1; //pro generovani unikatnich promennych
TString *dest;
TExpr lastExpr; //pro rozpoznavani unarniho -
/* Velikost kroku alokace noveho mista pro zasobnik */
unsigned int STACK_SIZE = 16;


void stackPrint(TExprStack* s);

TExpr tokenToIndex(tState state);

TExpr stackTopTerminal(TExprStack *stack, int * pos);
int isTerminal(TExpr expr);
TExpr getTableSymbol(TExpr x, TExpr y);


void generateVariable(TString *var);
int tstateToType(tState state);



int stackPTLInsert(TExprStack *stack);
int stackPTRInsert(TExprStack *stack);
/**
 * Precencni tabulka, je rozdelena podle priority sestupne
 */

/**
 * Staticka precedencni tabulka
 */
int precedenceTab[16][16] = {
              /* PRI EDITACI TABULKY MUSI ZUSTAT ZAHLAVI SLOUPCU A RADKU SYMETRICE */
 /*              *    /    +    -    .    <    >   <=   >=   ===  !==   (    i    )    $    u- */
 /* 0: * */   { PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /* 1: / */   { PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /* 2: + */   { PTL, PTL, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /* 3: - */   { PTL, PTL, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /* 4: . */   { PTL, PTL, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /* 5: < */   { PTL, PTL, PTL, PTL, PTL, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /* 6: > */   { PTL, PTL, PTL, PTL, PTL, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /* 7:<= */   { PTL, PTL, PTL, PTL, PTL, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /* 8:>= */   { PTL, PTL, PTL, PTL, PTL, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /* 9:===*/   { PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /*10:!==*/   { PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
 /*11: ( */   { PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTE, PTX , PTL},
 /*12: i */   { PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTX, PTX, PTR, PTR , PTR},
 /*13: ) */   { PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTX, PTX, PTR, PTR , PTR},
 /*14: $ */   { PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTL, PTX, PTX , PTL},
 /*15:u- */   { PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTR, PTL, PTL, PTR, PTR , PTL},
};


/**
 * Provede precedencni syntaktickou analyzu vyrazu na zaklade precedencni tabulky
 * a vygeneruje funkcne ekvivalentni 3AC
 * @param fT Globalni tabulka funkci
 * @param iL Globalni seznam instrukci
 * @param sS Zasobnik tabulek symbolu
 */
int precSA(htable_t *fT, tIlist *iL,htableStack *sS){
  funTable = fT;
  iList = iL;
  symbolStack = sS;

  lastExpr = tokenToIndex(tkState);

  int result = EOK;
  TExprStack* stack = stackInit();

  if (stack == NULL) {
    return internalError();
  }

  TExprStack* tmpstack = stackInit();

  if (tmpstack == NULL) {
    return internalError();
  }


  result = stackPush(stack, PT_DELIM);    // dno zasobnik
  if (result != EOK ) return result;


  TExpr tkIndex, tkValTest; // vstupni token prevedeny na index do tabulky
  int topTermPos; // pozice prvniho terminalu u vrcholu



  do {

     tkIndex = tokenToIndex(tkState); //prevede typ token na index do prec. tabulky
     tkValTest = getTableSymbol(stackTopTerminal(stack, &topTermPos), tkIndex);


     if (tkIndex == EX_ERROR || tkValTest == PTX){ //pokud dostane spatny token nebo PTX, tak by mohl byt konec vyrazu
       //zkusi vyhodnotit vsechny pravidla na zasobniku
       while(stack->top != 1){
         result = stackPTRInsert(stack);

         if (result != EOK ) return result;
       }

       if(stackTopTerminal(stack, &topTermPos)==PT_DELIM){ //kdyz je tam < tak je to spravne
          TVar * tmpx = NULL;
          TString * stringx = NULL;
        if (stack->data[stack->top].type!= S_VAR_IDENT){ //posledni na stacku neni promenna
          //vytvorim pomocnou promennou
          tmpx = gcMalloc(sizeof(TVar));
          if (tmpx == NULL) return internalError();

          stringx = gcMalloc(sizeof(TString));
          if (stringx == NULL) return internalError();

          strinit(stringx);
          generateVariable(stringx);

          tmpx->type = tstateToType(stack->data[stack->top].type);
          tmpx->data = stack->data[stack->top].value;

          //nahraju do TS
          result = htable_insert(htable_stack_top(symbolStack),stringx,tmpx);
          if (result != EOK) return result;
        }
        else{ //jinak si jen vezmu nazev existujici promenne
          stringx = stack->data[stack->top].value.str;
        }

        ///unikatni promenna ve ktere jsou mezivysledky a vysledek
        TString * tmpDest = gcMalloc(sizeof(TString));
        if (tmpDest == NULL) return internalError();
        result = strinit(tmpDest);
        if (result != EOK) return result;

        generateVariable(tmpDest);
        dest = tmpDest;

        TVar * tmpvar = gcMalloc(sizeof(TVar));
        if(tmpvar == NULL)
          return internalError();

        result = htable_insert(htable_stack_top(symbolStack),dest,tmpvar);
        if (result != EOK) return result;

        // printf("assign %s %s\n",dest->string,stringx->string);
         result = igenerate(iList,I_ASSIGN,dest,stringx,NULL);
         if (result != EOK) return result;
       //  printIList(iList);
         return EOK;
       }
       else
        return syntaxError();
     }

     switch(getTableSymbol(stackTopTerminal(stack, &topTermPos), tkIndex)) {
     case PTE: // =
       result = stackPush(stack, tkIndex);
       if (result != EOK ) return result;

       lastExpr = tkIndex;
       result = getToken(&tkAttr, &tkState, &tkLine);
       if (result != EOK ) return lexError();
       break;

     case PTL: // <
       //nahrad a za a<
       result = stackPTLInsert(stack);
       if (result != EOK ) return result;


       result = stackPush(stack, tkIndex);
       if (result != EOK ) return result;

       lastExpr = tokenToIndex(tkState);
       result = getToken(&tkAttr, &tkState, &tkLine);
       if (result != EOK ) return lexError();

       tkIndex = tkIndex;
       break;

     case PTR: // >
       result = stackPTRInsert(stack);
       if (result != EOK) return result;

       break;

     ///nikdy se sem nedostane, bere se jako konec vyrazu
     case PTX:{ // prazdne misto v tabulce
       //TExpr top = stackTopTerminal(stack, &topTermPos);

       //nejvrchnejsi je operand -> redukuj i na E (r: E -> i)


       return syntaxError();
       break;
     }
     default:
       return syntaxError();
     }
    //break;

  } while( stackTopTerminal(stack, &topTermPos) != PT_DELIM || tkIndex != PT_DELIM);


  return EOK;

}





/* ----------------------------------------------------------- */
/*                         ZASOBNIK                            */
/* ----------------------------------------------------------- */

/** Alokace a inicializace prázdného zásobníku
 * Příklad použití:
 * TExprStack* stack = StackInit();
 *
 * (Všechny následující operace se zásobníkem
 *  jsou automaticky prázdné operace, pokud
 *  není zásobník správně inicializovaný)
 *
 * @return Odkaz na prázný alokovaný zásobník
 */
TExprStack* stackInit()
{
  TExprStack* s = gcMalloc(sizeof(TExprStack));

  if ( s == NULL ) return NULL;

  s->top = -1;
  s->allocated = STACK_SIZE;
  s->data = gcMalloc(sizeof(TStackItem) * s->allocated);

  if ( s->data == NULL ) return NULL;

  return s;
}

/** Zjistí, zda-li je daný zásobník prázný
 *
 * @param s Odkaz na zásobník
 * @return Nula pokud není, nenulová hodnota pokud je
 */
int stackEmpty(TExprStack* s)
{
  if (s->top == -1 || s->data == NULL)
    return 1;
  return 0;
}

/** Získání hodnoty na vrcholu zásobníku
 * (Bez jeho odebrání; pouze přečtení)
 * (Pokud je zásobník prázdný, bude vráceno EX_ERROR)
 *
 * @param s Odkaz na zásobník
 * @return Prvek enum TExpr - vrchol zásobníku | EX_ERROR
 */
TExpr stackTop(TExprStack* s)
{
  if (stackEmpty(s))
    return EX_ERROR;
  return s->data[s->top].data;
}

/** Získání hodnoty na vrcholu zásobníku
 * (Bez jeho odebrání; pouze přečtení)
 * (Pokud je zásobník prázdný, bude vráceno EX_ERROR)
 *
 * @param s Odkaz na zásobník
 * @return Prvek enum TStackItem - vrchol zásobníku | EX_ERROR
 */
TStackItem stackTopItem(TExprStack* s){
  //if (stackEmpty(s))
    //return EX_ERROR;
  return s->data[s->top];
}

/** Odstranení prvku z vrcholu zásobníku
 * (bez jeho přečtení, pouze odebrání)
 * (je-li zásobník prázný, prázdná operace)
 *
 * @param s Odkaz na zásobník
 */
void stackPop(TExprStack* s)
{
  if (stackEmpty(s))
    return;
  s->top -= 1;
}

/** Vložení prvku enum TExpr na vrchol zásobníku
 *
 * @param s Odkaz na zásobník
 * @param d Data ke vložení - enum TExpr
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit zasobnik
 */
int stackPush(TExprStack* s, TExpr d)
{

  if (++s->top == s->allocated) {
    s->allocated += STACK_SIZE;
    s->data = gcRealloc(s->data, sizeof(TStackItem)*(s->allocated));
    if ( s->data == NULL ) return internalError();
  }

  s->data[s->top].data = d;

  //zapamatovani si hodnoty
  if(d == PT_I){
    s->data[s->top].type = tkState;
    if(tkState == S_DEC){
      s->data[s->top].value.integer = atoi(tkAttr.string);
    }
    else if(tkState == S_REAL || tkState == S_REALEXP){
      s->data[s->top].value.real = strtod(tkAttr.string,NULL);
    }
    else if(tkState == S_STRING){
      s->data[s->top].value.str = gcMalloc(sizeof(TString));
      strinit(s->data[s->top].value.str);
      strTStringCpy(s->data[s->top].value.str,&tkAttr);
    //  printf("%s\n",s->data[s->top].value.str->string);
    }
    else if(tkState == KW_TRUE){
      s->data[s->top].value.boolean = true;
    }
    else if(tkState == KW_FALSE){
      s->data[s->top].value.boolean = false;
    }
    else if(tkState==KW_NULL){
      s->data[s->top].value.null=NULL;
    }
    else if(tkState==S_VAR_IDENT){
      s->data[s->top].value.str = gcMalloc(sizeof(TString));
      strinit(s->data[s->top].value.str);
      strTStringCpy(s->data[s->top].value.str,&tkAttr);
     // printf("%s\n",s->data[s->top].value.str->string);
    }
  }
  return EOK;
}

/** Vložení prvku TStackItem na vrchol zásobníku
 *
 * @param s Odkaz na zásobník
 * @param d Data ke vložení - TStackItem
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit zasobnik
 */
int stackPushItem(TExprStack* s, TStackItem d){
  ++s->top;
  s->data[s->top]=d;
  return EOK;
}

/**
 * Prvni terminal na zasobniku se vyskytuje bud na tom nebo top-1
 * @param stack Zasobnik precedencni analyzy
 * @return Odpovidajici navratovou konstantu viz. zadani
 */
int stackPTLInsert(TExprStack *stack){
  int result;
  if (isTerminal(stackTop(stack))){
    result = stackPush(stack, PTL);
  } else {
    TStackItem tmp = stackTopItem(stack);
    //TExpr tmp = stackTop(stack);
    stack->top -= 1;
    result = stackPush(stack, PTL);
    if (result != EOK ) return result;
    //result = stackPush(stack, tmp);
    result = stackPushItem(stack,tmp);
    if (result != EOK ) return result;
  }
  return EOK;
}

/**
 * Vyhodnocovani pravidel
 * @param stack Zasobnik precedencni analyzy
 * @return EOK | E_SYNTAX |
 */
int stackPTRInsert(TExprStack *stack){
  int result;
  switch(stackTop(stack)){
    case PT_I:{ // E-> <i
      TStackItem tmp=stackTopItem(stack);
      stackPop(stack); //<
      if(stackTop(stack)==PTL){
        stackPop(stack);//
        stackPushItem(stack,tmp);// E-> <i
        stack->data[stack->top].data=PT_E; //i na E
        return EOK;
      }
      else
        return syntaxError();
    }

    case PT_E:{ //E-> <ExE
      TStackItem tmp1=stackTopItem(stack);
      stackPop(stack);
      switch(stackTop(stack)){
      case PT_USUB:{ //-E
        stackPop(stack); // oddela se u-
        stackPop(stack); // oddela se <

        result = doTheMath(&tmp1,NULL,PT_USUB);
        if (result != EOK) return result;
        stackPushItem(stack,tmp1);

        return EOK;
      }
        case PT_MUL: //<E*E
        case PT_DIV: //<E/E
        case PT_ADD: //<E+E
        case PT_SUB: //<E-E
        case PT_CONCAT: //<E.E
        case PT_LT: //<E<E
        case PT_GT: //<E>E
        case PT_LEQ: //<E<=E
        case PT_GEQ: //<E>=E
        case PT_EQ: //<E===E
        case PT_NEQ:{ //<E!==E
          TExpr tmpOp = stackTop(stack);
          stackPop(stack);
          if(stackTop(stack) == PT_E){ //<E znaminko E
            TStackItem tmp2 = stackTopItem(stack);
            stackPop(stack);
            if(stackTop(stack) == PTL){ //spravne <EznamenkoE{
              stackPop(stack); //oddela se <

              result = doTheMath(&tmp2,&tmp1,tmpOp); ///na zasobniku sou naopak
              if (result != EOK) return result;
              stackPushItem(stack,tmp2);//E->EznamenkoE

              return EOK;
            }
            else
              return syntaxError();
          }
          else
            return syntaxError();
        }

        default :return syntaxError();
      }
      break;
    }

    case PT_RPARENTH:{ //E-> <(E)
     // stackPrint(stack);
      TStackItem tmp;
      stackPop(stack); // <(E
      if(stackTop(stack) == PT_E){
        tmp = stackTopItem(stack);
        stackPop(stack); //<(
      }
      else
        return syntaxError();
      if(stackTop(stack) == PT_LPARENTH){
        stackPop(stack); //<
      }
      else
        return syntaxError();
      if(stackTop(stack) == PTL){
        stackPop(stack);  //konec
        stackPushItem(stack,tmp);   //E->(E)
      }
      else
        return syntaxError();
      return EOK;
    }



    default: return syntaxError();
  }
}

/**
 * Nalezne a vrati hodnotu nejvrchnejsiho terminalu na zasobniku
 * @param stack zasobnik precedencni analyzy
 * @param pos Vystupni parametr, vraci na kolikate pozici se nachazi prvni terminal
 * @return Datova slozka prvniho terminalu
 */
TExpr stackTopTerminal(TExprStack *stack, int * pos){
    int ntop =  stack->top;

    // dokud jsme nenasli terminal
    while ( ! isTerminal(stack->data[ntop].data ) )
      ntop--;

    *pos = ntop;

    return stack->data[ntop].data;
}

/**
 * Zjisti, zda je zkoumany prvek terminalem
 * @param expr Typ zkoumaneho vyrazu
 * @return True | False
 */
int isTerminal(TExpr expr) {
  return expr <= PT_USUB;
}

/**
 * Vraci hodnotu z precedencni tabulky na pozici x, y
 * @param x Index radku ( zasobnik )
 * @param y Index sloupce ( vstup )
 * @return Hodnotu z tabulky symbolu (typ TExpr)
 */
TExpr getTableSymbol(TExpr x, TExpr y) {
  return precedenceTab[x][y];
}

/**
 * Prevede typ tokenu na korespondujici identifikator prvku ve vyrazu
 * @param state Zkoumany typ tokenu
 * @return Odpovidaji hodnota do precedencni tabulky
 */
TExpr tokenToIndex(tState state) {
  switch (state) {
  case S_MUL: return PT_MUL;
  case S_DIV: return PT_DIV;
  case S_ADD: return PT_ADD;
  case S_CONCAT: return PT_CONCAT;
  case S_LT: return PT_LT;
  case S_GT: return PT_GT;
  case S_LEQ: return PT_LEQ;
  case S_GEQ: return PT_GEQ;
  case S_EQ: return PT_EQ;
  case S_NEQ: return PT_NEQ;
  case S_L_PARENTH: return PT_LPARENTH;
  case S_STRING:
  case S_REAL:
  case S_REALEXP:
  case KW_FALSE:
  case KW_TRUE:
  case KW_NULL:
  case S_VAR_IDENT:
  case S_DEC: return PT_I;
  case S_R_PARENTH: return PT_RPARENTH;
  case S_SUB:

    if (lastExpr == PT_I || lastExpr == PT_E || lastExpr == PT_RPARENTH){

      return PT_SUB;
    }
    else{

      return PT_USUB;
    }




  default: return EX_ERROR;
  }
}

/**
 * Na zaklade typu tokenu zjisti, jakemu datovemu typu
 * jazyka ifj13 dany lexem odpovida
 *
 * @param state Zkoumany stav tokenu
 * @return Odpovidajici datovy typ jazyka ifj13 nebo konstantu EX_ERROR
 */
int tstateToType(tState state){
  switch(state){
    case S_DEC: return T_INT;
    case S_REAL:
    case S_REALEXP: return T_REAL;
    case S_STRING: return T_STR;
    case KW_TRUE:
    case KW_FALSE: return T_BOOL;
    case KW_NULL: return T_NULL;

    default: return EX_ERROR;
  }

}

/**
 * Vytisne zasobni 's' na stdin
 * @param s Tisknuty zasobnik
 */
void stackPrint(TExprStack* s)
{
  int tmp = s->top;

  while(s->top != -1) {

    printf("%d", stackTop(s));
    if (s->top == tmp ) printf( " <- top ");
    if (s->top == 0 ) printf( " <- bottom ");
    printf("\n");

    stackPop(s);
  }

  //vrati ukazatel zpatky na puvodni vrchol
  s->top = tmp;
}

/** Provedeni operace mezi dvemi E
 *
 * @param x  prvni operand a zaroveni destinatinou
 * @param y  druhy operand
 * @param op  operator
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit zasobnik
 */
int doTheMath(TStackItem *x, TStackItem *y, TExpr op){
  TVar * tmpx = NULL;
  TVar * tmpy = NULL;
  TString * stringx = NULL;
  TString * stringy = NULL;
  int result;


  if (x->type != S_VAR_IDENT){ //vsechno krome promennych dostane svoji promennou
    tmpx = gcMalloc(sizeof(TVar));
    if (tmpx == NULL) return internalError();

    stringx = gcMalloc(sizeof(TString));
    if (stringx == NULL) return internalError();

    strinit(stringx);
    generateVariable(stringx);

    tmpx->type = tstateToType(x->type);
    tmpx->data = x->value;
  }
  else{
    //naplnim stringx nazvem promenne x
    stringx = x->value.str;
  }

  //kvuli unarnimu minus
  if (y != NULL){
    if (y->type != S_VAR_IDENT){ //vsechno krome promennych dostane svoji promennou
      tmpy = gcMalloc(sizeof(TVar));
      if (tmpy == NULL) return internalError();

      stringy = gcMalloc(sizeof(TString));
      if (stringy == NULL) return internalError();

      strinit(stringy);
      generateVariable(stringy);

      tmpy->type = tstateToType(y->type);
      tmpy->data = y->value;
    }
    else{
      //naplnim stringy nazvem promenne y
      stringy = y->value.str;
    }
  }


  if (tmpx != NULL){ //vlozim pomocnou promennou do tabulky
    result = htable_insert(htable_stack_top(symbolStack),stringx,tmpx);
    if (result != EOK) return result;
  }
  if (tmpy != NULL){
  result = htable_insert(htable_stack_top(symbolStack),stringy,tmpy);
  if (result != EOK) return result;
  }

  //vysledek se vrati v x
  if (x->type != S_VAR_IDENT || x->type != S_STRING){ //jeste nema namallocovano
    x->value.str = gcMalloc(sizeof(TString));
    strinit(x->value.str);
  }


  ///unikatni promenna ve ktere jsou mezivysledky a vysledek
  TString * tmpDest = gcMalloc(sizeof(TString));
  if (tmpDest == NULL) return internalError();
  result = strinit(tmpDest);
  if (result != EOK) return result;

  generateVariable(tmpDest);
  dest = tmpDest;

  TVar * tmpvar = gcMalloc(sizeof(TVar));   //prifani 25.11.2013 19:56:53
  if(tmpvar == NULL)
    return internalError();
  result = htable_insert(htable_stack_top(symbolStack),dest,tmpvar);
  if (result != EOK) return result;

  //promenna do ktere se ulozi mezivypocet a ktera se vrati nazpet
  x->type = S_VAR_IDENT; //vraci se to v x
  x->value.str=dest;



  ///instrukce podle operandu

  switch(op){
    case PT_ADD: //+
     // printf("add %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_ADD,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_SUB: //-
     // printf("sub %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_SUB,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_MUL: //*
     // printf("mul %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_MUL,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_DIV: // /
     // printf("div %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_DIV,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_CONCAT: // .
    //  printf("concat %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_CONCAT,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_LT: // <
     // printf("LT %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_LE,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_GT: // >
    //  printf("LT %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_GE,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_LEQ: // <=
     // printf("LEQ %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_LEQ,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_GEQ: // >=
    //  printf("GEQ %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_GEQ,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_EQ: // ===
    //  printf("EQ %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_EQ,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_NEQ: // !==
    //  printf("NEQ %s %s %s\n",dest->string,stringx->string,stringy->string);
      result = igenerate(iList,I_NEQ,dest,stringx,stringy); //zmenit hodnoty na htablefind
      if (result != EOK) return result;
      return EOK;

    case PT_USUB:
      result = igenerate(iList,I_NEG,dest,stringx,NULL);
      if (result != EOK) return result;
      return EOK;


    default: return EX_ERROR;
  }
}

/** generuje jedinecne nazvy identifikatoru
 *  nazev se sklada ze znaku # nasledovanym cislem
 * postupne se tu generuji prirozena cisla a do nazvu promenne se ukladaji
 * v reverzovanem poradi - na funkcnost to nema vliv, ale je jednodussi implementace
 *
 * @param var TString do ktereho se ulozi
*/
void generateVariable(TString *var)
{
  strReset(var);
  strCharAppend(var,'#');
  int i;
  i = counterVar;
  while (i != 0)
  {
    strCharAppend(var, (char)(i % 10 + '0'));
    i = i / 10;
  }
  counterVar ++;
}

/**
 * Prevede literlni hodnotu na ekvivalentni strukturu odpovidajici typu
 * s danou hodnotou (aby byly kompatibilni s typem promenne)
 * @param state Stav tokenu, kterym byla konstanta nactena
 * @para val Hodnota tokenu
 *
 * @return Ukazatel na odpovidajici
 */
TVar * constantToVar(tState * state,TString * val){
  int result;
  TVar * tmpVar = gcMalloc(sizeof(TVar));
  if (tmpVar == NULL) return NULL;

  tmpVar->type = tstateToType(*state);

  result = tokenToData(&(tmpVar->data),state,val);
  if (result != EOK) return NULL;

  return tmpVar;
}

/**
 * Na zaklade datove slozky tokenu inicializuje odpovidajici typ a hodnotu
 * nove vznikajici konstantni promenne
 *
 * @param data
 * @param state Stav tokenu
 * @param string
 * @return
 */
int tokenToData(TData *data,tState * state,TString * string){
  int result = EOK;
  if (*state == S_DEC){
    data->integer = atoi(string->string);
  }
  else if(*state == S_REAL || *state == S_REALEXP){
    data->real = strtod(string->string,NULL);
  }
  else if(*state == S_STRING){
    data->str = gcMalloc(sizeof(TString));
    if (data->str == NULL) return internalError();
    result = strinit(data->str);
    if (result != EOK) return result;
    result = strTStringCpy(data->str,string);
    if (result != EOK) return result;
  }
  else if(*state == KW_TRUE){
    data->boolean = true;
  }
  else if(*state == KW_FALSE){
    data->boolean = false;
  }
  else if(*state == KW_NULL){
    data->null = NULL;
  }
  else if(*state == S_VAR_IDENT){
    data->str = gcMalloc(sizeof(TString));
    if (data->str == NULL) return internalError();
    result = strinit(data->str);
    if (result != EOK) return result;
    result = strTStringCpy(data->str,string);
    if (result != EOK) return result;
  }
  return EOK;
}

int generateTrue(){
  TVar * tmp = gcMalloc(sizeof(TVar));
  if (tmp == NULL) return internalError();

  tmp->type = T_BOOL;
  tmp->data.boolean = true;

  TString *tmpstr = gcMalloc(sizeof(TString));
  if (tmpstr == NULL) return internalError();

  int result = strinit(tmpstr);
  if (result != EOK) return result;

  generateVariable(tmpstr);
  result = htable_insert(htable_stack_top(symbolStack),tmpstr,tmp);
  if (result != EOK) return result;

  result = igenerate(iList,I_ASSIGN,tmpstr,tmpstr,NULL);
  if (result != EOK) return result;
  return EOK;
}

int generateFalse(){
  TVar * tmp = gcMalloc(sizeof(TVar));
  if (tmp == NULL) return internalError();

  tmp->type = T_BOOL;
  tmp->data.boolean = false;

  TString *tmpstr = gcMalloc(sizeof(TString));
  if (tmpstr == NULL) return internalError();

  int result = strinit(tmpstr);
  if (result != EOK) return result;

  generateVariable(tmpstr);

  result = htable_insert(symbolStack->tables[0],tmpstr,tmp);
  if (result != EOK) return result;

  result = igenerate(iList,I_ASSIGN,tmpstr,tmpstr,NULL);
  if (result != EOK) return result;

  return EOK;
}

