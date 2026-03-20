CC = gcc 
CFLAGS = -g -O3 -march=native -fomit-frame-pointer -Wall -Wextra -lhdr_histogram
ifeq ($(pre_market), 1)
    CFLAGS += -Dpre_market
endif
SRC = $(shell find . -name '*.c')
OBJ = $(SRC:.c=.o)
TARGET = driver
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	find . -name '*.o' -delete
	rm -f $(TARGET)

.PHONY: all clean