all: ex6

ex6.cpp: ex6.h
	../../fructose_gen.py ex6.h >ex6.cpp

ex6: ex6.h ex6.cpp
	g++ -o ex6 -I../../include ex6.cpp
