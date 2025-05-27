CC = gcc
CFLAGS = -Iinclude -Wall
SRC = src/main.c src/shell.c
OBJ = $(SRC:.c=.o)
TARGET = mysh

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
