CXXFLAGS=-O0 -Wall
RM=rm -f
EXEC=himeno

all: $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) $(EXEC).cpp -c -o $(EXEC).o
	$(CXX) $(CXXFLAGS) $(EXEC).o -o $(EXEC)

run:
	./$(EXEC)

clean:
	$(RM) $(EXEC).o $(EXEC)
