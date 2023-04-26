# TARGET = sfml_app
# CC = g++
# FLAGS = sfml-app -lsfml-graphics -lsfml-window -lsfml-system

# $(TARGET) : main.o config.hpp
# 	$(CC) -c main.cpp
# 	$(CC) main.o -o $(FLAGS)
# main.o: config.hpp 
# clean : 
# 	rm *.o

TARGET = RayTracing

CC = g++

DFLAGS = -g -ggdb3 -Wall -Werror -Wpedantic -Wextra -fsanitize=undefined,address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

PREF_SRC = ./src/
PREF_OBJ = ./obj/

SRC = $(wildcard $(PREF_SRC)*.cpp)
OBJ = $(patsubst $(PREF_SRC)%.cpp, $(PREF_OBJ)%.o, $(SRC))

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) $(FLAGS) $(DFLAGS) -o  $(TARGET)

$(PREF_OBJ)%.o : $(PREF_SRC)%.cpp
	$(CC) -c $< -o $@

clean : 
	rm $(TARGET) $(PREF_OBJ)*.o 
