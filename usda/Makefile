.ONESHELL:

DB=$(abspath ../db)
SRC=$(abspath ../src)

VPATH=$(SRC)
vpath %.tsv $(DB)
vpath food.txt $(DB)

INCL=$(abspath $(HOME)/App/GSL/include)
STD=c++23

OPT=

.PHONY: all clean scour

all: CsvToTsv.exe TxtToTsv.exe usda_foods.tsv usda_portions.tsv food.txt

tabulate.exe: tabulate.cpp $(SRC)/Atwater.cpp $(SRC)/Atwater.h $(SRC)/Parse.h $(SRC)/To.h
	g++ -I $(INCL) -std=$(STD) $(OPT) tabulate.cpp $(SRC)/Atwater.cpp -o tabulate.exe

food.txt usda_foods.tsv usda_portions.tsv: tabulate.exe
	./tabulate.exe

CsvToTsv.exe: CsvToTsv.cpp $(SRC)/Parse.cpp $(SRC)/Parse.h
	g++ -I $(INCL) -std=$(STD) $(OPT) CsvToTsv.cpp $(SRC)/Parse.cpp -o CsvToTsv.exe

TxtToTsv.exe: TxtToTsv.cpp Parse.cpp Parse.h
	g++ -I $(INCL) -std=$(STD) $(OPT) TxtToTsv.cpp $(SRC)/Parse.cpp -o TxtToTsv.exe

clean:

scour: clean
	rm -f tabulate.exe CsvToTsv.exe TxtToTsv.exe
