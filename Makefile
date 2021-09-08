CFLAGS=-O2 -Wall

SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

OUTPUT=serial_passthrough

all: clean $(OUTPUT)
$(OUTPUT): $(OBJ)
	$(CC) -o $@ $(OBJ) 

$(OBJ): %.o :%.c
	$(CC) -c $(CFLAGS) -o "$@" "$<"

clean:
	rm -rf $(OBJ) $(OUTPUT) *.o

allclean: clean