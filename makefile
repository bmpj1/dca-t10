#GameOfLife Makefile
# Para ejecutar con otros parametros que no sean estos desde linea de comandos es con:
# make run M=(Tamaño de la Matriz MxM) I=(Iteraciones)
PROG = GameOfLife
CC = gcc
PARAM = -O3 -std=c++11 -lstdc++ -fopenmp
M = 100
I = 1

all : $(PROG)

$(PROG) : $(PROG)
	$(CC) -o $(PROG) $(PROG).cpp $(PARAM)

run : ./$(PROG)
	./$(PROG) $(M) $(I)

clean : 
	-rm $(PROG)
