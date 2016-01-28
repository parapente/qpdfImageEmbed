CC = gcc
CFLAGS = -O2 -DQT_NO_DEBUG_OUTPUT
#CFLAGS = -g
SRC = main.cpp imageProvider.cpp
LIBS = -L/usr/lib/qt4 -lqpdf -lstdc++ -lQtGui -lQtCore -lboost_program_options -lm
INC = -I/usr/include/qt4

all: $(SRC)
	$(CC) $(CFLAGS) $(INC) $(LIBS) -o qpdfImageEmbed $(SRC)

clean:
	rm qpdfImageEmbed
install:
	cp qpdfImageEmbed /usr/bin/