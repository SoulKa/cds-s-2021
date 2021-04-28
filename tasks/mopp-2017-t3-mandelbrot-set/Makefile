CXXFLAGS=-O3 -std=c++17 -Wall -pthread
RM=rm -f
EXEC=mandelbrot

all: $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) $(EXEC).cpp -o $(EXEC)

run:
	cat $(EXEC).in | ./$(EXEC) 

profile:
	$(CXX) $(CXXFLAGS) -pg $(EXEC).cpp -o $(EXEC)

timing:
	$(CXX) $(CXXFLAGS) -D MEASURE_TIME $(EXEC).cpp -o $(EXEC)

clean:
	$(RM) $(EXEC)
