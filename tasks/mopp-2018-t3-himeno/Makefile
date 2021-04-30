CXXFLAGS=-O0 -std=c++11 -Wall -pthread
RM=rm -f
EXEC=himeno

all: $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) $(EXEC).cpp -o $(EXEC)

original:
	gcc -O3 -Wall $(EXEC)_original.c -o $(EXEC)

original-float64:
	gcc -O3 -Wall -D USE_FLOAT64 $(EXEC)_original.c -o $(EXEC)

float64:
	$(CXX) $(CXXFLAGS) -D USE_FLOAT64 $(EXEC).cpp -o $(EXEC)

run:
	cat $(EXEC).in | ./$(EXEC) 

profile:
	$(CXX) $(CXXFLAGS) -pg $(EXEC).cpp -o $(EXEC)

timing:
	$(CXX) $(CXXFLAGS) -D MEASURE_TIME $(EXEC).cpp -o $(EXEC)

clean:
	$(RM) $(EXEC)
