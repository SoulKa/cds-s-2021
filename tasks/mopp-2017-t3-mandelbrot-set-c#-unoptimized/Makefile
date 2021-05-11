RM=rm -f
EXEC=mandelbrot

all: $(EXEC)

$(EXEC):
	dotnet publish -c Release

debug:
	dotnet publish

run:
	dotnet run

clean:
	$(RM) -rf obj && $(RM) -rf bin