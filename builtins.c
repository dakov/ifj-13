/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Modul       : builtins.c
             : Modul obsahuje implementaci vestaveny stuktur a funkci,
             : vyjma tech z formalnich duvodu ulozeny v souboru ial.c

 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral
 ============================================================================
 */
#include "builtins.h"
#include "scanner.h"


/**
 * Zkontroluje, zda rezec str lze (případně jak) prevest na realne cislo.
 * @param vstupni retezec
 * @return 1 pokud na zacatku je platne cislo, 0 - pokud se ma retezec preves jako 0.0, -1, pokud doslo k chybe
 */
int checkForDouble(char *str){
  int state = S_START;
  int c; //aktualni zank
  int len = strlen(str);

  for(int i=0; i <= len; ++i){
    c = str[i];

    switch (state) {
    case S_START:
      if (isspace(c)) continue;
      else if (isdigit(c)) state = S_DEC;
      else return 0;

      break;

    case S_DEC:
      if(isdigit(c)) continue;
      else if (c == '.') state = S_REAL_DOT;
      else if (c == 'e' || c == 'E') state = S_REALEXP_E;
      else return 1; // je koncovy stav

      break;
    case S_REAL_DOT:
      if (isdigit(c)) state = S_REAL;
      else return -1;
      break;
    case S_REAL:
      if (isdigit(c)) continue;
      else if (c == 'e' || c == 'E') state = S_REALEXP_E;
      else return 1; // je koncovy
      break;
    case S_REALEXP_E:
      if (isdigit(c)) state = S_REALEXP;
      else if( c == '+' || c == '-') state = S_EXPREAL_SIGN;
      else return -1; // neni koncovy
      break;
    case S_EXPREAL_SIGN:
      if (isdigit(c)) state = S_REALEXP;
      else return -1; //neni koncovy
      break;
    case S_REALEXP:
      if  (isdigit(c)) continue;
      else return 1;
      break;

    default:
        break;
    }

    //pocatecni stav

  }

  return 1;
}

/**
 * Zkontroluje, zda rezec str lze (případně jak) prevest na cele cislo.
 * @param vstupni retezec
 * @return 1 pokud na zacatku je platne cislo, 0 - pokud se ma retezec preves jako 0, -1, pokud doslo k chybe
 */
int checkForInt(char *str){
  int state = S_START;
  int c; //aktualni zank
  int len = strlen(str);

  for(int i=0; i <= len; ++i){
    c = str[i];

    switch (state) {
    case S_START:
      if (isspace(c)) continue;
      else if (isdigit(c)) state = S_DEC;
      else return 0;

      break;

    case S_DEC:
      if(isdigit(c)) continue;
      else return 1; // je koncovy stav
      break; //je koncovy

    default:
        break;
    }

    //pocatecni stav

  }

  return 1;
}

TVar * boolval(TVar *var, int * result)
{
    TVar * retvar = gcMalloc(sizeof(TVar));
    if(retvar == NULL){
      *result = internalError();
      return NULL;
    }
    retvar->type = T_BOOL;

    switch(var->type){
      case T_NULL:
        retvar->data.boolean = false;
        break;
      case T_INT:
        retvar->data.boolean = (bool)var->data.integer;
        break;
      case T_REAL:
        retvar->data.boolean =  (bool)var->data.real;
        break;
      case T_STR:
        retvar->data.boolean = ((strConstTStringCmp(var->data.str,"")) ? true : false);
        break;
      case T_BOOL:
        retvar->data.boolean =  var->data.boolean;
        break;
      default:
        retvar->data.boolean = false;

    }
    return retvar;
}


TVar * intval(TVar *var, int * result){
    TVar * retvar = gcMalloc(sizeof(TVar));
    if(retvar == NULL){
      *result = internalError();
      return NULL;
    }
    retvar->type = T_INT;


    switch(var->type){
      case T_NULL:
        retvar->data.integer = 0;
        break;
      case T_BOOL:
        if(!var->data.boolean)
          retvar->data.integer = 0;
        else
          retvar->data.integer = 1;
        break;
      case T_INT:
        retvar->data.integer = var->data.integer;
        break;
      case T_REAL:
        retvar->data.integer = (int)var->data.real;
        break;
      case T_STR:{
        int number;
        char * pEnd;
        int test;

        test = checkForInt(var->data.str->string);

        if (test == 1){
          number = strtol(var->data.str->string,&pEnd,10);//jak se chovat kdyz presahne maximalni velikost int?
          retvar->data.integer = number;
          *result = EOK;
        }
        else if (test == 0){
          retvar->data.integer = 0;
          *result = EOK;
        }
        break;
      }
      default:
        retvar->data.integer = 0;
    }
    return retvar;
}

TVar * strval(TVar *var, int * result){
    TVar * retvar = gcMalloc(sizeof(TVar));
    if(retvar == NULL){
      *result = internalError();
      return NULL;
    }
    retvar->type = T_STR;

    TString * str = gcMalloc(sizeof(TString));
    if(str == NULL){
      internalError();
      *result = internalError();
      return NULL;
    }
    strinit(str);

    switch(var->type){
      case T_NULL:
        break;
      case T_INT:
        sprintf(str->string, "%d", var->data.integer);       //uz mame alokovano 16 - staci pro int
        break;
      case T_REAL:
        sprintf(str->string, "%g", var->data.real);
        break;
      case T_STR:
        strTStringCpy(str, var->data.str);
        break;
      case T_BOOL:
        if(var->data.boolean)
          strCharAppend(str,'1');
        break;
      default:
        break;
    }

    retvar->data.str = str;
    retvar->data.str->length = strlen(str->string);
    return retvar;

}

TVar * doubleval(TVar *var, int * result){
  TVar * retvar = gcMalloc(sizeof(TVar));
  if(retvar == NULL){
    *result = internalError();
    return NULL;
  }

  retvar->type = T_REAL;

  switch(var->type){
    case T_NULL:
      retvar->data.real = 0.0;
      break;
    case T_BOOL:
      if(!var->data.boolean)
        retvar->data.real = 0.0;
      else
        retvar->data.real = 1.0;
      break;
    case T_INT:
      retvar->data.real = (double)var->data.integer;
      break;
    case T_REAL:
      retvar->data.real = var->data.real;
      break;
    case T_STR:{

      int test = checkForDouble(var->data.str->string);
      if (test == 1){
        char * pEnd;
        retvar->data.real = strtod(var->data.str->string,&pEnd);
        *result = EOK;
      }
      else if (test == 0){
        retvar->data.real = 0.0;
        *result = EOK;
      }
      else if (test == -1){
        *result = castError();
         return NULL;
      }


      break;
    }
    default:
      retvar->data.real = 0.0;
      break;
  }
  return retvar;
}




