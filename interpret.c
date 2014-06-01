/*
 ============================================================================
 Projekt     : Implementace interpretu imperativniho jazyka IFJ13
 Soubor      : interpret.c - Modul pro interpretaci seznamu 3AC
 Varianta    : a/3/II
 Autori      : xkovar66 David Kovarik
             : xholcn01 Jonas Holcner
             : xsychr05 Marek Sychra
             : xkaras27 Josef Karasek
             : xkralb00 Benjamin Kral

 ============================================================================
 */

#include "interpret.h"
#include "builtins.h"
#include "str.h"

int getOperands(tInstr * tmp, htableStack * TSstack, TVar ** op1, TVar ** op2, TVar ** op3, int numberOfOps){

    htable_listitem * tmpitem;


    if((tmp->op1) != NULL){             //konvertuju na tvar  --- jestli se naslo testuju az u isntrukci,
      if(((TString *)(tmp->op1))->string[0] == '#')
        tmpitem = htable_find_item(TSstack->tables[0], tmp->op1);                  //kde to je fakt potreba
      else
        tmpitem = htable_find_item(TSstack->tables[TSstack->top], tmp->op1);                  //kde to je fakt potreba
      if(tmpitem != NULL)
        *op1 = tmpitem->var;
      else
        return undeclarVarError(tmp->op1);
      }
    else
      return internalError();

    if((tmp->op2) != NULL){             //konvertuju na tvar  --- jestli se naslo testuju az u isntrukci,
      if(((TString *)(tmp->op2))->string[0] == '#')
        tmpitem = htable_find_item(TSstack->tables[0], tmp->op2);                  //kde to je fakt potreba
      else
        tmpitem = htable_find_item(TSstack->tables[TSstack->top], tmp->op2);                  //kde to je fakt potreba
      if(tmpitem != NULL)
        *op2 = tmpitem->var;
      else
        return undeclarVarError(tmp->op2);
      }
    else
      return internalError();

    if( numberOfOps == 3){

      if((tmp->op3) != NULL){             //konvertuju na tvar  --- jestli se naslo testuju az u isntrukci,
        if(((TString *)(tmp->op3))->string[0] == '#')
          tmpitem = htable_find_item(TSstack->tables[0], tmp->op3);                  //kde to je fakt potreba
        else
          tmpitem = htable_find_item(TSstack->tables[TSstack->top], tmp->op3);                  //kde to je fakt potreba
        if(tmpitem != NULL)
          *op3 = tmpitem->var;
        else
          return undeclarVarError(tmp->op3);
        }
      else
        return internalError();
    }
    return EOK;


}


int interpret(tIlist * ilist, htable_t * funTable, htableStack * TSstack){


  tInstr * instr;

  listFirst(ilist);

  TStringQueue * argQueue = TString_queue_init();
  if (argQueue == NULL) return internalError();

  while(1){
    instr = listGetData(ilist);         //vytahnu instrukci
    if(instr == NULL)
      return internalError();

    TVar * op1 = NULL;
    TVar * op2 = NULL;
    TVar * op3 = NULL;

    TVar * tmpvar;


    //printInstr(instr);

    int result;

    switch(instr->type){

      /********* ARITMETICKE OPERACE **********/

      case I_ADD:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;
        if(op2->type == T_INT && op3->type == T_INT){
          op1->type = T_INT;
          op1->data.integer = op2->data.integer + op3->data.integer;
          }
        else if(op2->type == T_INT && op3->type == T_REAL){
          op1->type = T_REAL;
          op1->data.real = op2->data.integer + op3->data.real;
          }
        else if(op2->type == T_REAL && op3->type == T_INT){
          op1->type = T_REAL;
          op1->data.real = op2->data.real + op3->data.integer;
          }
        else if(op2->type == T_REAL && op3->type == T_REAL){
          op1->type = T_REAL;
          op1->data.real = op2->data.real + op3->data.real;
          }
        else
          return incompatibleTypesError();
        break;

      case I_SUB:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;
        if(op2->type == T_INT && op3->type == T_INT){
          op1->type = T_INT;
          op1->data.integer = op2->data.integer - op3->data.integer;
          }
        else if(op2->type == T_INT && op3->type == T_REAL){
          op1->type = T_REAL;
          op1->data.real = op2->data.integer - op3->data.real;
          }
        else if(op2->type == T_REAL && op3->type == T_INT){
          op1->type = T_REAL;
          op1->data.real = op2->data.real - op3->data.integer;
          }
        else if(op2->type == T_REAL && op3->type == T_REAL){
          op1->type = T_REAL;
          op1->data.real = op2->data.real - op3->data.real;
          }
        else
          return incompatibleTypesError();
        break;

      case I_MUL:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;
        if(op2->type == T_INT && op3->type == T_INT){
          op1->type = T_INT;
          op1->data.integer = op2->data.integer * op3->data.integer;
          }
        else if(op2->type == T_INT && op3->type == T_REAL){
          op1->type = T_REAL;
          op1->data.real = op2->data.integer * op3->data.real;
          }
        else if(op2->type == T_REAL && op3->type == T_INT){
          op1->type = T_REAL;
          op1->data.real = op2->data.real * op3->data.integer;
          }
        else if(op2->type == T_REAL && op3->type == T_REAL){
          op1->type = T_REAL;
          op1->data.real = op2->data.real * op3->data.real;
          }
        else
          return incompatibleTypesError();
        break;

      case I_DIV:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;
        if(op2->type == T_INT && op3->type == T_INT){
          if(op3->data.integer == 0)
            return zeroDivError();
          op1->type = T_REAL;
          op2 = doubleval(op2, &result);     //pretypuju jedno na real
          if(result != EOK) return result;
          op1->data.real = op2->data.real / op3->data.integer;
          }
        else if(op2->type == T_INT && op3->type == T_REAL){
          if(op3->data.real == 0.0)
            return zeroDivError();
          op1->type = T_REAL;
          op1->data.real = op2->data.integer / op3->data.real;
          }
        else if(op2->type == T_REAL && op3->type == T_INT){
          if(op3->data.integer == 0)
            return zeroDivError();
          op1->type = T_REAL;
          op1->data.real = op2->data.real / op3->data.integer;
          }
        else if(op2->type == T_REAL && op3->type == T_REAL){
          if(op3->data.real == 0)
            return zeroDivError();
          op1->type = T_REAL;
          op1->data.real = op2->data.real / op3->data.real;
          }
        else
          return incompatibleTypesError();
        break;

      case I_NEG:{
        if ((result = (getOperands(instr,TSstack,&op1, &op2, &op3, 2))) != EOK)
          return result;

      if (op2->type == T_INT){
        op1->data.integer = -1*op2->data.integer;
        op1->type = T_INT;
      }
      else if (op2->type == T_REAL){
        op1->data.real = -1*op2->data.real;
        op1->type = T_REAL;
      }
      else
        return incompatibleTypesError();


        break;
      }


     /********** RETEZCOVE OPERACE **************/

      case I_CONCAT:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;

        if(op2->type != T_STR)
          return incompatibleTypesError();

        op3 = strval(op3,&result);
        if (result != EOK) return result;

        if((result = strTStringCat(&(op1->data.str), op2->data.str, op3->data.str)) != EOK)
          return result;
          
        op1->type = T_STR;  
        break;


      /********* POROVNAVACI OPERACE **********/

      case I_EQ:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;        
        if(op2->type != op3->type)
          op1->data.boolean = false;
        else if(op2->type == T_INT)
          op1->data.boolean = (op2->data.integer == op3->data.integer);
        else if(op2->type == T_REAL)
          op1->data.boolean = (op2->data.real == op3->data.real);
        else if(op2->type == T_BOOL)
          op1->data.boolean = (op2->data.boolean == op3->data.boolean);
        else if(op2->type == T_STR)
          op1->data.boolean = (!strTStringCmp(op2->data.str,op3->data.str));
        else if(op2->type == T_NULL)
          op1->data.boolean = (op2->data.null == op3->data.null);  
        else
          op1->data.boolean = false;
        op1->type = T_BOOL;  
        break;

      case I_NEQ:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;        
        if(op2->type != op3->type)
          op1->data.boolean = true;
        else if(op2->type == T_INT)
          op1->data.boolean = !(op2->data.integer == op3->data.integer);
        else if(op2->type == T_REAL)
          op1->data.boolean = !(op2->data.real == op3->data.real);
        else if(op2->type == T_BOOL)
          op1->data.boolean = !(op2->data.boolean == op3->data.boolean);
        else if(op2->type == T_STR)
          op1->data.boolean = (strTStringCmp(op2->data.str,op3->data.str));
        else if(op2->type == T_NULL)
          op1->data.boolean = !(op2->data.null == op3->data.null);  
        else
          op1->data.boolean = true;
        op1->type = T_BOOL;  
        break;

      case I_LE:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;        
        if(op2->type != op3->type)
          return incompatibleTypesError();
        else if(op2->type != T_NULL){
          if(op2->type == T_INT)
            op1->data.boolean = (op2->data.integer < op3->data.integer);
          else if(op2->type == T_REAL)
            op1->data.boolean = (op2->data.real < op3->data.real);
          else if(op2->type == T_BOOL)
            op1->data.boolean = (op2->data.boolean < op3->data.boolean);
          else if(op2->type == T_STR){
            if(strTStringCmp(op2->data.str,op3->data.str) < 0 )    //kdyz to vrati zapornou hodnotu, prvni je mensi
              op1->data.boolean = true;
            else
              op1->data.boolean = false;
            }          
        }
        else
          return incompatibleTypesError();
        op1->type = T_BOOL;    
        break;

      case I_LEQ:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;        
        if(op2->type != op3->type)
          return incompatibleTypesError();
        else if(op2->type != T_NULL){
          if(op2->type == T_INT)
            op1->data.boolean = (op2->data.integer <= op3->data.integer);
          else if(op2->type == T_REAL)
            op1->data.boolean = (op2->data.real <= op3->data.real);
          else if(op2->type == T_BOOL)
            op1->data.boolean = (op2->data.boolean <= op3->data.boolean);
          else if(op2->type == T_STR){
            if(strTStringCmp(op2->data.str,op3->data.str) <= 0 )    //kdyz to vrati zapornou hodnotu, prvni je mensi
              op1->data.boolean = true;
            else
              op1->data.boolean = false;
            }           
        }
        else
          return incompatibleTypesError();
        op1->type = T_BOOL;   
        break;

      case I_GE:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;        
        if(op2->type != op3->type)
          return incompatibleTypesError();
        else if(op2->type != T_NULL){
          if(op2->type == T_INT)
            op1->data.boolean = (op2->data.integer > op3->data.integer);
          else if(op2->type == T_REAL)
            op1->data.boolean = (op2->data.real > op3->data.real);
          else if(op2->type == T_BOOL)
            op1->data.boolean = (op2->data.boolean > op3->data.boolean);
          else if(op2->type == T_STR){
            if(strTStringCmp(op2->data.str,op3->data.str) > 0 )    //kdyz to vrati zapornou hodnotu, prvni je mensi
              op1->data.boolean = true;
            else
              op1->data.boolean = false;
            }        
        }
        else
          return incompatibleTypesError();
        op1->type = T_BOOL;      
        break;

      case I_GEQ:
        if((result = (getOperands(instr, TSstack, &op1, &op2, &op3, 3))) != EOK)
          return result;        
        if(op2->type != op3->type)
          return incompatibleTypesError();
        else if(op2->type != T_NULL){
          if(op2->type == T_INT)
            op1->data.boolean = (op2->data.integer >= op3->data.integer);
          else if(op2->type == T_REAL)
            op1->data.boolean = (op2->data.real >= op3->data.real);
          else if(op2->type == T_BOOL)
            op1->data.boolean = (op2->data.boolean >= op3->data.boolean);
          else if(op2->type == T_STR){
            if(strTStringCmp(op2->data.str,op3->data.str) >= 0 )    //kdyz to vrati zapornou hodnotu, prvni je mensi
              op1->data.boolean = true;
            else
              op1->data.boolean = false;
            }            
        }
        else
          return incompatibleTypesError();
        op1->type = T_BOOL;  
        break;


      /********* LOGICKE OPERACE **********/

      case I_NOT:
        if((result = (getOperands(instr, TSstack, &op1, &op2, NULL, 2))) != EOK)
          return result;
        tmpvar = boolval(op2, &result);
        if (result != EOK) return result;
        op1->data.boolean = !(tmpvar->data.boolean);   // ? false : true;
        op1->type = T_BOOL;
        break;


      /********* INSTRUKCE PRIRAZENI **********/

      case I_ASSIGN:
        if((instr->op1) == NULL)        // kdyz je prvni operand null
          return internalError();

        htable_listitem * tmpitem;
        if((instr->op2) != NULL){          //ziskani druehhho operandu
          if(((TString *)(instr->op2))->string[0] == '#')   //v nejnizsi tabulce
            tmpitem = htable_find_item(TSstack->tables[0], instr->op2);
          else
            tmpitem = htable_find_item(TSstack->tables[TSstack->top], instr->op2);
          if(tmpitem != NULL)           //kdyz nebyla nalezena v tabulce
            op2 = tmpitem->var;
          else
            return undeclarVarError(instr->op2);
          }
        else
          return internalError();

        tmpvar = gcMalloc(sizeof(TVar));
        if( tmpvar == NULL)
          return internalError();
        tmpvar->data = op2->data;  //cool, sem neveril ze bude fungovat
        tmpvar->type = op2->type;


        if(((TString *)(instr->op1))->string[0] == '#')
          result = htable_overwrite(TSstack->tables[0], instr->op1, tmpvar );
        else
          result = htable_overwrite(TSstack->tables[TSstack->top], instr->op1,tmpvar);

        if (result != EOK) return result;

        break;


      /********* INSTRUKCE SKOKU **********/

      case I_JMP:
        listGoto(ilist, instr->op1);
        break;

      case I_IFJUMP:{
        if((instr->op2) != NULL){          //ziskani druehhho operandu
          if(((TString *)(instr->op2))->string[0] == '#')   //v nejnizsi tabulce
            tmpitem = htable_find_item(TSstack->tables[0], instr->op2);
          else
            tmpitem = htable_find_item(TSstack->tables[TSstack->top], instr->op2);
          if(tmpitem != NULL)           //kdyz nebyla nalezena v tabulce
            op2 = tmpitem->var;
          else
            return undeclarVarError(instr->op2);
          }
        else
          return internalError();

        tmpvar = boolval(op2, &result);     //prevedu na bool aby to bylo jednotny
        if(tmpvar == NULL)
          return result;

        if(tmpvar->data.boolean)
          listGoto(ilist, instr->op1);
        break;
        }


      /********* POKOUTNE INSTRUKCE SKAKANI FCI **********/

      case I_FSKIP:
        while(instr->type != I_FSTOP){
          listNext(ilist);
          instr = listGetData(ilist);
          if(instr == NULL)
            return internalError();
          }
        break;

      case I_FSTOP:
        printf ("i shouldn't have really been able to get here...\n");
        break;

      case I_LABEL:
        break;

      /********* PRAZDNA INSTRUKCE **********/

      case I_NOP:
        break;

      /********* INSTRUKCE PRO ARGUMENTY *****/
      case I_PUSHPAR:
        result = TString_queue_put(argQueue,((TString **)&(instr->op1)));
        if (result != EOK) return result;
        break;



      /********* KONCOVA INSTRUKCE **********/

      case I_STOP:
        return EOK;


     /*****************************************/
     /*********** Vestavene funkce ************/

      case I_BOOLVAL:
      case I_DOUBLEVAL:
      case I_INTVAL:
      case I_STRVAL:
      {
        htable_listitem *tmpitem;

        if (!TString_queue_empty(argQueue)){
          if(TString_queue_top(argQueue)->string[0] == '#')
            tmpitem = htable_find_item(TSstack->tables[0], TString_queue_top(argQueue));
          else
            tmpitem = htable_find_item(TSstack->tables[TSstack->top], TString_queue_top(argQueue));
          if(tmpitem != NULL)
            tmpvar = tmpitem->var;
          else
            return undeclarVarError(TString_queue_top(argQueue));


          ///vyhozeni veci z fronty kdyby bylo vic argumentu
          TString_queue_set_empty(argQueue);
        }
        else{            //error podle typu
          switch(instr->type){
            case I_BOOLVAL:
              return argumentError("boolval");
            case I_DOUBLEVAL:
              return argumentError("doubleval");
            case I_INTVAL:
              return argumentError("intval");
            case I_STRVAL:
              return argumentError("strval");
            default:
              return internalError();

          }
        }

        TString *retval = gcMalloc(sizeof(TString));
        if (retval == NULL) return internalError();


        result = strInitDefault(retval,"@retval");
        if (result != EOK) return result;

        TVar * var;

        //provedeni skutecne odpovidajici funkce
        switch(instr->type){
            case I_BOOLVAL:
              var = boolval(tmpvar,&result);
              break;
            case I_DOUBLEVAL:
              var = doubleval(tmpvar,&result);
              break;
            case I_INTVAL:
              var = intval(tmpvar,&result);
              break;
            case I_STRVAL:
              var = strval(tmpvar,&result);
              break;
            default:
              return internalError();
          }
        if (var == NULL) return result;

        result = htable_overwrite(htable_stack_top(TSstack),retval,var);
        if (result != EOK) return result;


        break;
      }





      /*********** Stringove funkce ************/

      case I_PUTSTR:{
        int count = 0;
        htable_listitem *tmpitem;

        while (!TString_queue_empty(argQueue)){  //dokud jsou na stacku argumenty
          if(TString_queue_top(argQueue)->string[0] == '#')
            tmpitem = htable_find_item(TSstack->tables[0], TString_queue_top(argQueue));
          else
            tmpitem = htable_find_item(TSstack->tables[TSstack->top], TString_queue_top(argQueue));
          if(tmpitem != NULL)
            tmpvar = tmpitem->var;
          else
            return undeclarVarError(TString_queue_top(argQueue));


          tmpvar = strval(tmpvar, &result); //provede se strval
          if(tmpvar == NULL)
            return result;
          printf("%s",tmpvar->data.str->string);  //vypsani
          count += 1;
          TString_queue_pop(argQueue);
        }
        //alokace navratove hodnoty a TVaru do ktereho se ulozi
        TString *retval = gcMalloc(sizeof(TString));
        if (retval == NULL) return internalError();


        result = strInitDefault(retval,"@retval");
        if (result != EOK) return result;

        TVar * var = gcMalloc(sizeof(TVar));
        if (var == NULL) return internalError();

        var->type = T_INT;
        var->data.integer = count;

        result = htable_overwrite(htable_stack_top(TSstack),retval,var);
        if (result != EOK) return result;

        break;
      }

      case I_GETSTR:{
        TString *line = gcMalloc(sizeof(TString));
        if (line == NULL) return internalError();

        result = strinit(line);
        if (result != EOK) return result;

        int c;
        c = getchar();

        while (c != '\n' && c != EOF){
          result = strCharAppend(line,c);
          if (result != EOK) return result;
          c = getchar();
        }

        TString *retval = gcMalloc(sizeof(TString));
        if (retval == NULL) return internalError();

        result = strInitDefault(retval,"@retval");
        if (result != EOK) return result;

        TVar * var = gcMalloc(sizeof(TVar));
        if (var == NULL) return internalError();

        var->type = T_STR;
        var->data.str = line;

        result = htable_overwrite(htable_stack_top(TSstack),retval,var);
        if (result != EOK) return result;

        ///vyhozeni veci z fronty kdyby bylo vic argumentu
        TString_queue_set_empty(argQueue);

        break;
      }

      case I_STRLEN:{
        htable_listitem *tmpitem;

        if (!TString_queue_empty(argQueue)){
          if(TString_queue_top(argQueue)->string[0] == '#')
            tmpitem = htable_find_item(TSstack->tables[0], TString_queue_top(argQueue));
          else
            tmpitem = htable_find_item(TSstack->tables[TSstack->top], TString_queue_top(argQueue));
          if(tmpitem != NULL)
            tmpvar = tmpitem->var;
          else
            return undeclarVarError(TString_queue_top(argQueue));

          tmpvar = strval(tmpvar, &result);
          if(tmpvar == NULL)
            return result;

          ///vyhozeni veci z fronty kdyby bylo vic argumentu
          TString_queue_set_empty(argQueue);
        }
        else
          return argumentError("strlen");

        TString *retval = gcMalloc(sizeof(TString));
        if (retval == NULL) return internalError();


        result = strInitDefault(retval,"@retval");
        if (result != EOK) return result;

        TVar * var = gcMalloc(sizeof(TVar));
        if (var == NULL) return internalError();

        var->type = T_INT;
        var->data.integer = strlen(tmpvar->data.str->string);   //volani strlen

        result = htable_overwrite(htable_stack_top(TSstack),retval,var);
        if (result != EOK) return result;

        break;
      }

      case I_GET_SUBSTR:{
        htable_listitem *tmpitem;


        TVar * start;       //hranice vyrezu
        TVar * end;


        if (!TString_queue_empty(argQueue)){          //pokdu tam nic neni - > chyba
          if(TString_queue_top(argQueue)->string[0] == '#')
            tmpitem = htable_find_item(TSstack->tables[0], TString_queue_top(argQueue));
          else
            tmpitem = htable_find_item(TSstack->tables[TSstack->top], TString_queue_top(argQueue));
          if(tmpitem != NULL)
            tmpvar = tmpitem->var;
          else
            return undeclarVarError(TString_queue_top(argQueue));

          tmpvar = strval(tmpvar, &result);     // prvni je string
          if(tmpvar == NULL)
            return result;

          TString_queue_pop(argQueue);          //popnua jdu pro dalsi

          if (!TString_queue_empty(argQueue)){    //druhy parametr
            if(TString_queue_top(argQueue)->string[0] == '#')
              tmpitem = htable_find_item(TSstack->tables[0], TString_queue_top(argQueue));
            else
              tmpitem = htable_find_item(TSstack->tables[TSstack->top], TString_queue_top(argQueue));
            if(tmpitem != NULL)
              start = tmpitem->var;
            else
              return undeclarVarError(TString_queue_top(argQueue));

            start = intval(start, &result);     //druhej je integer start
            if(start == NULL)
              return result;


            TString_queue_pop(argQueue);

            if (!TString_queue_empty(argQueue)){    //treti parametr
              if(TString_queue_top(argQueue)->string[0] == '#')
                tmpitem = htable_find_item(TSstack->tables[0], TString_queue_top(argQueue));
              else
                tmpitem = htable_find_item(TSstack->tables[TSstack->top], TString_queue_top(argQueue));
              if(tmpitem != NULL)
                end = tmpitem->var;
              else
                return undeclarVarError(TString_queue_top(argQueue));

              end = intval(end, &result);        //treti je integer end
              if(end == NULL)
                return result;
            }
            else
              return argumentError("get_substring");
          }
          else
            return argumentError("get_substring");
        }
        else
          return argumentError("get_substring");

        TString_queue_set_empty(argQueue);   ///vyhozeni veci z fronty kdyby bylo vic argumentu



        TString *retval = gcMalloc(sizeof(TString));
        if (retval == NULL) return internalError();


        result = strInitDefault(retval,"@retval");   //pripraveni retvalu
        if (result != EOK) return result;

        TVar * var = gcMalloc(sizeof(TVar));
        if (var == NULL) return internalError();

        var->type = T_STR;
        if( (result = get_substring(var, tmpvar, start->data.integer, end->data.integer)) != EOK )   //volani get_substring
          return result;

        result = htable_overwrite(htable_stack_top(TSstack),retval,var);
        if (result != EOK) return result;

        break;
      }

      case I_FIND_STR:{
        htable_listitem *tmpitem;

        TVar * tmpvar2;       // tmpvar = haystack .... tmpvar2 = needle
        int position = -1;    //pozice ve slove


        if (!TString_queue_empty(argQueue)){
          if(TString_queue_top(argQueue)->string[0] == '#')
            tmpitem = htable_find_item(TSstack->tables[0], TString_queue_top(argQueue));
          else
            tmpitem = htable_find_item(TSstack->tables[TSstack->top], TString_queue_top(argQueue));
          if(tmpitem != NULL)
            tmpvar = tmpitem->var;
          else
            return undeclarVarError(TString_queue_top(argQueue));

          tmpvar = strval(tmpvar, &result);
          if(tmpvar == NULL)
            return result;

          TString_queue_pop(argQueue);

          if (!TString_queue_empty(argQueue)){    //dva parametry
            if(TString_queue_top(argQueue)->string[0] == '#')
              tmpitem = htable_find_item(TSstack->tables[0], TString_queue_top(argQueue));
            else
              tmpitem = htable_find_item(TSstack->tables[TSstack->top], TString_queue_top(argQueue));
            if(tmpitem != NULL)
              tmpvar2 = tmpitem->var;
            else
              return undeclarVarError(TString_queue_top(argQueue));

            tmpvar2 = strval(tmpvar2, &result);
            if(tmpvar2 == NULL)
              return result;
            }
          else
            return argumentError("find_string");
        }
        else
          return argumentError("find_string");
                  ///vyhozeni veci z fronty kdyby bylo vic argumentu
        TString_queue_set_empty(argQueue);



        TString *retval = gcMalloc(sizeof(TString));
        if (retval == NULL) return internalError();


        result = strInitDefault(retval,"@retval");
        if (result != EOK) return result;

        TVar * var = gcMalloc(sizeof(TVar));
        if (var == NULL) return internalError();

        var->type = T_INT;
        if( (result = find_string(tmpvar->data.str, tmpvar2->data.str, &position)) != EOK )   //volani sort_string
          return result;
        var->data.integer = position;

        result = htable_overwrite(htable_stack_top(TSstack),retval,var);
        if (result != EOK) return result;

        break;
      }


      case I_SORT_STR:{
        htable_listitem *tmpitem;

        if (!TString_queue_empty(argQueue)){
          if(TString_queue_top(argQueue)->string[0] == '#')
            tmpitem = htable_find_item(TSstack->tables[0], TString_queue_top(argQueue));
          else
            tmpitem = htable_find_item(TSstack->tables[TSstack->top], TString_queue_top(argQueue));
          if(tmpitem != NULL)
            tmpvar = tmpitem->var;
          else
            return undeclarVarError(TString_queue_top(argQueue));

          tmpvar = strval(tmpvar, &result);
          if(tmpvar == NULL)
            return result;

          ///vyhozeni veci z fronty kdyby bylo vic argumentu
          TString_queue_set_empty(argQueue);
        }
        else
          return argumentError("sort_string");

        TString *retval = gcMalloc(sizeof(TString));
        if (retval == NULL) return internalError();


        result = strInitDefault(retval,"@retval");
        if (result != EOK) return result;

        TVar * var = gcMalloc(sizeof(TVar));
        if (var == NULL) return internalError();

        var->type = T_STR;
        if( (result = sort_string(var, tmpvar)) != EOK )   //volani sort_string
          return result;

        result = htable_overwrite(htable_stack_top(TSstack),retval,var);
        if (result != EOK) return result;

        break;
      }

      case I_CALL:{
        //najdu funkci ve funtablu
        htable_listitem * funcItem = htable_find_item(funTable,((TString *)(instr->op1)));
        //priradim TVar do funcVar
        TVar *funcVar = funcItem->var;

        //pokud je definovana
        if ( funcVar->data.function.start!= NULL){
          //nova TS
          htable_t *table = htable_init(HTABLE_SIZE);
          if (table == NULL)
            return internalError();
          //nahraju novou TS jako aktualni
          result = htable_stack_push(TSstack,table);
          if (result != EOK) return result;

          //ulozim si kolik argumentu funkce ocekava
          int argc = funcVar->data.function.argc;

          for (int i = 0; i < argc; i++){
             //kdyz je malo parametru -> chyba
            if (TString_queue_empty(argQueue))
              return argumentError(funcItem->name->string);

            //vyhledam promennou v minule TS a ulozim do stavajici
            //promena nebo konstranta
            htable_listitem * tmp;
            if (TString_queue_top(argQueue)->string[0] == '#')
              tmp = htable_find_item(TSstack->tables[0],TString_queue_top(argQueue));
            else
              tmp = htable_find_item(TSstack->tables[TSstack->top-1],TString_queue_top(argQueue));

            // musela byt zadana nedeklarovana promenna
            if (tmp == NULL)
              return undeclarVarError(TString_queue_top(argQueue));

            //zkopiruju obsah
            //nejdriv tstring
            TString *tmpstr = gcMalloc(sizeof(TString));
            if (tmpstr == NULL) return internalError();
            result = strinit(tmpstr);
            if (result != EOK) return result;
            result = strTStringCpy(tmpstr,funcItem->var->data.function.argv[i]);
            if (result != EOK) return result;
            //printf("cislo %d\n",TSstack->top);
            //htable_print(TSstack->tables[TSstack->top-1]);
           // htable_print(TSstack->tables[0]);
            //printf("%s %s\n",TString_queue_top(argQueue)->string,funcItem->var->data.function.argv[i]->string);

            //pak TVar
            TVar *tmpvar = gcMalloc(sizeof(TVar));
            if (tmpvar == NULL)
              return internalError();
            tmpvar->type = tmp->var->type;
            tmpvar->data = tmp->var->data;


            result = htable_overwrite(htable_stack_top(TSstack),tmpstr,tmpvar);
            if (result != EOK) return result;

            TString_queue_pop(argQueue);
          }

          ///vyhozeni veci z fronty kdyby bylo vic argumentu
          TString_queue_set_empty(argQueue);



          //vytvorim implicitni return val ktery je NULL kdyz v kodu neni return
          TString *retval = gcMalloc(sizeof(TString));
          if (retval == NULL) return internalError();

          result = strInitDefault(retval,"@retval");
          if (result != EOK) return result;

          //a jeho tvar
          TVar *retVar = gcMalloc(sizeof(TVar));
          if (retVar == NULL) return internalError();
          retVar->type = T_NULL;
          retVar->data.null=NULL;

          //a nahraju ho do aktualni TS
          result = htable_overwrite(htable_stack_top(TSstack),retval,retVar);
          if (result != EOK) return result;

          //vytvorim si ukazatel na hodnotu zpatky, pro return
          TString * ip = gcMalloc(sizeof(TString));
          if (ip == NULL) return internalError();

          result = strInitDefault(ip,"@ip");
          if (result != EOK) return result;

          TVar *ipVar = gcMalloc(sizeof(TVar));
          if (ipVar == NULL) return internalError();
          //nahraje se adresa dalsi instrukce
          ipVar->data.null = ((tListItem*)ilist->active);


          //pushnu do aktualni TS
          result = htable_overwrite(htable_stack_top(TSstack),ip,ipVar);
          if (result != EOK) return result;
          //skoci na zacatek funkce a dole si listNext nacte dalsi instrukci
          listGoto(ilist,funcVar->data.function.start);

        }
        else{ //nedefinovana funkce
          return funSemanticError();
        }
        break;
      }

      case I_RET:{
        //to co vracim NULL nebo promenna
        TString * ret = ((TString *)instr->op1);

        //udelam si a najdu navratovou adresu ip
        TString * ipTstring = gcMalloc(sizeof(TString));
        if (ipTstring == NULL) return internalError();

        result = strInitDefault(ipTstring,"@ip");
        if (result != EOK) return result;




        //pripravim si navratovou hodnotu
        TString * retvalTString = gcMalloc(sizeof(TString));
        if (retvalTString == NULL) return internalError();

        result = strInitDefault(retvalTString,"@retval");
        if (result != EOK) return result;

        TVar * retVar;
        htable_listitem * retval;
        retVar = gcMalloc(sizeof(TVar));
        if (retVar == NULL) return internalError();

        ///pokud prazdny, je konec vseho!! vrati ok (jsme v mainu)
        if (htable_stack_nearly_empty(TSstack))
          return EOK;
        else{
          //kdyz neni navratova hodnota
          if (ret == NULL){
            retVar->type = T_NULL;
            retVar->data.null=NULL;
          }
          else{
            //kdyz je, tak ji vyhledam a priradim

            //promena nebo konstranta
            if (ret->string[0] == '#')
              retval = htable_find_item(TSstack->tables[0],ret);
            else
              retval = htable_find_item(htable_stack_top(TSstack),ret);

            if(retval == NULL)
              return internalError();

            retVar->type = retval->var->type;
            retVar->data = retval->var->data;

          }
          htable_listitem *ip = htable_find_item(htable_stack_top(TSstack),ipTstring);
          if(ip == NULL)
            return internalError();

          htable_stack_pop(TSstack);

          result = htable_overwrite(htable_stack_top(TSstack),retvalTString,retVar);
          if (result != EOK) return result;

          retval = htable_find_item(htable_stack_top(TSstack),retvalTString);

          //skok
          listGoto(ilist,ip->var->data.null);
          break;

        }


      }

      default:
        break;

    }

    listNext(ilist);           //nacteni dalsi isntrukce

  }

  return EOK;
}
