.ONESHELL:

INCL=$(abspath $(HOME)/App/GSL/include)

nut.exe: nut.cpp
	g++ -I $(INCL) -std=c++20 nut.cpp -o nut.exe

clean:

scour: clean
	rm nut.exe
