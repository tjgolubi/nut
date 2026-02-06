# Copyright 2025 Terry Golubiewski, all rights reserved.

export PROJDIR := $(abspath .)
SWDEV := $(PROJDIR)/SwDev
include $(PROJDIR)/SwDev/project.mk

SUBDIRS:=src usda
TARGETS:=all clean scour

.ONESHELL:

.PHONY: subdirs $(SUBDIRS) $(TARGETS)

$(TARGETS): subdirs
	@echo "Making top $@"

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(filter $(TARGETS), $(MAKECMDGOALS))

%::
	@echo "Making top $@"
