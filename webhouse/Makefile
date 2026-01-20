# Makefile for Webhouse Project

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lbcm2835 -lpthread -ljansson -lm

# Target executables
TARGET = webhouse
TEST_TARGET = test_hardware

# Object files for main webhouse application
MAIN_OBJS = main.o Webhouse.o handshake.o base64.o sha1.o

# Object files for test_hardware
TEST_OBJS = test_hardware.o Webhouse.o

# Default target - build both executables
all: $(TARGET) $(TEST_TARGET)

# Main webhouse application
$(TARGET): $(MAIN_OBJS)
	$(CC) -o $(TARGET) $(MAIN_OBJS) $(LDFLAGS)

# Test hardware executable
$(TEST_TARGET): $(TEST_OBJS)
	$(CC) -o $(TEST_TARGET) $(TEST_OBJS) $(LDFLAGS)

# Object file rules
main.o: main.c Webhouse.h handshake.h jansson.h
	$(CC) $(CFLAGS) -c main.c

test_hardware.o: test_hardware.c Webhouse.h
	$(CC) $(CFLAGS) -c test_hardware.c

Webhouse.o: Webhouse.c Webhouse.h
	$(CC) $(CFLAGS) -c Webhouse.c

handshake.o: handshake.c handshake.h base64.h sha1.h
	$(CC) $(CFLAGS) -c handshake.c

base64.o: base64.c base64.h
	$(CC) $(CFLAGS) -c base64.c

sha1.o: sha1.c sha1.h
	$(CC) $(CFLAGS) -c sha1.c

# Clean up build artifacts
clean:
	rm -f $(TARGET) $(TEST_TARGET) *.o

# Phony targets
.PHONY: all clean
