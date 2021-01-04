SRC = src/
UTILS = utils/
LIB = lib/
COMPILER = g++
FLAGS = -c -Wall

run: stonks
	./stonks

stonks: $(SRC)main.cpp $(UTILS)singleStock.cpp
	g++ -o stonks $(SRC)main.cpp $(UTILS)singleStock.cpp -I.

clean: clean
	rm $(LIB)*.o
	rm stonks


