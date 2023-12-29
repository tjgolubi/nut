.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)

.PHONY: all clean scour

all: nut.exe digest.exe ingred.dat barf.exe barf.txt

nut.exe: nut.cpp Nutrition.h
	g++ -I $(INCL) -std=c++20 nut.cpp -o nut.exe

digest.exe: digest.cpp Nutrition.h
	g++ -I $(INCL) -std=c++20 digest.cpp -o digest.exe

barf.exe: barf.cpp Nutrition.h
	g++ -I $(INCL) -std=c++20 barf.cpp -o barf.exe

ingred.dat: digest.exe ingred.txt defs.txt chicken.txt turkey.txt $(wildcard branded/*.txt)
	./digest.exe

barf.txt: barf.exe ingred.dat
	./barf.exe > barf.txt

clean:
	rm -f ingred.dat barf.txt

scour: clean
	rm -f nut.exe digest.exe barf.exe
