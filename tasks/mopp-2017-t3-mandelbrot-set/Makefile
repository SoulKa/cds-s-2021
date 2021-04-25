CXXFLAGS=-O3 -std=c++11 -Wall -pthread
RM=rm -f
EXEC=mandelbrot

all: $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) $(EXEC).cpp -o $(EXEC)

timing:
	$(CXX) $(CXXFLAGS) -D MEASURE_TIMING $(EXEC).cpp -o $(EXEC)

run:
	cat judge.in | ./$(EXEC) >test.out 2>/dev/null

profile:
	$(CXX) $(CXXFLAGS) -pg $(EXEC).cpp -o $(EXEC)

clean:
	$(RM) $(EXEC).o $(EXEC)
