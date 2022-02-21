# Shameless copy paste from tj/luna project
# https://github.com/tj/luna/blob/master/Makefile

SRC = $(wildcard *.cpp)
OBJ = ${SRC:.cpp=.o}

# Fallback to gcc if clang not available
CPP = g++

OPT=-O3 -DNDEBUG
#OPT=-O0 -g -fsanitize=bounds -fsanitize=address -fsanitize=undefined -pg -DENABLE_DEBUG

all: verify5 verify10 verify25 verify50

verify5: $(SRC)
	@printf "Compiling 1/4...\n"
	@$(CPP) --std=c++17 $(OPT) -march=native -DNR_TASKS=5 $(SRC) -o verify5
	@printf "  verify5: done!\n"

verify10: $(SRC)
	@printf "Compiling 2/4...\n"
	@$(CPP) --std=c++17 $(OPT) -march=native -DNR_TASKS=10 $(SRC) -o verify10
	@printf "  verify10: done!\n"

verify25: $(SRC)
	@printf "Compiling 3/4...\n"
	@$(CPP) --std=c++17 $(OPT) -march=native -DNR_TASKS=25 $(SRC) -o verify25
	@printf "  verify25: done!\n"

verify50: $(SRC)
	@printf "Compiling 4/4...\n"
	@$(CPP) --std=c++17 $(OPT) -march=native -DNR_TASKS=50 $(SRC) -o verify50
	@printf "  verify50: done!\n"

clean:
	@rm -f verify5 verify10 verify25 verify50 $(OBJ)
	@printf "All clear!\n"
