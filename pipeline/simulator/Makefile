CC = g++ $(LIB)
LIB = -std=c++11

PROGRAM = pipeline
CFILES = main.cpp simulator.cpp instruction.cpp pipestg.cpp
HFILES = simulator.h instruction.h pipestg.h pipereg.h
OFILES = main.o simulator.o instruction.o pipestg.o

$(PROGRAM): $(OFILES)
	$(CC) -o $(PROGRAM) $(OFILES)
main.o: main.cpp simulator.o
	$(CC) -g -c main.cpp
simulator.o: simulator.cpp simulator.h pipestg.o
	$(CC) -g -c simulator.cpp
instruction.o: instruction.cpp instruction.h
	$(CC) -g -c instruction.cpp
pipestg.o: pipestg.cpp pipestg.h pipereg.h instruction.o simulator.h
	$(CC) -g -c pipestg.cpp

clean:
	rm -f $(OFILES) $(PROGRAM)

