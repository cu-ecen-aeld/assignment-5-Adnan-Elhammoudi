CC = gcc
CROSS_COMPILE ?=

ifeq ($(CROSS_COMPILE),aarch64-none-linux-gnu-)
    CC = $(CROSS_COMPILE)gcc
endif

SRCS = writer.c
OBJS = $(SRCS:.c=.o)
TARGET = writer

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f *.o $(TARGET)
