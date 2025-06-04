.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)
STD=c++23
BIN=$(abspath $(HOME)/bin)

OPT=

.PHONY: all clean scour install uninstall

all: nut.exe digest.exe barf.exe lookup.exe

nut.exe: nut.cpp Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) nut.cpp -o $@

digest.exe: digest.cpp Atwater.cpp Atwater.h To.h Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) digest.cpp Atwater.cpp Nutrition.cpp -o $@

barf.exe: barf.cpp Nutrition.cpp Nutrition.h
	g++ -I $(INCL) -std=$(STD) $(OPT) barf.cpp Nutrition.cpp -o $@

lookup.exe: lookup.cpp Atwater.cpp Atwater.h To.h Parse.h
	g++ -I $(INCL) -std=$(STD) $(OPT) lookup.cpp Atwater.cpp -o $@

clean:

scour: clean
	rm -f nut.exe digest.exe barf.exe lookup.exe

$(BIN)/nut: nut.exe
	ln --verbose --force --symbolic $(PWD)/$< $@

$(BIN)/digest: digest.exe
	ln --verbose --force --symbolic $(PWD)/$< $@

$(BIN)/barf: barf.exe
	ln --verbose --force --symbolic $(PWD)/$< $@

$(BIN)/lookup: lookup.exe
	ln --verbose --force --symbolic $(PWD)/$< $@

$(BIN)/findfood: findfood.bash
	ln --verbose --force --symbolic $(PWD)/$< $@

install: $(BIN)/nut $(BIN)/digest $(BIN)/barf $(BIN)/lookup $(BIN)/findfood

uninstall:
	rm -f $(BIN)/nut $(BIN)/digest $(BIN)/barf $(BIN)/lookup $(BIN)/findfood
