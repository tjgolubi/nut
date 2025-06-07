.ONESHELL:

DB=$(abspath ../db)
SRC=$(abspath ../src)

VPATH=$(SRC)
vpath usda_foods.tsv $(DB)
vpath usda_portions.tsv $(DB)
vpath food.txt $(DB)

INCL=$(abspath $(HOME)/App/GSL/include)
STD=c++23

OPT=

.PHONY: all clean scour unzip

all: usda_foods.tsv usda_portions.tsv food.txt

tabulate.exe: tabulate.cpp $(SRC)/Atwater.cpp $(SRC)/Atwater.h $(SRC)/Parse.h $(SRC)/To.h
	g++ -I $(INCL) -std=$(STD) $(OPT) tabulate.cpp $(SRC)/Atwater.cpp -o tabulate.exe

CsvToTsv.exe: CsvToTsv.cpp $(SRC)/Parse.cpp $(SRC)/Parse.h
	g++ -I $(INCL) -std=$(STD) $(OPT) CsvToTsv.cpp $(SRC)/Parse.cpp -o CsvToTsv.exe

TxtToTsv.exe: TxtToTsv.cpp Parse.cpp Parse.h
	g++ -I $(INCL) -std=$(STD) $(OPT) TxtToTsv.cpp $(SRC)/Parse.cpp -o TxtToTsv.exe

clean:

scour: clean
	rm -fr tabulate.exe CsvToTsv.exe TxtToTsv.exe zip

zip/fdc.zip:
	mkdir -p zip
	curl -o $@ https://fdc.nal.usda.gov/fdc-datasets/FoodData_Central_csv_2025-04-24.zip

zip/sr.zip:
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

$(addprefix zip/,$(FDC_TSV)): CsvToTsv.exe

$(addprefix zip/,$(SR_TSV)): TxtToTsv.exe

zip/food.tsv: zip/food.csv
	sed -f fdc_cleanup.sed zip/food.csv | ./CsvToTsv.exe > $@

zip/food_portion.tsv: zip/food_portion.csv
	sed -f fdc_cleanup.sed $< | ./CsvToTsv.exe > $@

zip/%.tsv: zip/%.csv
	./CsvToTsv.exe < $< > $@

zip/%.tsv: zip/%.txt
	./TxtToTsv.exe < $< > $@

food.txt usda_foods.tsv usda_portions.tsv: tabulate.exe $(addprefix zip/, $(TSV))
	./tabulate.exe
