.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)

OPT=

.PHONY: all clean scour

all: nut.exe digest.exe ingred.dat barf.txt lookup.exe usda_foods.tsv usda_portions.tsv food.txt

nut.exe: nut.cpp Nutrition.h
	g++ -I $(INCL) -std=c++23 $(OPT) nut.cpp -o nut.exe

digest.exe: digest.cpp Atwater.cpp Atwater.h To.h Nutrition.h
	g++ -I $(INCL) -std=c++23 $(OPT) digest.cpp Atwater.cpp Nutrition.cpp -o digest.exe

barf.exe: barf.cpp Nutrition.cpp Nutrition.h
	g++ -I $(INCL) -std=c++23 $(OPT) barf.cpp Nutrition.cpp -o barf.exe

ingred.dat: digest.exe ingred.txt defs.txt chicken.txt turkey.txt $(wildcard branded/*.txt)
	./digest.exe

barf.txt: barf.exe ingred.dat
	./barf.exe > barf.txt
	if test -f tjg.txt
	then diff -b tjg.txt barf.txt
	fi

lookup.exe: lookup.cpp Atwater.cpp Atwater.h To.h
	g++ -I $(INCL) -std=c++23 $(OPT) lookup.cpp Atwater.cpp -o lookup.exe

tabulate.exe: tabulate.cpp Atwater.cpp Atwater.h To.h
	g++ -I $(INCL) -std=c++23 $(OPT) tabulate.cpp Atwater.cpp -o tabulate.exe

usda_foods.tsv usda_portions.tsv: tabulate.exe
	./tabulate.exe

food.txt: mkfood.awk usda_foods.tsv
	awk -f mkfood.awk usda_foods.tsv > food.txt

clean:
	rm -f ingred.dat barf.txt

scour: clean
	rm -f nut.exe digest.exe barf.exe lookup.exe tabulate.exe food.txt tjg.txt
