/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : parser.c - Modulu provadejici syntaktickou analyzu struktury programu
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
#include <string.h>

#include "parser.h"
#include "scanner.h"
#include "error.h"
#include "expr.h"
#include "ilist.h"
#include "ial.h"

TString tkAttr;
tState tkState;
int tkLine = 1;

htable_t *funTable;
tIlist *iList;
htableStack * symbolStack;
TStringQueue* argQueue;

/**
 * zasobnik cyklu, pro kazdy cyklus ve kterem se program aktualne nachazi
 * generuje na vrcholu zasobniku dvojici adres navesti, na ktere provede
 * skok v pripade, kdy narazi na prikazy break a continue (pouze u cyklu for)
 */
loopStack *loops;

/*
 * Deklarace
 */

int innerBlock();
int statement();


/*
 * obsluzne funkce
 */

/**
 * Zjisti, zda identifikator odpovida vestavene funkci
 * @param str Zkoumany identifikator
 * @return 1 pokud je identifikatorem vestavene funkce, jinak 0
 */
int isBuiltinFunc(TString * str) {

  if ( strcmp(str->string, "put_string") == 0 ||
       strcmp(str->string, "get_string") == 0 ||
       strcmp(str->string, "strlen") == 0 ||
       strcmp(str->string, "get_substring") == 0 ||
       strcmp(str->string, "find_string") == 0 ||
       strcmp(str->string, "sort_string") == 0 ||
       strcmp(str->string, "boolval") == 0 ||
       strcmp(str->string, "doubleval") == 0 ||
       strcmp(str->string, "intval") == 0 ||
       strcmp(str->string, "strval") == 0
     )
     return 1;


  return 0;
}




void printToken(){
  printStateLabel(tkState);
  printf(" : attr: %s\n", tkAttr.string);
}

/*
 * Funkce parseru
 */

int term(){
  int result = EOK;
  switch (tkState){
  case S_DEC:
  case S_REAL:
  case S_REALEXP:
  case S_STRING:
  case KW_TRUE:
  case KW_FALSE:
  case KW_NULL:
  case S_VAR_IDENT:

    result = EOK;
    break;

  default:
    result = syntaxError();
    break;

  }

  if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();
  return result;
}

int expression() {
  return precSA(funTable,iList,symbolStack);
}

int isExprStart(tState tkState){
  switch(tkState){
    case S_DEC:
    case S_REAL:
    case S_REALEXP:
    case S_STRING:
    case KW_TRUE:
    case KW_FALSE:
    case KW_NULL:
    case S_VAR_IDENT:
    case S_L_PARENTH:
    case S_SUB:
      return 1;
    default:
      return 0;
  }


}
int argsNext(){
  int result = EOK;
  bool wasMinus = false;
  // pravidlo <args-next> -> eps
  if (tkState == S_R_PARENTH) return EOK;

  // pravidlo <args-next> -> , <term> <args-next>
  // token je jiz nacteny
  if ( tkState != S_COMMA ) return syntaxError();

  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;

  //unarni minus
  if (tkState == S_SUB){
    wasMinus = true;
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  }

  //nahrani dalsiho argumentu do fronty
  TString *tmp = gcMalloc(sizeof(TString));
  if (tmp == NULL) return internalError();
  result = strinit(tmp);
  if (result != EOK) return result;

  //docasne ulozeni tokenu protoze term() si pak nacte dalsi
  tState tmptkState = tkState;
  TString *tmptkAttr = gcMalloc(sizeof(TString));
  if (tmptkAttr == NULL) return internalError();

  result = strinit(tmptkAttr);
  if (result != EOK) return result;

  result = strTStringCpy(tmptkAttr,&tkAttr);
  if (result != EOK) return result;

  result = term();
  if (result != EOK ) return result;


  if (tmptkState == S_VAR_IDENT){ //pokud je to promenna, jen zkopiruje nazev
    result = strTStringCpy(tmp,tmptkAttr);
    if (result != EOK) return result;
  }
  else{ //jinak musi vyrobit docasnou promennou a prevest
    TVar *var;
    var = constantToVar(&tmptkState,tmptkAttr);
    if (var == NULL)
      return internalError();
    generateVariable(tmp);

    result = htable_insert(htable_stack_top(symbolStack),tmp,var);
    if (result != EOK) return result;

  }
  //generace NEG pokud bylo pouzito unarni minus
  if (wasMinus)
    igenerate(iList,I_NEG,tmp,tmp,NULL);

  //tohle udela interpret
  // result = TString_queue_put(argQueue,&tmp);
  //if (result != EOK) return result;

  result = igenerate(iList,I_PUSHPAR,tmp,NULL,NULL);
  if (result != EOK) return result;


  // token jiz nacteny
  result = argsNext();
  if (result != EOK ) return result;

  // asi nenacitat dalsi, protoze pravou zavorku osetruje funcCall
  //if getToken(&tkAttr, &tkState, &tkLine )== E_LEX) return E_LEX;
  return result;
}

int args() {
  int result = EOK;
  bool wasMinus = false;

  // pravidlo <args> -> eps
  if (tkState == S_R_PARENTH) return EOK;

  //unarni minus
  if (tkState == S_SUB){
    wasMinus = true;
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  }

  //nahrani prvniho argumentu do fronty
  TString *tmp = gcMalloc(sizeof(TString));
  if (tmp == NULL) return internalError();
  result = strinit(tmp);
  if (result != EOK) return result;

  //docasne ulozeni tokenu protoze term() si pak nacte dalsi
  tState tmptkState = tkState;
  TString *tmptkAttr = gcMalloc(sizeof(TString));
  if (tmptkAttr == NULL) return internalError();

  result = strinit(tmptkAttr);
  if (result != EOK) return result;

  result = strTStringCpy(tmptkAttr,&tkAttr);
  if (result != EOK) return result;


  // <args> -> <term> <args-next>
  result = term();
  if (result != EOK ) return result;

  if (tmptkState == S_VAR_IDENT){ //pokud je to promenna, jen zkopiruje nazev
    result = strTStringCpy(tmp,tmptkAttr);
    if (result != EOK) return result;
  }
  else{ //jinak musi vyrobit docasnou promennou a prevest
    TVar *var;
    var = constantToVar(&tmptkState,tmptkAttr);
    if (var == NULL)
      return internalError();
    generateVariable(tmp);

    result = htable_insert(htable_stack_top(symbolStack),tmp,var);
    if (result != EOK) return result;

  }
  //generace NEG pokud bylo pouzito unarni minus
  if (wasMinus)
    igenerate(iList,I_NEG,tmp,tmp,NULL);

  //tohle udela interpret
  // result = TString_queue_put(argQueue,&tmp);
  //if (result != EOK) return result;

  result = igenerate(iList,I_PUSHPAR,tmp,NULL,NULL);
  if (result != EOK) return result;

  result = argsNext();
  if (result != EOK ) return result;

  //UKONCENI PRAVOU ZAVORKOU OSETRI VOLAJICI: NENACITEJ DALSI
  return result;
}


int funcCall() {
  htable_listitem * tmp;
  TString *tmpTkAttr = gcMalloc(sizeof(TString));
  if (tmpTkAttr == NULL) return internalError();

  int result = strinit(tmpTkAttr);
  if (result != EOK) return result;

  result = strTStringCpy(tmpTkAttr,&tkAttr);
  if (result != EOK) return result;

  /*
   * Priznak toho, zda je identifikator funkce vestavena funkce,
   * Pokud ano, uchovava identifikator odpovidajiciho typu instrukce
   */
  int isBuiltin = 0;

  if ( tkState != S_IDENT ) return syntaxError();
  //nacteny identifikator,

  /* kontrola, zda se nevola vestavena funkce */
  if ( strcmp(tkAttr.string, "put_string") == 0)
  {
    isBuiltin = I_PUTSTR;
  }
  else if ( strcmp(tkAttr.string, "get_string") == 0)
  {
    isBuiltin = I_GETSTR;
  }
  else if ( strcmp(tkAttr.string, "strlen") == 0)
  {
      isBuiltin = I_STRLEN;
  }
  else if ( strcmp(tkAttr.string, "get_substring") == 0)
  {
      isBuiltin = I_GET_SUBSTR;
  }
  else if ( strcmp(tkAttr.string, "find_string") == 0)
  {
      isBuiltin = I_FIND_STR;
  }
  else if ( strcmp(tkAttr.string, "sort_string") == 0)
  {
      isBuiltin = I_SORT_STR;
  }
  else if ( strcmp(tkAttr.string, "boolval") == 0)
  {
      isBuiltin = I_BOOLVAL;
  }
  else if ( strcmp(tkAttr.string, "doubleval") == 0)
  {
      isBuiltin = I_DOUBLEVAL;
  }
  else if ( strcmp(tkAttr.string, "intval") == 0)
  {
      isBuiltin = I_INTVAL;
  }
  else if ( strcmp(tkAttr.string, "strval") == 0)
  {
      isBuiltin = I_STRVAL;
  }
  else /* volana fce neni vestavena funkce, pokus se ji najit v tabulce funkci */
  {
    //kontrola jestli identifikator (funkce) uz je ci neni v tabulce fci
    tmp = htable_find_item(funTable,&tkAttr);

    if (tmp == NULL){ //neni tam -> nova funkce

     // printf("pusteni nezname funkce: %s\n", tkAttr.string);
      TVar *func = gcMalloc(sizeof(TVar));
      func->type = T_FUNC;  //tvar dostane typ funkce
      func->data.function.start = NULL; //jeste se nevi kde je start
      htable_insert(funTable,&tkAttr,func);
    }

  }

  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  if ( tkState != S_L_PARENTH ) return syntaxError();

  // argumenty funkce:
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  result = args();
  if (result != EOK) return result;

  //generovani call
  /* Pokud jsem ovsem na zacatku narazil na vestavenou funkci, negeneruji CALL,
   * ale instrukci dane vestavene funkce */
  if (isBuiltin)
    result = igenerate(iList,isBuiltin,NULL, NULL, NULL);
  else // neni vestavena funkce
    result = igenerate(iList,I_CALL,tmpTkAttr,NULL,NULL);
  if (result != EOK) return result;

  // token mam nacteny z volani args()
  if ( tkState != S_R_PARENTH ) return syntaxError();
  return result;
}

int forAssign() {

  TString *resval; //promenna ze ktere se bude prirazovat
  int result = EOK;
  // ocekava prvni token jiz nacteny

  TString *tmpString = gcMalloc(sizeof(TString));
  if (tmpString == NULL) return internalError();
  result = strinit(tmpString);
  if (result != EOK) return internalError();
  result = strTStringCpy(tmpString,&tkAttr);
  if(result != EOK) return internalError();

  if( tkState == S_VAR_IDENT) { //jinak mi musel prijit identifikator promenne
    //za identifikatorem musi nasledovat '='
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    if ( tkState != S_ASSIGN ) return syntaxError();

    //nacti dalsi, musi byt <expression>
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    result = expression();
    if ( result != EOK ) return result;

    //po expression je vysledna promenna v prvnim operandu
    resval =  ((tListItem *) listGetPointerLast(iList))->instr.op1;
    result = igenerate(iList,I_ASSIGN,tmpString,resval,NULL);
    if (result != EOK) return result;
  }

  return result;
}

int elseifLoop(void * prevJmpAddr, void ** endLabel){

   if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();


   // <ifbody> -> <elseifbody> <ifbody>
   if( tkState == KW_ELSEIF ){
     // <elseifbody> -> elseif ( <expr> )  { <inner-block> }
     void *elseIfLabel; // zde zacina blok <elseif>
     int result = igenerate(iList, I_LABEL, NULL, NULL, NULL);
     if( result != EOK ) return result;
     elseIfLabel = listGetPointerLast(iList);

     listGoto(iList, prevJmpAddr);      // naplnim adresu, kterou jsem dostal z minuleho volani
     tInstr *datax;
     datax = listGetData(iList);
     datax->op1 = elseIfLabel;

     // zacina podminka  elseif (
     if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
     if ( tkState != S_L_PARENTH ) return syntaxError();

     //  elseif ( <expr>
     if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
     result = expression();
     if (result != EOK ) return result;

     // hodnota vyrazu
     void * resvalx =  ((tListItem *) listGetPointerLast(iList))->instr.op1;
     // znegujeme a ulozime zpatky
     result = igenerate(iList, I_NOT, resvalx, resvalx, NULL);
     if( result != EOK ) return result;

     // kdyz neplati, skoci se jinam (na dalsi elseif nebo pryc)
     result = igenerate(iList, I_IFJUMP, NULL, (void*) resvalx, NULL);
     if (result != EOK) return result;
     prevJmpAddr = listGetPointerLast(iList);

     //  elseif ( <expr> )
     if ( tkState != S_R_PARENTH ) return syntaxError();

     // elseif ( <expression> ) {
     if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
     if ( tkState != S_L_BRACE ) return syntaxError();

     // elseif ( <expression> ) { <inner-block>
     if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
     result = innerBlock();
     if ( result != EOK ) return result;

     // token nacten z innerBlock
     // elseif ( <expression> ) { <inner-block> }
     if ( tkState != S_R_BRACE ) return syntaxError();

     // kdyz se provede cele telo, musi se vyskocit ven
     void *endJumpAddr;
     result = igenerate(iList, I_JMP, NULL, NULL, NULL);
     if( result != EOK ) return result;
     endJumpAddr = listGetPointerLast(iList);


     result = elseifLoop( prevJmpAddr, endLabel);  //volani rekurze
     if( result != EOK ) return result;


     // z rekurze jsme ziskali koncove navesti, muzeme plnit
     listGoto(iList, endJumpAddr);
     datax = listGetData(iList);
     datax->op1 = *endLabel;

     return EOK;

   }

   // <ifbody> -> <elsebody>
   else if ( tkState == KW_ELSE ){

     void *elseLabel;    // zde zacina blok <else>
     int result = igenerate(iList, I_LABEL, NULL, NULL, NULL);
     if( result != EOK ) return result;
     elseLabel = listGetPointerLast(iList);

     //ani jedna z predchozich podminek nevysla = skace se sem
     listGoto(iList, prevJmpAddr);
     tInstr *datax;
     datax = listGetData(iList);
     datax->op1 = elseLabel;


     //samotny blok  <else>
     if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
     // else {
     if ( tkState != S_L_BRACE ) return syntaxError();

     // else { <innerBlock>
     if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
     result = innerBlock();
     if ( result != EOK ) return result;

     // else { <innerBlock> }
     if ( tkState != S_R_BRACE ) return syntaxError();

     // a zde konci cela serie: vygenerujeme koncove navesti a posilame ho zpet

      // zde konci celej if/elseif/else
     result = igenerate(iList, I_LABEL, NULL, NULL, NULL);
     if( result != EOK ) return result;
     *endLabel = listGetPointerLast(iList);

     if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;

     return EOK;


   }

   // <ifbody> -> eps
   else{     //neprisel elseif ani else -> hodim sem i prevJmpAddr a taky
            //zmenim endLabel na ttento label

     void *finalLabel;    // zde zacina blok <else>
     int result = igenerate(iList, I_LABEL, NULL, NULL, NULL);
     if( result != EOK ) return result;
     finalLabel = listGetPointerLast(iList);

     //ani jedna z predchozich podminek nevysla = skace se sem
     listGoto(iList, prevJmpAddr);
     tInstr *datax;
     datax = listGetData(iList);
     datax->op1 = finalLabel;


     //poslu zpet ostatnim if/elseif
     *endLabel = finalLabel;


     return EOK;

   }

}



int statement(){
  //prvni token nacteny od volajiciho

  int result = EOK;

  switch (tkState){
  /*
   * ROZSIRENI: CYKLUS FOR... for($id=<vyraz>;<vyraz>;$idx=<vyraz>){};
   * vcetne operatoru break a continue
   */
  case KW_BREAK:
    if (loopStack_empty(loops))
      return otherSemanticError("Prikazy break/continue se mohou vyskytovat pouze v cyklu for");

    result = igenerate(iList, I_JMP, loopStack_top(loops)->breakLab, NULL, NULL);
    if (result != EOK) return result;


    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    if ( tkState != S_SEMICOLON) return syntaxError();
    break;

  case KW_CONT:
    if (loopStack_empty(loops))
      return otherSemanticError("Prikazy break/continue se mohou vyskytovat pouze v cyklu for");

    result = igenerate(iList, I_JMP, loopStack_top(loops)->contLab, NULL, NULL);
    if (result != EOK) return result;

    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    if ( tkState != S_SEMICOLON) return syntaxError();
    break;

  case KW_FOR:

    // vstupujes do cyklu -> zvys pocitadlo cyklu
    loopStack_push(loops,NULL, NULL);

    if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();
    if ( tkState != S_L_PARENTH ) return syntaxError();

    if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();

    /* ============================================================
     * PRVNI SEKCE HLAVICKY FOR
     */
    result = forAssign();
    if(result != EOK) return result;

    // pokud byla sekce prazdna, nedelej nic, ovsem ikdyz prazdna nebyla
    // musi koncit strednikem (token je nacten od expr)
    if ( tkState != S_SEMICOLON ) return syntaxError();

    /* ============================================================
     * SEKCE 2, obsahuje vyraz nebo nic (nic = true)
     */

    // Je nutne vygenerovat navesti uvozujici sekci podminky:
    result = igenerate(iList, I_LABEL, NULL, NULL, NULL);
    if(result != EOK) return result;

    //zamapatuj si jeji adresu
    void * condLabAddr = listGetPointerLast(iList);

    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;

    void *res;
    if (isExprStart(tkState)){ //pokud je vyraz, vyhodnot ho
      result = expression();
      if ( result != EOK ) return result;

      // ziskej hodnotu vyrazu a zneguj ji
      res =  ((tListItem *) listGetPointerLast(iList))->instr.op1;
      result = igenerate(iList, I_NOT, res, res, NULL);
      if(result != EOK) return result;

    } else {
      // pro nekonecny cyklus vygeneruj konstantni FALSE a neneguj ho
      generateFalse();
      res =  ((tListItem *) listGetPointerLast(iList))->instr.op1;
    }


    // pokud podminka neplatila (pred znegovanim), skoc na koncove navesti (neznam ho zatim)
    result = igenerate(iList, I_IFJUMP, NULL, res, NULL);
    if(result != EOK) return result;
    void *breakJumpAddr = listGetPointerLast(iList);

    // skok na navesti tela cyklu, navesti opet neznam
    result = igenerate(iList, I_JMP, NULL, NULL, NULL);
    if(result != EOK) return result;
    void *bodyJumpAddr = listGetPointerLast(iList);

    // musi koncit strednikem, nicmene je tu finta, ze tato kontrola
    // nevi, kdo ji ten token nacetl -> jedna podminka pro prazne i neprazdne telo sekce
    if ( tkState != S_SEMICOLON ) return syntaxError();


    /* ============================================================
     * sekce 3, obsahuje $idy = <vyraz>, pripadne je prazdny
     */

    result = igenerate(iList, I_LABEL, NULL, NULL, NULL);
    if(result != EOK) return result;
    void * postLabAddr = listGetPointerLast(iList);

    //na adresu postLabAddr skace prikaz continue;
    //vloz polozku do vrcholu zasobniku cyklu
    loopStack_top(loops)->contLab = postLabAddr;

    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    //token je nacten ze sekce2
    result = forAssign();
    if(result != EOK) return result;


    // postup hlavickou neni linearni, po sekci 3 se opet kontroluje podminka
    // proto provedu nepodmineny skok (adresu navesti znam, proto si skok nemusim pamatovat)
    result = igenerate(iList, I_JMP, condLabAddr, NULL, NULL);
    if(result != EOK) return result;

    /*
     * Konec sekci -> zacina telo
     */

    // telo musi byt uvozeno navestim. Na toto navesti se jiz nektere skoky odkazovaly,
    // proto doplnim jejich operandy
    result = igenerate(iList, I_LABEL, NULL, NULL, NULL);
    if(result != EOK) return result;
    void * bodyLabAddr = listGetPointerLast(iList);

    listGoto(iList, bodyJumpAddr);
    tInstr *d;
    d = listGetData(iList);
    d->op1 = bodyLabAddr;


    /* zacina telo cyklu */
    /*
     * Zde nastave jeden problem s prikazem break...v tele cyklu se muze vyskytnout nekolik
     * prikazu break, jim potrebuju priradit spravnou hodnotu pro skok, jenze navesti jeste neznam!
     * Doted to problem nedelalo, protoze instrukce byla vzdy jen jedna, tak jsem si ji lokalne zapamatoval.
     *
     * Proto v tento moment vygeneruju navesti pomoci igenerateNoAppend, ziskam jeho adresu a nakonci, v dobe,
     * kdy bych navesti teprve generoval ho pouze vlozim nakonec
     */


    tListItem * breakLabAddr = igenerateNoAppend( I_LABEL, NULL, NULL, NULL);
    if( breakLabAddr == NULL) return internalError();

    listGoto(iList, breakJumpAddr);
    d = listGetData(iList); //d je pomocna struktura definovana vyse
    d->op1 = breakLabAddr;

    loopStack_top(loops)->breakLab = breakLabAddr;


    // for (-,-,-) {
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    if ( tkState != S_L_BRACE ) return syntaxError();

    //if ( <expression> ) { <inner-block>
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    result = innerBlock();
    if ( result != EOK ) return result;

    //if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();
    //token nacen z vnitrniho bloku
    //nacteno: if ( <expression> ) { <inner-block> }
    if ( tkState != S_R_BRACE ) return syntaxError();


    /* konec iterace cyklu */
    /* Na konci iterace cyklu se teprve provadi sekce 3 */
    result = igenerate(iList, I_JMP, postLabAddr, NULL, NULL);
    if(result != EOK) return result;

    /* konec celeho cyklu, je uvozen navestim */
    /* jenze navesti jsem generoval uz driv, tak ho tam vlozim cele, rovnou, nic negeneruju */
    listInsertLastItem(iList, breakLabAddr);


    loopStack_pop(loops); // odstran vrchol ze zasobniku cyklu

    // vystupujes do cyklu -> sniz pocitadlo cyklu
    //loopCounter--;
    break;



  case KW_IF:
    // nacteno: if (
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
   if ( tkState != S_L_PARENTH ) return syntaxError();

   /* Semanticka akce:
    * Funkce expression vygeneruje instrukce pro vyhodnocení výrazu
    * Poslední instrukce obsahuje výsledek (pozice výsledku v instrukci
    * je udána sémantikou instrukce {op1/op2/op3})
    */
   if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
   result = expression();
   if (result != EOK ) return result;


   // hodnota vyrazu
   void * resvalx =  ((tListItem *) listGetPointerLast(iList))->instr.op1;
   // znegujeme a ulozime zpatky
   result = igenerate(iList, I_NOT, resvalx, resvalx, NULL);
   if( result != EOK ) return result;

   /* semanticka akce
    * Provedu podmineny skok na navesti L1, jehoz adresu vsak jeste neznam!
    * Budu si tedy muset zapamatovat adresu teto instrukce, abych se k ni
    * mohl vratit, az budu adresu navesti znat
    */

   void *prevJmpAddr; //adresa instrukce skoku na dalsi elseif/else
   result = igenerate(iList, I_IFJUMP, NULL, (void*) resvalx, NULL);
   if( result != EOK ) return result;
   prevJmpAddr = listGetPointerLast(iList);

   /*
    * Nasleduje telo bloku <if>
    */

   // if ( <expression> )
   // if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();
   if ( tkState != S_R_PARENTH ) return syntaxError();

   // if ( <expression> ) {
   if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
   if ( tkState != S_L_BRACE ) return syntaxError();

   //if ( <expression> ) { <inner-block>
   if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
   result = innerBlock();
   if ( result != EOK ) return result;

   //token nacen z vnitrniho bloku
   //nacteno: if ( <expression> ) { <inner-block> }
   if ( tkState != S_R_BRACE ) return syntaxError();

   /*
    * Provedla se sekce <if>, tedy musim nepodminene skocit na konec
    * bloku if-elseif-else, tedy navesti L2, jehoz adresu opet neznam zatim
    */

   void *endJumpAddr;
   result = igenerate(iList, I_JMP, NULL, NULL, NULL);
   if( result != EOK ) return result;
   endJumpAddr = listGetPointerLast(iList);

   void * endLabel;

   // <stat> -> if ( <expr> ) { <inner-block> } <ifbody>
   int result = elseifLoop(prevJmpAddr, &endLabel);  //volani rekurze
   if( result != EOK ) return result;

   // nastaveni posledniho endjump skoku
   listGoto(iList, endJumpAddr);
   tInstr * datax;
   datax = listGetData(iList);
   datax->op1 = endLabel;


    return EOK;

  case KW_WHILE:{
    // nacteno: while (
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    if ( tkState != S_L_PARENTH ) return syntaxError();

    /* Semanticka akce:
     * Vygeneruju navesti, ktere odpovida zacatku cyklu, na konci tela cyklu
     * se skoci zpatky sem (pokud bude podminka splnena
     */
    void *label1;
    result = igenerate(iList,I_LABEL, NULL, NULL, NULL);
    if( result != EOK ) return result;
    label1 = listGetPointerLast(iList);

    /* Semanticka akce:
     * Funkce expression vygeneruje instrukce pro vyhodnocení výrazu
     * Poslední instrukce obsahuje výsledek (pozice výsledku v instrukci
     * je udána sémantikou instrukce {op1/op2/op3})
     */
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    result = expression();
    if (result != EOK ) return result;

    /* Semanticka akce:
     * Nacte posledni instrukci seznamu, ktera obsahuje vysledek, zneguje ji
     * a uloží zpět. Následně provede podmíněný skok, kterým buď ukončí
     * cyklus nebo provede tělo vyklu
     */
    //---- pouze cvicna promenna: smazat az bude hotove vyhodnocovani vyrazu TODO
    //TVar t;
    //t.type = T_INT;
    //t.data.integer = 10;
    //igenerate(iList, I_ADD, &t, &t, &t);
    // ----

    // hodnota vyrazu
    TVar * resval =  ((tListItem *) listGetPointerLast(iList))->instr.op1;
    // znegujeme a ulozime zpatky
    result = igenerate(iList, I_NOT, (void*) resval, (void*) resval, NULL);
    if( result != EOK ) return result;
    /* Semanticka akce:
     * Vygeneruje podmineny skok na konec cyklu v pripade, kdy podminka neplati
     * Vyraz jsme negovali, takze podminka neplati pokud v resval je TRUE.
     * V tento poment ovsem nezname adresu navesti, kam bychom meli skocit,
     * tu dopline pozdeji, proto si ale musime pamatovat adresu tohoto skoku
     */

    void *ifJumpAddr;
    result = igenerate(iList, I_IFJUMP, NULL, (void*) resval, NULL);
    if( result != EOK ) return result;
    ifJumpAddr = listGetPointerLast(iList);

    // while ( <expression> )
    //if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();
    if ( tkState != S_R_PARENTH ) return syntaxError();

    // if|while ( <expression> ) {
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    if ( tkState != S_L_BRACE ) return syntaxError();

    //if|while ( <expression> ) { <inner-block>
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    result = innerBlock();
    if ( result != EOK ) return result;


    /* Semanticka akce:
     * Jsme na konci tela cyklu, proto provedeme nepodmineny skok na zacatek pro overeni podminky.
     * Jedna se o posledni instrukci cyklu, proto za ni vygenerujeme navesti znacici konec cyklu.
     * Toto navesti uz jsme uvazovali na zacatku, nyni vsak zname i jeho adresu, proto ji zpetne doplnime
     */
    result = igenerate(iList, I_JMP, label1, NULL, NULL);
    if( result != EOK ) return result;

    // nagenerujeme instrukci druheho navesti
    result = igenerate(iList, I_LABEL, NULL, NULL, NULL);
    if( result != EOK ) return result;

    void *endLabel;
    endLabel = listGetPointerLast(iList);

    listGoto(iList, ifJumpAddr);
    tInstr *data;
    data = listGetData(iList);
    data->op1 = endLabel;

    //if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();
    //token nacen z vnitrniho bloku
    if ( tkState != S_R_BRACE ) return syntaxError();
    // TODO: smazat - slouzi pouze jako graficky oddelovat
    //igenerate(iList, I_NOP, NULL, NULL, NULL);

    break;
    }
  case S_VAR_IDENT:{
    //uchovani nazvu identifikatoru do ktereho se bude prirazovat
    TString *tmpString = gcMalloc(sizeof(TString));
    if (tmpString == NULL) return internalError();
    result = strinit(tmpString);
    if (result != EOK) return internalError();
    result = strTStringCpy(tmpString,&tkAttr);
    if(result != EOK) return internalError();

    TString *resval; //promenna ze ktere se bude prirazovat

    //za identifikatorem musi nasledovat '='
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
    if ( tkState != S_ASSIGN ) return syntaxError();

    //nacti dalsi, musi byt bud <expression> nebo <func-call>
    if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;

    if ( isExprStart(tkState )) { // todo: r-hodnota je vyraz
      result = expression();
      if ( result != EOK ) return result;

    //po expression je vysledna promenna v prvnim operandu
    resval =  ((tListItem *) listGetPointerLast(iList))->instr.op1;

    }
    else if (tkState == S_IDENT) { // prirazuje se navratova hodnota funkce:
      result = funcCall();
      if ( result != EOK ) return result;
      //tady gettoken misto toho zakomentovanyho pod tim, protoze expression ma na konci taky gettoken
      if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;

      //vysledek bude v @retval
      resval = gcMalloc(sizeof(TString));
      if (resval == NULL) return internalError();


      result = strInitDefault(resval,"@retval");
      if (result != EOK) return result;

    } else {
      return syntaxError();
    }


    result = igenerate(iList,I_ASSIGN,tmpString,resval,NULL);
    if( result != EOK ) return result;
    //printf("%s\n",resval->string); //prirazovana hodnota

    // ukonceni prikazu strednikem
    //if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();
    if ( tkState != S_SEMICOLON ) return syntaxError();

    break;
  }

  case KW_RET:{
    // mam nacteny return, potrebuju dalsi token -> tj. prvni token vyrazu
    if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();
    result = expression();
    if (result != EOK) return result;

    TString *resval;
    resval =  ((tListItem *) listGetPointerLast(iList))->instr.op1;

    result = igenerate(iList,I_RET,resval,NULL,NULL);
    if( result != EOK ) return result;

    break;
  }

  default:
    return syntaxError();
    break;
  }
  //return, assign ...
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;

return result;
}

int innerBlock(){
  int result = EOK;

  // pravidlo <inner-block> -> eps
  if (tkState == S_R_BRACE || tkState == S_EOF) return EOK;

  result = statement();
  if (result != EOK) return result;

  result = innerBlock();
  if (result != EOK) return result;

  // pravidlo <inner-block> -> <statement> <inner-block>
  //if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return E_LEX;
  return result;
}

int paramsNext(int *argc, TString *** argv) {
  // prvni token mi nacetl volajici!

  int result = EOK;

  //pravidlo <param-next> -> eps
  if (tkState == S_R_PARENTH) return EOK;

  // pravidlo <param-next> -> , var-id
  // zkontroluj carku
  if ( tkState != S_COMMA ) return syntaxError();

  // nacti dalsi + zkontroluj, zda je identifikator
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  if ( tkState != S_VAR_IDENT) return syntaxError();

  *argc += 1;
  *argv = gcRealloc(*argv, sizeof(TString*)*(*argc));
  if (*argv == NULL)
    return internalError();

  (*argv)[(*argc)-1] = gcMalloc(sizeof(TString));
  if ((*argv)[(*argc)-1] == NULL)
    return internalError();


  strinit((*argv)[(*argc)-1]);
  strTStringCpy((*argv)[(*argc)-1],&tkAttr);

  // kontrola redefinice parametru
  for(int i = (*argc)-2; i >= 0; --i ) {
    if ( strTStringCmp( (*argv)[(*argc)-1] ,  (*argv)[i] ) == 0 )
      return otherSemanticError("Redifinice parametru");
  }


  // nacti dalsi parametr a rekurzivne opakuj
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  result = paramsNext(argc,argv);

  return result;
}

int params(int *argc, TString *** argv) {
  // prvni token uz mam nacteny od volajiciho
  int result = EOK;

  //pravidlo <params> -> eps
  if (tkState == S_R_PARENTH) return EOK;

  // pravidlo <params> -> var-ident  <param-next>
  if ( tkState != S_VAR_IDENT ) return syntaxError();

  //pricteni argc precteni argv
  *argc += 1;
  *argv = gcMalloc(sizeof(TString*));
  if (*argv == NULL)
    return internalError();

  *(argv[0]) = gcMalloc(sizeof(TString));
  if (*(argv[0]) == NULL)
    return internalError();

  strinit(*(argv[0]));
  strTStringCpy(*(argv[0]),&tkAttr);

  //mam jeden formalni parametr
  // nacti dalsi a rozvin <param-next>
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  result = paramsNext(argc,argv);

  return result;
}

int function(){
  int result = EOK;

  // function id
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  if ( tkState != S_IDENT ) return syntaxError();

  // pravidlo <function> -> function id ( <params> ) {<st-list>}

  if ( isBuiltinFunc(&tkAttr))
    return funSemanticError();

  //generace FSKIP - uvozuje zacatek funkce,
  // interpret preskoci vsechny instrukce az po FSTOP
  result = igenerate(iList,I_FSKIP,NULL,NULL,NULL);
  if (result != EOK) return result;


  //kontrola jestli identifikator (funkce) uz je ci neni v tabulce fci
  htable_listitem * tmp = htable_find_item(funTable,&tkAttr);
  if (tmp == NULL){
    /* Definici teto funkce nepredchazi zadne jeji volani */
    TVar * func = gcMalloc(sizeof(TVar));
    if(func == NULL) return internalError();
    func->type = T_FUNC;  //tvar dostane typ funkce
    func->data.function.start = listGetPointerLast(iList); //start na FSKIP

    result = htable_insert(funTable,&tkAttr,func);
    if (result != EOK) return result;
  }
  else if(tmp->var->data.function.start == NULL){ //je tam ale miri na null
    /* Na funkci uz jsem nekdy narazil, ale ale az ted je definovana */
    tmp->var->data.function.start = listGetPointerLast(iList); //start na FSKIP
  }
  else{ //redefinice funkce
    /* funkce jiz byla jednou definovana */
    return funSemanticError();
  }

  //inicializace argc argv, do params se preda jejich adresa
  tmp = htable_find_item(funTable,&tkAttr);
  tmp->var->data.function.argc = 0;
  tmp->var->data.function.argv = NULL;


  // function id (
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  if ( tkState != S_L_PARENTH ) return syntaxError();


  // function id ( <params>
  if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX) return lexError();
  if ( (result = params(&(tmp->var->data.function.argc),&(tmp->var->data.function.argv)) ) != EOK ) return result;

  // volal jsem pod-pravidlo => token uz mam nacteny!
  // function id ( <params> )

  if ( tkState != S_R_PARENTH ) return syntaxError();

  // function id ( <params> ) {
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  if ( tkState != S_L_BRACE ) return syntaxError();

  // function id ( <params> ) { <inner-block>
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;
  result = innerBlock();
  if ( result != EOK ) return result;


  // function id ( <params> ) { <st-list> }
  // token byl nacten
  if ( tkState != S_R_BRACE ) return syntaxError();

  //generace FSTOP - indikuje konec funkce na instrukcni pasce,
  // odsud interpret pokracuje instrukci za instrukci

  //htable_print(funTable);

  result = igenerate(iList,I_RET,NULL,NULL,NULL);
  if( result != EOK ) return result;

  result = igenerate(iList,I_FSTOP,NULL,NULL,NULL);
  if (result != EOK) return result;

  // token nacten
  if ( (result=getToken(&tkAttr, &tkState, &tkLine )) != EOK ) return result;

  return result;
}

int programBody(){
  int result = EOK;
  // <program-body> -> eps
  if (tkState == S_EOF) return EOK;

  //pravidlo <program-body> -> <statement> <program-body>
  switch (tkState){
  case KW_IF:
  case KW_RET:
  case KW_WHILE:
  case KW_FOR:
  case KW_CONT:
  case KW_BREAK:
  case S_IDENT:
  case S_VAR_IDENT:
    result = statement();
    if (result != EOK) return result;
    result = programBody();
    if (result != EOK) return result;

    return EOK;
  break;

  //pravidlo <program-body> -> <function> <program-body>
  case KW_FUNC:
    result = function();
    if (result != EOK) return result;
    result = programBody();
    if (result != EOK) return result;

    return EOK;

    break;
  default:
    return syntaxError();
  }


}

int program(){
  int result = EOK;

  // pravidlo <program> -> <elem-list> !!! smazat
  result = programBody();

  if (tkState == S_EOF) {
      return result;
  }

  return result;
}



int parse(htable_t *fT, tIlist *iL,htableStack *sS){
  //nahrani do globalnich promennych
  funTable = fT;
  iList = iL;
  symbolStack = sS;

  // inicializujeme zasobnik cyklu
  loops = loopStack_init();
  if (loops == NULL) return internalError();

  //inicializace fronty argumentu funkci
  argQueue = TString_queue_init();

  // otestuj oteviraci znacku
  if ( ! hasValidOpenTag() )
      return syntaxError();

  int result;
   // = malloc(sizeof(TString));
  strinit(&tkAttr);

  if ( getToken(&tkAttr, &tkState, &tkLine ) == E_LEX)
     // nastala chyba jiz pri nacteni prvniho lexemu
     result = lexError();
  else
     result = program(); // uvodni terminal
  if( result != EOK ) return result;

  result = igenerate(iL,I_STOP,NULL,NULL,NULL);
  if( result != EOK ) return result;
  return result;
}
