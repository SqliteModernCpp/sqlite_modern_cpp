FLAGS=-Wall -Werror
LIBS=-lpthread -ldl
all: sqlite3.o
	        g++ -std=c++11 $(FLAGS) sqlite3.o src/test.cpp $(LIBS)  

sqlite3.o:
	        gcc $(FLAGS) sqlite3.c -c
