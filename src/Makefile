.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)
STD=c++23

EXE=$(abspath ..)

OPT=

.PHONY: all clean scour

all: $(EXE)/nut.exe $(EXE)/digest.exe $(EXE)/barf.exe $(EXE)/lookup.exe

$(EXE)/nut.exe: nut.cpp Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) nut.cpp -o $@

$(EXE)/digest.exe: digest.cpp Atwater.cpp Atwater.h To.h Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) digest.cpp Atwater.cpp Nutrition.cpp -o $@

$(EXE)/barf.exe: barf.cpp Nutrition.cpp Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) barf.cpp Nutrition.cpp -o $@

$(EXE)/lookup.exe: lookup.cpp Atwater.cpp Atwater.h To.h
	g++ -I $(INCL) -std=$(STD) $(OPT) lookup.cpp Atwater.cpp -o $@

clean:

scour: clean
	rm -f $(EXE)/nut.exe $(EXE)/digest.exe $(EXE)/barf.exe $(EXE)/lookup.exe
