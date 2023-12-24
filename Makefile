.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)

.PHONY: all clean scour

all: nut.exe digest.exe ingred.dat

nut.exe: nut.cpp Nutrition.h
	g++ -I $(INCL) -std=c++20 nut.cpp -o nut.exe

digest.exe: digest.cpp Nutrition.h
	g++ -I $(INCL) -std=c++20 digest.cpp -o digest.exe

ingred.dat: digest.exe ingred.txt
	./digest

clean:

scour: clean
	rm -f nut.exe digest.exe ingred.dat
