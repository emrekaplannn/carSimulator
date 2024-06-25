CC = g++
CFLAGS = -Wall -Wextra -pthread

# Define the source files and object files
SRCS = main.cpp helper.c WriteOutput.c
OBJS = $(SRCS:.cpp=.o)

# Define the target executable
TARGET = simulator

# Default target
all: $(TARGET)

# Compile C++ files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Clean the generated files
clean:
	rm -f $(OBJS) $(TARGET)

