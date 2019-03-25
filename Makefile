CC = gcc

INCLUDE_DIRS = inc
CFLAGS = -Wall -funroll-loops -std=c11 -O3 $(addprefix "-I", $(INCLUDE_DIRS))

SRC = src/options.c \
			src/main.c
OBJ = $(SRC:.c=.o)

all: enigmax

enigmax: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	$(RM) $(OBJ) enigmax
