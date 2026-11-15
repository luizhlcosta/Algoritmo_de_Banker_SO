
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = banker

all: $(TARGET)

$(TARGET): banker.c
	$(CC) $(CFLAGS) banker.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
