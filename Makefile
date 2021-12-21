all:
	g++ -static -std=c++17 -O2 src/anicons.cpp -o anicons

clean:
	rm -f anicons