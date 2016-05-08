all: ex12

ex12: 
	../../fructose_gen.py x1.h x2.h x3.h >ex12.cpp
	g++ -o ex12 -I../../include -I../../../tclap-1.2.0/include ex12.cpp
	@diff x_expected.cpp ex12.cpp >/dev/null; if [ $$? -eq 0 ]; then echo "xUnit style passed";else echo "xUnit style failed"; fi

clean:
	rm -f ex12 ex12.cpp
