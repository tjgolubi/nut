.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)
STD=c++23

OPT=

.PHONY: all clean scour

all: nut.exe digest.exe ingred.dat barf.txt lookup.exe usda_foods.tsv usda_portions.tsv food.txt CsvToTsv.exe TxtToTsv.exe fndds_tab.exe

nut.exe: nut.cpp Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) nut.cpp -o nut.exe

digest.exe: digest.cpp Atwater.cpp Atwater.h To.h Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) digest.cpp Atwater.cpp Nutrition.cpp -o digest.exe

barf.exe: barf.cpp Nutrition.cpp Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) barf.cpp Nutrition.cpp -o barf.exe

ingred.dat: digest.exe ingred.txt defs.txt chicken.txt turkey.txt $(wildcard branded/*.txt)
	./digest.exe

barf.txt: barf.exe ingred.dat
	./barf.exe > barf.txt
	if test -f tjg.txt
	then diff -b tjg.txt barf.txt
	fi

lookup.exe: lookup.cpp Atwater.cpp Atwater.h To.h
	g++ -I $(INCL) -std=$(STD) $(OPT) lookup.cpp Atwater.cpp -o lookup.exe

tabulate.exe: tabulate.cpp Atwater.cpp Atwater.h Parse.h Progress.h To.h
	g++ -I $(INCL) -std=$(STD) $(OPT) tabulate.cpp Atwater.cpp -o tabulate.exe

food.txt usda_foods.tsv usda_portions.tsv: tabulate.exe
	./tabulate.exe

CsvToTsv.exe: CsvToTsv.cpp Parse.cpp Parse.h Progress.h
	g++ -I $(INCL) -std=$(STD) $(OPT) CsvToTsv.cpp Parse.cpp -o CsvToTsv.exe

TxtToTsv.exe: TxtToTsv.cpp Parse.cpp Parse.h Progress.h
	g++ -I $(INCL) -std=$(STD) $(OPT) TxtToTsv.cpp Parse.cpp -o TxtToTsv.exe

fndds_tab.exe: fndds_tab.cpp Parse.h Progress.h To.h
	g++ -I $(INCL) -std=$(STD) $(OPT) fndds_tab.cpp -o fndds_tab.exe

clean:
	rm -f ingred.dat barf.txt

scour: clean
	rm -f nut.exe digest.exe barf.exe lookup.exe tabulate.exe food.txt tjg.txt CsvToTsv.exe TxtToTsv.exe fndds_tab.exe
