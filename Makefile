CC = g++
NAME = t2-mi
OBJECTS = t2-mi.cpp

tsniv2ni: $(OBJECTS)
	$(CC) -o $(NAME) $(OBJECTS)
