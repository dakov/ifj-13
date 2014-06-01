/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : str.c - Modul pro praci s datovy type TString
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */
#include "str.h"
#include "error.h"

                       
#define BUFFINC 16

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
int strinit(TString *str) {

  if ( (str->string = (char*) gcMalloc(BUFFINC)) == NULL)  {
    return internalError();
  }

  str->string[0] = '\0'; //ukonci retezec
  str->length = 0; // nulova delka
  str->allocated = BUFFINC; // realne alokovan 1 BUFFINC blok

  return EOK;
}

/**
 * Provede inicializaci typu TString zapouzdrujici
 * retezce. Alokuje datovy atribut `string` na
 * velikost BUFFINC, delka retezce `length` je nastavena
 * na 0.
 * A načte do něj řetězec z druhého parametru
 *
 * @param str Ukazatel na inicializovanou strukturu
 * @param s Řetězec který se zkopíruje
 *
 * @return Konstantu STROK, nebo STRERR pri chybe
 */
int strInitDefault(TString * str, char * s){
  int result;

  result = strinit(str);
  if (result != EOK) return result;

  for (unsigned i = 0; i < strlen(s); i++){
    result = strCharAppend(str,s[i]);
    if (result != EOK) return result;
  }

  return EOK;
}

/**
 * Prida jeden znak na konec retezce. Pokud nestaci
 * jit alokovana pamet, pokusi se o jeji rozsireni.
 *
 * @param str Retezec typu TString, k nemuz se ma znak pridat
 * @param c Znak, ktery se ma pridat na konec retezce
 *
 * @return EOK nenastala chyba, jinak STRERR
 */
int strCharAppend(TString *str, char c) {

  int absLen = str->length + 1; //absolutni delka (vcetne \0)

  // pokud by se dalsi znak nevlezl do alokovane pameti
  // +1 kvuli '\0'
  if (absLen >= str->allocated ) {
    // malo alokovane pameti -> pokus se ji rozsirit
    // alokuji se chary ( = 1B ), netreba pouzivat sizeof
    str->string = gcRealloc( str->string, absLen + BUFFINC );

    if(str->string == NULL) { //nepovedlo se
      return internalError();
    }
    // aktualizuj informaci o velikost alokovane pameti
    str->allocated = absLen + BUFFINC;

  }
  str->string[str->length++] = c; //pridej znak
  str->string[str->length] = '\0'; //posun konec retezce

  return EOK;

}

/**
 * Resetuje atributy rezezce. Pokud je jiz alokovana pamet
 * vetsi nez byla pri inicializaci, NEBUDE velikost aktualne
 * alokovane snizena.
 *
 * Nastavi delku retezce na 0 a znak '\0' umistni na index 0.
 */
void strReset(TString *str) {
  str->string[0] = '\0';
  str->length = 0;
}

/**
 * Porovna retezce zapouzdrene v strukturach typu TString.
 * Zachovava stejne chovani jako standardni funkce strcmp.
 *
 * @return viz manualove stranky strcmp
 */
int strTStringCmp(const TString *str1, const TString *str2) {
  return strcmp(str1->string, str2->string);
}

/**
 * Porovna retezec zapouzdreny v strukture typu TString s normalnim ceckovskym.
 * Zachovava stejne chovani jako standardni funkce strcmp.
 *
 * @return viz manualove stranky strcmp
 */
int strConstTStringCmp(TString *str1, char* str2)
{
   return strcmp(str1->string, str2);
}


/**
 * Kopiruje retezec zapouzdrene v strukturach typu TString.
 * Podle standartni funkce strcpy je prvni parametr cil a druhy zdroj.
 *
 * @param str Retezec typu TString do ktereho se ma kopirovat.
 * @param str Retezec typu TString ze ktereho se ma kopirovat.
 *
 * @return Konstrantu EOK nebo STRERR pri chybe
 */
int strTStringCpy(TString *destination, const TString *source){
  if (destination == NULL || source == NULL)
    return internalError();

  int length = source->length;

  if(length + 1 >= destination->allocated){
    //pokud delka retezce i s '\0' prekracuje naalokovany prostor
    if((destination->string=(char*)gcRealloc(destination->string,length + 1))==NULL)
      return internalError();
  }
  strcpy(destination->string,source->string);

  destination->length = length;
  destination->allocated = length+1;

  return EOK;
}

/**
 * Vrati delku retezce v str
 * Podle standartni funkce strcpy je prvni parametr cil a druhy zdroj.
 *
 * @param str Retezec typu TString
 *
 * @return delka retezece
 */
int strTStringLen(TString *str){
  return str->length;
}


/**
 * Konkatenace retezcu dest = str1.str2 
 *
 * @param str Retezec typu TString do ktereho vleze vysledek.
 * @param str Retezec typu TString .. prvni cast  
 * @param str Retezec typu TString .. druha cast
 *
 * @return Konstrantu STROK nebo STRERR pri chybe
 */
int strTStringCat(TString ** destination, TString * source1, TString * source2){
  if ( source1 == NULL || source2 == NULL)
    return internalError();

  *destination = gcMalloc(sizeof(TString));
  strinit(*destination);

  int length = source1->length + source2->length;

  if(length + 1 >= (*destination)->allocated){ //tohle je uplne zbytecny, ne?
    //pokud delka retezce i s '\0' prekracuje naalokovany prostor
    if(((*destination)->string=(char*)gcRealloc((*destination)->string,length + 1))==NULL)
      return internalError();
  }  


  (*destination)->length = length;
  (*destination)->allocated = length+1;

  strcpy((*destination)->string,source1->string);
  strcat((*destination)->string,source2->string);


  return EOK;

}


