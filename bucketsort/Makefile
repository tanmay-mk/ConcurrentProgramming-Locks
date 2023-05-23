CC=g++
CFLAGS = -O3 -pthread -g -std=c++2a

OBJFILES = bucketsort.o main.o locks.o time.o

CFILES = bucketsort.cpp main.cpp locks.cpp time.cpp

TARGET = mysort

all: $(TARGET)

$(TARGET): $(CFILES)
	$(CC) $(CFILES) $(CFLAGS) -o $(TARGET) ; mv $(TARGET) ..

clean:
	rm -rf $(OBJFILES) $(TARGET) *~ 