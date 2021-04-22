CXXFLAGS=-O3 -Wall -pthread
RM=rm -f
EXEC=himeno

all: $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) $(EXEC).cpp -o $(EXEC)

run:
	cat $(EXEC).in | ./$(EXEC) 

profile:
	$(CXX) $(CXXFLAGS) -pg $(EXEC).cpp -o $(EXEC)

clean:
	$(RM) $(EXEC)
