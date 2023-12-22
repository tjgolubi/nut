.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)

all: nut.exe digest.exe

nut.exe: nut.cpp
	g++ -I $(INCL) -std=c++20 nut.cpp -o nut.exe

digest.exe: digest.cpp
	g++ -I $(INCL) -std=c++20 digest.cpp -o digest.exe
clean:

scour: clean
	rm nut.exe digest.exe
