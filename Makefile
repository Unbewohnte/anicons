all:
	g++ -static -std=c++17 -O2 src/anicons.cpp -o anicons

install: all
	mv anicons /usr/local/bin

clean:
	rm -f anicons