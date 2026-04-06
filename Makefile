# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lmd

INCLUDES = -I.

# Source files (list explicitly)
SRC = main.c \
      project/program.c \
      project/core/file/file_repository.c \
      project/domain/subway_record.c \
      project/lib/provided.c \
      project/service/database/data_base_repository.c \
      project/service/database/header_repository.c \
      project/service/database/record_repository.c \
      project/service/input/input_repository.c

OBJ = $(SRC:.c=.o)

# Output binary
TARGET = programaTrab

.PHONY: all run clean

# Default rule: compile the program
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Run the program
run:
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(OBJ)
