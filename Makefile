CC=gcc
UNAME := $(shell uname)
CPPFLAGS= -Wall -Wextra -Werror -g #-Wno-sign-compare
LDFLAGS= -lSDL2

# OSX uses a different link option
ifeq ($(UNAME),Darwin)
	LDFLAGS += -framework OpenCL
else
	LDFLAGS += -l OpenCL
endif
	
OBJ= main.o clerror.o sdl.o opencl.o log.o
BIN=firegpu

all: $(OBJ) $(BIN)
	
$(BIN): $(OBJ) 
	$(CC) $(LDFLAGS) $(OBJ) -o $@

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(BIN)
