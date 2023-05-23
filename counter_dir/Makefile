CC=g++
CFLAGS = -O3 -pthread -g -std=c++2a

OBJFILES = counter.o main.o locks.o time.o

CFILES = counter.cpp main.cpp locks.cpp time.cpp

TARGET = counter

all: $(TARGET)

$(TARGET): $(CFILES)
	$(CC) $(CFILES) $(CFLAGS) -o $(TARGET) ; mv $(TARGET) ..

clean:
	rm -rf $(OBJFILES) $(TARGET) *~ 