# Shameless copy paste from tj/luna project
# https://github.com/tj/luna/blob/master/Makefile

SRC = $(wildcard *.cpp)
OBJ = ${SRC:.cpp=.o}
OUT = verify

# Fallback to gcc if clang not available
CPP = g++

#OPT=-O3
OPT=-O0 -g -fsanitize=bounds -fsanitize=address -fsanitize=undefined

$(OUT): $(OBJ)
	@printf "Linking %s...\n" $@
	@printf "Done!\n"
	@$(CPP) --std=c++17 $(OPT) -march=native $^ -o $@

%.o: %.cpp
	@printf "Compiling %s...\n" $@
	@$(CPP) --std=c++17 $(OPT) -march=native -c $< -o $@

clean:
	@rm -f $(OUT) $(OBJ)
	@printf "All clear!\n"
