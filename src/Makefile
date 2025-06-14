.ONESHELL:

APP=$(abspath $(HOME)/App)
GSL=$(APP)/GSL
UNITS=$(APP)/mp-units/src

INCL=$(addsuffix /include, $(UNITS)/core $(UNITS)/systems $(GSL))

BIN=$(abspath $(HOME)/bin)

GPP=g++ -std=c++23 $(addprefix -I, $(INCL)) -o $@

.PHONY: all clean scour install uninstall

all: nut.exe digest.exe barf.exe lookup.exe

nut.exe: nut.cpp Nutrition.h
	$(GPP) nut.cpp

digest.exe: digest.cpp Atwater.cpp Atwater.h To.h Nutrition.h
	$(GPP) digest.cpp Atwater.cpp Nutrition.cpp

barf.exe: barf.cpp Nutrition.cpp Nutrition.h
	$(GPP) barf.cpp Nutrition.cpp

lookup.exe: lookup.cpp Atwater.cpp Atwater.h To.h Parse.h
	$(GPP) lookup.cpp Atwater.cpp

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
