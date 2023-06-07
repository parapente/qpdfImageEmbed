CC = clang++
CFLAGS = -O2 -DQT_NO_DEBUG_OUTPUT -fPIC -std=c++17
#CFLAGS = -g -fPIC
SRC = main.cpp imageProvider.cpp
LIBS = -L/usr/lib/x86_64-linux-gnu -lstdc++ -lqpdf -lQt5Gui -lQt5Core -lboost_program_options -lm
INC = -I/usr/include/x86_64-linux-gnu/qt5

all: $(SRC)
	$(CC) $(CFLAGS) $(INC) -o qpdfImageEmbed $(SRC) $(LIBS)

clean:
	rm qpdfImageEmbed
install:
	cp qpdfImageEmbed /usr/bin/
