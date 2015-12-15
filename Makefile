CC = gcc
CFLAGS = -g
SRC = main.cc
LIBS = -lqpdf -lstdc++

all: $(SRC)
	$(CC) $(CFLAGS) $(INC) $(LIBS) -o qpdfImageEmbed $(SRC)

clean:
	rm qpdfImageEmbed
