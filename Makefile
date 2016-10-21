CC=gcc
UNAME := $(shell uname)
CFLAGS= -Wall -Wextra -Werror -g $(shell sdl2-config --cflags) #-Wno-sign-compare
LDFLAGS= $(shell sdl2-config --libs) -lm

# OSX uses a different link option
ifeq ($(UNAME),Darwin)
	CFLAGS += -I/usr/local/include
	LDFLAGS += -framework OpenCL
else
	LDFLAGS += -l OpenCL
endif
	
OBJ= main.o clerror.o sdl.o opencl.o flame.o xform.o variation.o rng.o log.o
BIN=firegpu

all: $(OBJ) $(BIN)
	
$(BIN): $(OBJ) 
	$(CC) $(LDFLAGS) $(OBJ) -o $@

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(BIN)
