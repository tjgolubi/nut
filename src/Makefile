.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)
STD=c++23
BIN=$(abspath $(HOME)/bin)

OPT=

.PHONY: all clean scour

all: nut.exe digest.exe barf.exe lookup.exe

nut.exe: nut.cpp Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) nut.cpp -o $@

digest.exe: digest.cpp Atwater.cpp Atwater.h To.h Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) digest.cpp Atwater.cpp Nutrition.cpp -o $@

barf.exe: barf.cpp Nutrition.cpp Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) barf.cpp Nutrition.cpp -o $@

lookup.exe: lookup.cpp Atwater.cpp Atwater.h To.h
	g++ -I $(INCL) -std=$(STD) $(OPT) lookup.cpp Atwater.cpp -o $@

clean:

scour: clean
	rm -f nut.exe digest.exe barf.exe lookup.exe

install: all findfood.bash
	ln --verbose --force --symbolic $(PWD)/nut.exe       $(BIN)/nut
	ln --verbose --force --symbolic $(PWD)/digest.exe    $(BIN)/digest
	ln --verbose --force --symbolic $(PWD)/barf.exe      $(BIN)/barf
	ln --verbose --force --symbolic $(PWD)/lookup.exe    $(BIN)/lookup
	ln --verbose --force --symbolic $(PWD)/findfood.bash $(BIN)/findfood

uninstall:
	rm -f $(BIN)/nut $(BIN)/digest $(BIN)/barf $(BIN)/lookup $(BIN)/findfood
