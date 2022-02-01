# Shameless copy paste from tj/luna project
# https://github.com/tj/luna/blob/master/Makefile

SRC = $(wildcard *.cpp)
OBJ = ${SRC:.cpp=.o}
OUT = verify

# Fallback to gcc if clang not available
CPP = g++

$(OUT): $(OBJ)
	@printf "Linking %s...\n" $@
	@printf "Done!\n"
	@$(CPP) $^ -o $@

%.o: %.cpp
	@printf "Compiling %s...\n" $@
	@$(CPP) -c $< -o $@

clean:
	@rm -f $(OUT) $(OBJ)
	@printf "All clear!\n"
