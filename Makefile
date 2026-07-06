# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lmd
INCLUDES = -I.

# Source files (list explicitly)
SRC = main.c \
      project/program.c \
      project/services/file_repository.c \
      project/services/search_service.c \
      project/services/types.c \
      project/lib/provided.c \
      project/subway/subway_record_list.c \
      project/subway/subway_header_repository.c \
      project/subway/subway_record_repository.c \
      project/subway/input/input_repository.c \
      project/indexable/indexable_record_avl.c \
      project/indexable/indexable_record_repository.c \
      project/subway/graph.c


OBJ = $(SRC:.c=.o)

# Output binary
TARGET = programaTrab

.PHONY: all run clean build

# Default rule: compile the program
all:
	gcc $(INCLUDES) -o $(TARGET) $(SRC)

run:
	./$(TARGET)

build: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(TARGET) $(OBJ)
