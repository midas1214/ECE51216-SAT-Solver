CXX := g++
CXXFLAGS := -O2 -std=c++17 -Wall -Wextra -pedantic

all: mySAT

mySAT: main.o dimacs.o sat_solver.o
	$(CXX) $(CXXFLAGS) -o mySAT main.o dimacs.o sat_solver.o

main.o: main.cpp dimacs.h sat_solver.h
	$(CXX) $(CXXFLAGS) -c main.cpp

dimacs.o: dimacs.cpp dimacs.h sat_solver.h
	$(CXX) $(CXXFLAGS) -c dimacs.cpp

sat_solver.o: sat_solver.cpp sat_solver.h
	$(CXX) $(CXXFLAGS) -c sat_solver.cpp

clean:
	rm -f *.o mySAT