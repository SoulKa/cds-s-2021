CXXFLAGS=-O0 -Wall
RM=rm -f
EXEC=himeno

all: $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) $(EXEC).cpp matrix.cpp -o $(EXEC)

run:
	./$(EXEC)

clean:
	$(RM) $(EXEC)
