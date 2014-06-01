#
# ==========================================================================
# Projekt	  : Implementace interpretu imperativniho jazyka IFJ13
# Soubor      : main.c
# Varianta	  :
# Autori      : xkovar66 David Kovarik
#             : xkralb00 Benjamin Kral
#             : xholcn01 Jonas Holcner
#             : xsychr05 Marek Sychra
#             : xkaras27 Josef Karasek
#
# ==========================================================================
#
 
CC=gcc
CFLAGS=-std=c99 -Wall -pedantic -W -Wextra  -g

CDEST=ifj13

# mezerou oddeleny seznam objektovych souboru, ten je nasledne pouzit
# pro rozgenerovani zavislosti pro kazdou jednu polozku.

OBJFILES=main.o garbage.o str.o error.o ial.o builtins.o scanner.o expr.o parser.o ilist.o interpret.o


# vytvori "relativni" promennou pro kazdy .o | .c | .h soubor
vpath %.c src
vpath %.h head
vpath %.o obj

all: $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) -o $(CDEST)
   
build: clean $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) -o $(CDEST)


# rozgenerovani pro vsechny .o soubory
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

run: all
	./$(CDEST)	

testall: clean lextest parsetest outputtest
		

parsetest: $(OBJFILES) test/parsetest.o
	$(CC) $(CFLAGS) garbage.o str.o error.o ial.o builtins.o scanner.o parser.o expr.o test/parsetest.o -o test/parsetest

# smaze vsechny objektove soubory a binarku
clean: 
	rm -f $(OBJFILES) $(CDEST) $(CDEST).exe test/intertest.exe

LEXOBJ = garbage.o str.o error.o ial.o builtins.o scanner.o expr.o parser.o ilist.o  test/lextest.o
lextest: clean $(LEXOBJ)
	$(CC) $(CFLAGS) $(LEXOBJ) -o test/lextest
	@./test/lextest.sh
	
ILISTOBJ = garbage.o str.o error.o ial.o builtins.o scanner.o expr.o parser.o ilist.o test/ilisttest.o
ilisttest: clean $(ILISTOBJ)
	$(CC) $(CFLAGS) $(ILISTOBJ) -o test/ilisttest
  
INTERTOBJ = garbage.o str.o error.o ial.o builtins.o scanner.o expr.o parser.o ilist.o interpret.o test/interpret_test.o
intertest: $(INTERTOBJ)
	$(CC) $(CFLAGS) $(INTERTOBJ) -o test/intertest

outputtest: clean all
	./test/outputtest.sh

gendoc: 
	doxygen
	
	
VALID_SOURCES = main.c garbage.c str.c error.c ial.c builtins.c scanner.c expr.c parser.c ilist.c interpret.c
VALID_HEADERS = garbage.h str.h error.h ial.h builtins.h scanner.h expr.h parser.h ilist.h interpret.h
VALID_OTHERS = rozdeleni rozsireni Makefile dokumentace.pdf
VALID_OUTPUT = xkovar66

pack: clean 
	(cd doc/text && make;cp dokumentace.pdf ../../)
	zip $(VALID_OUTPUT) $(VALID_SOURCES) $(VALID_HEADERS) $(VALID_OTHERS)
	
packtest: 
	@make clean > /dev/null
	@make pack > /dev/null
	@mkdir packtest 
	@mv $(VALID_OUTPUT).zip packtest 
	@cp is_it_ok.sh packtest 
	
	
	@cd packtest && \
	  mkdir tmp; \
	  bash is_it_ok.sh $(VALID_OUTPUT).zip tmp;
	  
	@rm -rf packtest
	


#udela jonas exe pro jonase protoze nechce pouzivat daviduv main  
JONASOBJFILES=jonas.o garbage.o str.o error.o ial.o builtins.o scanner.o
jonas: $(JONASOBJFILES)
	$(CC) $(CFLAGS) $(JONASOBJFILES) -o jonas
jonasclean: 
	rm -f $(JONASOBJFILES) jonas.exe
  
