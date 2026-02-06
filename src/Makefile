# Copyright 2025 Terry Golubiewski, all rights reserved.

export PROJDIR := $(abspath ..)
SWDEV := $(PROJDIR)/SwDev
include $(PROJDIR)/SwDev/project.mk

LIBDIR:=$(PROJDIR)/lib

TARGET1=$(LIBDIR)/libNut$(COMPILER)$(DBGSFX).$A
TARGET2=nut$(DBGSFX).$E
TARGET3=digest$(DBGSFX).$E
TARGET4=barf$(DBGSFX).$E
TARGET5=lookup$(DBGSFX).$E
TARGETS=$(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5)

SRC1 := Nutrition.cpp Atwater.cpp Nutrition.cpp Parse.cpp
SRC2 := nut.cpp
SRC3 := digest.cpp
SRC4 := barf.cpp
SRC5 := lookup.cpp
SOURCE := $(SRC1) $(SRC2) $(SRC3) $(SRC4) $(SRC5)

SYSINCL:=$(addsuffix /include, $(UNITS)/core $(UNITS)/systems $(GSL))
INCLUDE:=$(PROJDIR)

LIBS=$(TARGET1)

include $(SWDEV)/$(COMPILER).mk
include $(SWDEV)/build.mk

BIN=$(abspath $(HOME)/bin)

SCOUR:=$(LIBDIR)

.ONESHELL:

.PHONY: all clean scour install uninstall

all: $(TARGETS)

$(LIBDIR):
	mkdir -p $@

$(TARGET1): $(OBJ1) | $(LIBDIR)

$(TARGET2): $(OBJ2) $(LIBS)
        $(LINK)

$(TARGET3): $(OBJ3) $(LIBS)
        $(LINK)

$(TARGET4): $(OBJ4) $(LIBS)
        $(LINK)

$(TARGET5): $(OBJ5) $(LIBS)
        $(LINK)

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
