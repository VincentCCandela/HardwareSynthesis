CC = g++
CFLAGS = -Wall -Wextra -O2

TARGET = main

SRCS     := $(wildcard *.cpp)
OBJS     := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean