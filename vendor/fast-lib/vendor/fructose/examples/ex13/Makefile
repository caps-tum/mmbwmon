all: ex13

ex13: 
	../../fructose_gen.py --generator g1.h g2.h g3.h >ex13.cpp
	g++ -o ex13 -I../../include ex13.cpp
	@diff g_expected.cpp ex13.cpp >/dev/null; if [ $$? -eq 0 ]; then echo "generator style passed";else echo "generator style failed"; fi

clean:
	rm -f ex13 ex13.cpp
