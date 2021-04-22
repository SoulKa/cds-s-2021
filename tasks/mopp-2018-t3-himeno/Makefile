CXXFLAGS=-O0 -Wall -pthread
RM=rm -f
EXEC=himeno

all: $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) $(EXEC).cpp -o $(EXEC)

run:
	./$(EXEC)

clean:
	$(RM) $(EXEC)
