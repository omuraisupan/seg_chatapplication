TARGET = server
CC = /usr/bin/gcc
SRCS = server.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -pthread -o $(TARGET)

clean:
	rm -rf $(TARGET)