/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13

 Soubor      : ial.c - Modul obsahujici vestavene struktury a funkce predmetu IAL
             : Soubor obsahuje i veci, ktere se primo nevztahuji k premetu IAL, z toho
             : duvodu je implementace tabulky symbolu, vyhledavaciho a radiciho algoritmu
             : vytknuta na zacatek souborug

 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral
 ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "ial.h"
#include "error.h"

/* Velikost kroku alokace noveho mista pro zasobnik */
extern unsigned int STACK_SIZE;

bool less(char first, char second);
void swap(char* input, int i, int j);

void list_init(htable_t * t, unsigned int index);
void list_insert_first(htable_list * list, htable_listitem* new_item);


/*
 * -----------------------------------------------------------------------------
 *                       IMPLEMENTACE ADT a ALGORITMU PRO PREDMET IAL
 * -----------------------------------------------------------------------------
 */

  /* -----------------------  Hashovaci tabulka ----------------------------- */

  /* --------------------------  Shell sort  -------------------------------- */

/**
 * Funkce pro serazeni retezce dle ordinalni hodnoty znaku
 *
 * Pro sorting je pouzit algoritmus Shellsort
 *
 * @para TVar dest string, kterej se alokuje
 * @param TVar obsahujici retezec, ktery se bude radit
 * @return errcode
 */
int sort_string(TVar* dest, TVar * str)
{

    dest->data.str = gcMalloc(sizeof(TString));
    if(dest->data.str == NULL)
      return internalError();

    int result = strinit(dest->data.str);
    if(result != EOK) return result;

    result = strTStringCpy(dest->data.str, str->data.str);
    if(result != EOK) return result;

    TString * tmpstr = dest->data.str;


//     if(str == NULL)          //neni potreba, osetruje se v interpretu
//         return internalError();
//     if(str->string == NULL)
//         return internalError();

    int h = 1;  //prirustek

    while(h <   tmpstr->length/3)
        h = 3*h+1;  //Knuthova rovnice pro vyjadreni prirustku

    while(h >= 1)  //dokud pracujeme s kladnym prirustkem
    {
        for(int i=h; i<tmpstr->length; i++)  //retezcem se prochazi sekvencne
        {
            //zde se musime vyporadat s ukoncenim retezce. Nechat jej na konci!
            if(tmpstr->string[i] == 0)
                continue;

            //dokud 'j' je vetsi nez prirustek a hodnoty nejsou serazene
            for(int j=i; j >= h && less(tmpstr->string[j], tmpstr->string[j-h]); j -= h)
                swap(tmpstr->string, j, j-h);
        }
        h = h/3;  //snizeni prirustku == snizeni skoku mezi porovnavanymi znaky
    }
    return EOK;
}

  /* -----------------  Knuth-Morris-Prattuv algoritmus --------------------- */

/**
 * Vyhleda prvni vyskyt podretezce needle v retezci haystack a vrati
 * jeho pozici (od nuly). Prazdny retezec se vyskytuje v kazdem
 * retezci na pozici 0.
 *
 * Vyhledavani je provádí Knuth-Moris-Prattuv algoritmus. (opora IAL str 171)
 *
 * @param haystack Retezec, ve kterem se vyhledava
 * @param needle Hledany prodretezec.
 * @param position misto kde se to naslo/-1
 *
 * @return Errcode
 */

int find_string(TString * haystack, TString *needle, int * position) {



  if (haystack == NULL || needle == NULL)
    return internalError();

  if(needle->length == 0){ //prazdny retezec se vyskytuje na pozici 0
    *position =  0;
    return EOK;
    }


  //nejdriv se musi vytvorit pole fail, ktere se naplni indexy kam skakat pri
  //neshode

  //vytvareni vektoru fail, indexu na text a slovo, pomocny index
  int fail[needle->length], haystack_index, needle_index,next_pos;

  fail[0] = -1; //prvni je vzdycky -1

  for(int k = 1; k < needle->length; k++){   //prochazeni slovem
    next_pos = fail[k-1];
    while(next_pos > -1 && (needle->string[next_pos] != needle->string[k-1]))
      next_pos = fail[next_pos];         //pokud se nerovnaji, tak uz skace jinam
    fail[k] = next_pos+1;
  }

  //vyhledavani substringu
  haystack_index = 0;
  needle_index = 0;                         //pokud to jeste nepresahlo hranice
  while((haystack_index < haystack->length) && (needle_index < needle->length)){
    if(needle_index == -1 || (haystack->string[haystack_index] == needle->string[needle_index])){
      haystack_index++;
      needle_index++;
    }
    else
      needle_index=fail[needle_index];   //skace podle pole fail
  }
  if(needle_index == needle->length)
    *position = haystack_index - needle->length;
  else
    *position = -1;


  return EOK;

}




/******************************************************************************/
/******************************************************************************/
/**************************ZACATEK HASHOVACI TABULKY***************************/
/******************************************************************************/
/******************************************************************************/

/**
 * Inicializuje tabulku
 *
 * @param int size Velikost tabulky
 *
 * @return htable_t NULL nebo tabulka
 */
htable_t * htable_init(unsigned int size)
{

    htable_t * t = gcMalloc(sizeof(htable_t) + size*sizeof(htable_list));
    if(t == NULL)
        return NULL;
    t->htable_size = size;
    for(unsigned int i = 0; i < size; i++)
        list_init(t,i);
    return t;

}


/**
 * funkce na incializaci seznamu
 */
void list_init(htable_t * t, unsigned int index)
{
    t->ptr[index].head = NULL;

}


/**
 * vlozeni nove polozky do seznamu
 */
void list_insert_first(htable_list * list, htable_listitem* new_item)
{
    new_item->next = list->head;
    list->head = new_item;

}


/**
 * Vypis obsahu htable, pro kazdy datovy typ je jiny vystup
 *
 * @param htable_t t Tabulka, ktera se vypisuje
 */
void htable_print(htable_t * t)
{

     for(unsigned int i = 0; i < t->htable_size; i++){
        htable_listitem * ptr = t->ptr[i].head;
        //if(ptr != NULL)
          //printf("\n%d chlivek:\n",i);
        while(ptr != NULL){
            printf("id: \"%s\" ",ptr->name->string);
            if(ptr->var->type == T_INT)
              printf("type: INT  value: %d\n",ptr->var->data.integer);
            else if(ptr->var->type == T_REAL)
              printf("type: REAL  value: %lf\n",ptr->var->data.real);
            else if(ptr->var->type == T_STR)
              printf("type: STRING  value: \"%s\"\n",ptr->var->data.str->string);
            else if(ptr->var->type == T_BOOL)
              printf("type: BOOLEAN  value: %s\n",(ptr->var->data.boolean) ? "true" : "false");

            else if(ptr->var->type == T_FUNC) {
              printf("type: FUNC argc: %d ", ptr->var->data.function.argc);
              if(ptr->var->data.function.argc != 0){
                printf("argv: ");
                for(int j = 0; j < ptr->var->data.function.argc; j++)
                  printf("\"%s\" ",ptr->var->data.function.argv[j]->string);
                }
              printf("\n");
            }
            ptr = ptr->next;
            }

        }
}

/**
 * Vrati index do tabulky podle hashe zadanemu klici,
 *
 * cool hashovaci fce podle literatury
 *
 * @param str Klic - identifikator
 * @param htable_size Velikost tabulky
 *
 * @return index v tabulce
 */
int hash_function(const char *str, unsigned int htable_size)
{
    if(str == NULL)
      return -1;
    unsigned int h = 0;
    unsigned char *p;
    for(p=(unsigned char*)str; *p!='\0'; p++)
        h = 31*h + *p;
    return h % htable_size;
}


/**
 * funkce na vytvoreni nove polozky(key,data,next)
 */
htable_listitem * new_listitem(TString * name, TVar * var)
{
    htable_listitem * new_item;
    new_item = gcMalloc(sizeof(htable_listitem));
    if(new_item == NULL)
        return NULL;

    new_item->name = name;
    new_item->var = var;

    new_item->next = NULL;
    return new_item;

}

/**
 * Vyhleda polozku
 *
 * @param htable_t t Tabulka, kde se hleda
 * @param TString key Jmeno identifikatoru
 *
 * @return htable_listitem NULL nebo polozka
 */
htable_listitem * htable_find_item(htable_t *t, TString * key)
{
    unsigned int hash = hash_function(key->string, t->htable_size);

    for(htable_listitem *i = t->ptr[hash].head; i != NULL; i = i->next){
        if((strTStringCmp(key,i->name)) == 0)
            return i;
        }
    return NULL;

}

/**
 * Vlozeni polozky do tabulky
 *
 * @param htable_t t Tabulka, kam se strka
 * @param TString key Jmeno identifikatoru
 * @param TVar var Uzitecny data
 *
 * @return errcode (TODO: nutno synchronizovat)
 */
int htable_insert(htable_t * t, TString * key, TVar * var)
{
    unsigned int hash = hash_function(key->string,t->htable_size);
    htable_listitem * tmp = htable_find_item(t,key);

    TString * new_key = gcMalloc(sizeof(TString));
    if (new_key == NULL)
      return internalError();
    
    int result = strinit(new_key);
    if(result != EOK) return result;
    
    result = strTStringCpy(new_key, key);
    if(result != EOK) return result;
    
    if(!tmp){   //pokud nebylo nalezeno takove jmeno
      tmp = new_listitem(new_key,var);
      if(!tmp)  //pokud se nepovedl malloc
        return internalError();
      list_insert_first(&t->ptr[hash],tmp);
      return EOK;
    }
    return E_RUNTIME_SEMANT;

}


/**
 * Prepsani polozky v tabulce
 *
 * Vyhleda polozku, kdyz nenajde, zalozi novou, kdyz najde, tak prepise data
 *
 * @param htable_t t Tabulka, kam se strka
 * @param TString key Jmeno identifikatoru
 * @param TVar var Uzitecny data
 *
 * @return errcode
 */
int htable_overwrite(htable_t * t, TString * key, TVar * var)
{
    unsigned int hash = hash_function(key->string,t->htable_size);
    htable_listitem * tmp = htable_find_item(t,key);
    if(tmp == NULL){
      tmp = new_listitem(key,var);
      if(tmp == NULL)
        return internalError();

      list_insert_first(&t->ptr[hash],tmp);      
    }
    else{
      tmp->var = var;
    }

    return EOK;
}

/*
 * -----------------------------------------------------------------------------
 *         DALSI ADT A POMOCNE FUNKCE - NESOUVISI PRIMO S PREDMETEM IAL
 * -----------------------------------------------------------------------------
 */

/*
 * ---------------------------------------------------------------
 *      ZASOBNIK TABULEK SYMBOLU - potrebne pro interpretaci
 * ---------------------------------------------------------------
 */


/** Alokace a inicializace prázdného zásobníku
 * Příklad použití:
 * htableStack* stack = htable_stack_init();
 *
 * (Všechny následující operace se zásobníkem
 *  jsou automaticky prázdné operace, pokud
 *  není zásobník správně inicializovaný)
 *
 * @return Odkaz na prázný alokovaný zásobník
 */
htableStack *htable_stack_init(){
  htableStack *stack;
  stack = gcMalloc(sizeof(htableStack));
  if (stack == NULL) return NULL;   ///VRACET INTERNAL ERROR pri pouziti tohodle s nullem

  stack->top = -1;
  stack->allocated = STACK_SIZE;

  stack->tables = gcMalloc(sizeof(htable_t*)*STACK_SIZE);
  if (stack == NULL) return NULL;

  return stack;
}

/** Zjistí, zda-li je daný zásobník prázný
 *
 * @param s Odkaz na zásobník
 * @return Nula pokud není, nenulová hodnota pokud je
 */
int htable_stack_empty(htableStack *stack){
  if (stack->top == -1)
    return 1;
  else
    return 0;
}

int htable_stack_nearly_empty(htableStack *stack){
  if (stack->top == 0)
    return 1;
  else
    return 0;
}

/** Získání hodnoty na vrcholu zásobníku
 * (Bez jeho odebrání; pouze přečtení)
 * (Pokud je zásobník prázdný, bude vráceno NULL)
 *
 * @param s Odkaz na zásobník
 * @return htable_t - vrchol zásobníku | NULL
 */
htable_t *htable_stack_top(htableStack *stack){
  if (htable_stack_empty(stack))
    return NULL;

  return stack->tables[stack->top];
}

/** Odstranení prvku z vrcholu zásobníku
 * (bez jeho přečtení, pouze odebrání)
 * (je-li zásobník prázný, prázdná operace)
 *
 * @param s Odkaz na zásobník
 */
void htable_stack_pop(htableStack *stack){
  if (htable_stack_empty(stack))
    return;
  stack->top -= 1;
}

/** Vložení htable_t na vrchol zásobníku
 *
 * @param s Odkaz na zásobník
 * @param d Data ke vložení - htable_t
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit zasobnik
 */
int htable_stack_push(htableStack *stack,htable_t *table){
  stack->top += 1;

  if (stack->top == stack->allocated){
    stack->allocated += STACK_SIZE;
    stack->tables = gcRealloc(stack->tables, sizeof(htable_t*)*stack->allocated);

    if (stack->tables == NULL) return internalError();
  }

  stack->tables[stack->top] = table;

  return EOK;

}

/*
 * ---------------------------------------------------------------------------
 */

/**
 * Prohodi znaky dle jejich indexu.
 *
 * @param input ukazatel na retezec, ve kterem ma prohodit znaky
 * @param i index prvniho znaku
 * @param j index druheho znaku
 */
void swap(char* input, int i, int j)
{
    char aux = input[i];
    input[i] = input[j];
    input[j] = aux;
}

/**
 * Porovna ordinalni hodnotu vstupnich znaku.
 *
 * @param first prvni znak
 * @param second druhy znak
 * @return bool (prvni>druhy || !(prvni>druhy))
 */
bool less(char first, char second)
{
    return first < second;
}



int get_substring(TVar * substr, TVar * str, int start, int end){

  if( start < 0 || end < 0 || start > end ||
    start >= strTStringLen(str->data.str) || end > strTStringLen(str->data.str))
    return otherSemanticError("Nespravny index pro get_substring");

  substr->data.str = gcMalloc(sizeof(TString));
  if(substr->data.str == NULL)
   return internalError();

  int result = strinit(substr->data.str);
  if ( result != EOK) return result;

  for(int i = start; i < end; i++){
    strCharAppend(substr->data.str, str->data.str->string[i]);
  }


  return EOK;

}



/*
 * -------------------------------------------------------------------------------
 *                              FRONTA ARGUMENTU
 * -------------------------------------------------------------------------------
 */

/** Alokace a inicializace prázdné fronty
 * Příklad použití:
 * TStringQueue* front = TString_queue_init();
 *
 * (Všechny následující operace s frontou
 *  jsou automaticky prázdné operace, pokud
 *  není zásobník správně inicializovaný)
 *
 * @return Odkaz na práznout alokovanou frontu
 */
TStringQueue *TString_queue_init(){
  TStringQueue * queue = gcMalloc(sizeof(TStringQueue));
  if (queue == NULL) return NULL;

  queue->bottom = 0;
  queue->top = 0;
  queue->allocated = STACK_SIZE;

  queue->queue = gcMalloc(sizeof(TString*)*queue->allocated);
  if (queue->queue == NULL) return NULL;

  return queue;

}

/** Zjistí, zda-li je daná fronta  prázndá
 *
 * @param s Odkaz na frontu
 * @return Nula pokud není, nenulová hodnota pokud je
 */
int TString_queue_empty(TStringQueue *queue){
  if (queue->top == queue->bottom)
    return 1;
  else
    return 0;
}

/** Získání hodnoty na začátku fronty
 * (Bez jeho odebrání; pouze přečtení)
 * (Pokud je fronta prázdná, bude vráceno NULL)
 *
 * @param s Odkaz na frontu
 * @return TString - začátek fronty | NULL
 */
TString *TString_queue_top(TStringQueue *queue){
  if (TString_queue_empty(queue))
    return NULL;

  return queue->queue[queue->top];
}

/** Odstranení prvku z vrcholu fronty
 * (bez jeho přečtení, pouze odebrání)
 * (je-li fronta prázdná, prázdná operace)
 *
 * @param s Odkaz na frontu
 */
void TString_queue_pop(TStringQueue *queue){
  if (TString_queue_empty(queue))
    return;

  queue->top += 1;
}

/** Vložení TString na konec fronty
 *
 * @param s Odkaz na frontu
 * @param d Data ke vložení - TString*
 *
 * @return EOK pokud vse vporadku, E_INTERNAL pokud se nepodarilo rozsirit frontu
 */
int TString_queue_put(TStringQueue *queue,TString **arg){

  if (queue->bottom == queue->allocated-1){
    queue->allocated += STACK_SIZE;
    queue->queue = gcRealloc(queue->queue,sizeof(TString*)*queue->allocated);

    if (queue->queue == NULL) return internalError();
  }

  queue->queue[queue->bottom] = *arg;

  queue->bottom += 1;

  return EOK;
}

/** Vyprázdnění fronty
 *
 * @param s Odkaz na frontu
 */
void TString_queue_set_empty(TStringQueue *queue){
    queue->top = 0;
    queue->bottom = 0;
  }

  void TString_queue_print(TStringQueue *queue){
  if (TString_queue_empty(queue))
    return;
  for (int i = queue->top; i < queue->bottom; i++)
    printf("argument %d: %s\n",i,queue->queue[i]->string);

}


/*
 * -------------------------------------------------------------------------------
 *                              ZASOBNIK CYKLU
 * -------------------------------------------------------------------------------
 */

loopStack *loopStack_init(){
  loopStack *stack;
  stack = gcMalloc(sizeof(loopStack));
  if (stack == NULL) return NULL;   ///VRACET INTERNAL ERROR pri pouziti tohodle s nullem

  stack->top = -1;
  stack->allocated = STACK_SIZE;

  stack->stack = gcMalloc(sizeof(TLoop*)*STACK_SIZE);
  if (stack == NULL) return NULL;

  return stack;
}


int loopStack_empty(loopStack *stack){
  if (stack->top == -1)
    return 1;
  else
    return 0;
}

TLoop *loopStack_top(loopStack *stack){
  if (loopStack_empty(stack))
    return NULL;

  return stack->stack[stack->top];
}

void loopStack_pop(loopStack *stack){
  if (loopStack_empty(stack))
    return;
  stack->top -= 1;
}

int loopStack_push(loopStack *stack,void* breakLab, void* contLab){
  stack->top += 1;

  TLoop *tmpItem = gcMalloc(sizeof(TLoop));
  if (tmpItem == NULL) return internalError();

  tmpItem->breakLab = breakLab;
  tmpItem->contLab = contLab;

  if (stack->top == stack->allocated){
    stack->allocated += STACK_SIZE;
    stack->stack = gcRealloc(stack->stack, sizeof(TLoop*)*stack->allocated);

    if (stack->stack == NULL) return internalError();
  }

  stack->stack[stack->top] = tmpItem;

  return EOK;
}
