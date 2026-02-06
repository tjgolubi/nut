# Copyright 2025 Terry Golubiewski, all rights reserved.

export PROJDIR := $(abspath ..)
SWDEV := $(PROJDIR)/SwDev
include $(SWDEV)/project.mk

DB=$(abspath ../db)
SRC=$(abspath ../src)

TABULATE=tabulate$(DBGSFX).$E
CSVTOTSV=CsvToTsv$(DBGSFX).$E
TXTTOTSV=TxtToTsv$(DBGSFX).$E

TARGET1=$(DB)/food.txt
TARGET2=$(DB)/usda_foods.tsv
TARGET3=$(DB)/usda_portions.tsv
TARGET4=$(TABULATE)
TARGET5=$(CSVTOTSV)
TARGET6=$(TXTTOTSV)
TARGETS=$(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6)

DB_TGTS=$(TARGET1) $(TARGET2) $(TARGET3)

SRC4 := tabulate.cpp
SRC5 := CsvToTsv.cpp
SRC6 := TxtToTsv.cpp
SOURCE := $(SRC4) $(SRC5) $(SRC6)

SYSINCL:=$(addsuffix /include, $(UNITS)/core $(UNITS)/systems $(GSL))
INCLUDE:=$(PROJDIR)

LIBS=
LIBS+=$(PROJDIR)/lib/libNut$(COMPILER)$(DBGSFX).$A

include $(SWDEV)/$(COMPILER).mk
include $(SWDEV)/build.mk

.ONESHELL:

.PHONY: all clean scour

SCOUR=zip

all: $(DB_TGTS)

$(LIBS): ;

$(TARGET4): $(OBJ4) $(LIBS)
        $(LINK)

$(TARGET5): $(OBJ5) $(LIBS)
        $(LINK)

$(TARGET6): $(OBJ6) $(LIBS)
        $(LINK)

zip/fdc.zip:
	@set -euo pipefail
	set -x
	mkdir -p zip
	fdc_file=$$(curl -sL https://fdc.nal.usda.gov/download-datasets \
	  | grep -o 'href="/fdc-datasets/FoodData_Central_csv_[0-9-]*\.zip"' \
	  | sed -e 's|^href="/fdc-datasets/||;s|"$$||' \
	  | sort -u \
	  | tail -1)
	curl -o $@ "https://fdc.nal.usda.gov/fdc-datasets/$$fdc_file"

zip/sr.zip:
	@set -euo pipefail
	set -x
	mkdir -p zip
	curl -o $@ https://www.ars.usda.gov/ARSUserFiles/80400535/DATA/SR/sr28/dnload/sr28asc.zip

FDC=food food_calorie_conversion_factor food_nutrient food_nutrient_conversion_factor food_portion measure_unit sr_legacy_food
FDC_CSV=$(addsuffix .csv, $(FDC))
FDC_TSV=$(addsuffix .tsv, $(FDC))

SR=FOOD_DES
SR_TXT=$(addsuffix .txt, $(SR))
SR_TSV=$(addsuffix .tsv, $(SR))

TSV=$(FDC_TSV) $(SR_TSV)

$(addprefix zip/, $(FDC_CSV)): zip/fdc.zip
	unzip -u -aa -DD -j -d zip zip/fdc.zip $(addprefix */, $(FDC_CSV))

zip/FOOD_DES.txt: zip/sr.zip
	unzip -u -aa -DD -j -d zip zip/sr.zip FOOD_DES.txt

$(addprefix zip/,$(FDC_TSV)): $(CSVTOTSV)

$(addprefix zip/,$(SR_TSV)): $(TXTTOTSV)

zip/food.tsv: zip/food.csv
	sed -f fdc_cleanup.sed zip/food.csv | ./$(CSVTOTSV) > $@

zip/food_portion.tsv: zip/food_portion.csv
	sed -f fdc_cleanup.sed $< | ./$(CSVTOTSV) > $@

zip/%.tsv: zip/%.csv
	./$(CSVTOTSV) < $< > $@

zip/%.tsv: zip/%.txt
	./$(TXTTOTSV) < $< > $@

.PHONY: generate-db

$(DB_TGTS): $(addprefix zip/, $(TSV))

$(TARGET3): $(TABULATE)
	mkdir -p ../db
	./$(TABULATE)
