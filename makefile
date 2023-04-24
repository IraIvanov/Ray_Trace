TARGET = sfml_app
CC = g++
FLAGS = sfml-app -lsfml-graphics -lsfml-window -lsfml-system

$(TARGET) : main.o config.hpp
	$(CC) -c main.cpp
	$(CC) main.o -o $(FLAGS)
main.o: config.hpp 
clean : 
	rm *.o