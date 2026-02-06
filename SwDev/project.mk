# Copyright 2025 Terry Golubiewski.  All rights reserved.

VERSION := 0.0.0
#DEBUG ?= 1
USE_DYNAMIC_RTL := true

ifndef PROJDIR
$(error PROJDIR is not defined)
endif

SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c

EXT:=$(PROJDIR)/ext
UNITS:=$(EXT)/mp-units/src
GSL:=$(EXT)/GSL

COMPILER ?= gcc
PLATFORM := Windows

ifndef BOOST
export BOOST:=/usr/include
export BOOSTLIB:=/usr/lib
endif

ifndef BOOSTLIB
export BOOSTLIB:=$(BOOST)/lib
endif

ifndef TJG
export TJG:=$(PROJDIR)/tjg
endif

ifdef DEBUG
DBGSFX := D
else
DBGSFX :=
endif

PATH_OBJ := obj_$(COMPILER)$(DBGSFX)
CLEAN := obj_* *.stackdump *.exe.manifest
