HEADERS=graph.h bfs.h
SOURCES=program.cpp graph.cpp bfs.cpp
FILES=$(HEADERS) $(SOURCES)

all: $(FILES)
	./bin/clang++ -std=c++17 -O2 -fopencilk -DUSE_CILK_PLUS_RUNTIME $(SOURCES) -o program
	# ./bin/clang++ -std=c++17 -O2 -fcilkplus -DUSE_CILK_PLUS_RUNTIME $(SOURCES) -pthread -o program
	# ./bin/clang++ -std=c++17 -O2 -fopencilk -DUSE_CILK_RUNTIME $(SOURCES) -o program

run: all
	CILK_NWORKERS=4 ./program

