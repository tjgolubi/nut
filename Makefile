.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)

OPT=

.PHONY: all clean scour

all: nut.exe digest.exe ingred.dat barf.txt # lookout.txt food.txt

nut.exe: nut.cpp Nutrition.h
	g++ -I $(INCL) -std=c++20 $(OPT) nut.cpp -o nut.exe

digest.exe: digest.cpp Atwater.cpp Atwater.h Nutrition.h
	g++ -I $(INCL) -std=c++20 $(OPT) digest.cpp Atwater.cpp -o digest.exe

barf.exe: barf.cpp Nutrition.cpp Nutrition.h
	g++ -I $(INCL) -std=c++20 $(OPT) barf.cpp Nutrition.cpp -o barf.exe

ingred.dat: digest.exe ingred.txt defs.txt chicken.txt turkey.txt $(wildcard branded/*.txt)
	./digest.exe

barf.txt: barf.exe ingred.dat
	./barf.exe > barf.txt
	if test -f tjg.txt
	then diff -b tjg.txt barf.txt
	fi

lookup.exe: lookup.cpp
	g++ -I $(INCL) -std=c++20 $(OPT) lookup.cpp -o lookup.exe

lookout.txt: lookup.exe lookup.txt
	./lookup.exe

getfood.exe: getfood.cpp
	g++ -I $(INCL) -std=c++20 $(OPT) getfood.cpp -o getfood.exe

food.txt: getfood.exe ../usda/fdc/food.csv
	./getfood.exe

clean:
	rm -f ingred.dat barf.txt lookout.txt

scour: clean
	rm -f nut.exe digest.exe barf.exe lookup.exe getfood.exe food.txt tjg.txt
