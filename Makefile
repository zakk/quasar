OBJS1=quasar.o movgen.o eval.o make.o attacks.o search.o null.o xboard.o
OBJS2=hash.o loadfen.o see.o notation.o loadepd.o next.o legal.o
OBJECTS=$(OBJS1) $(OBJS2)
TARGET=quasar
OPTFLAGS=-O3 -fomit-frame-pointer -fforce-mem -Wall
SAFEFLAGS=-O -Wall
CFLAGS=$(OPTFLAGS)
CC=gcc

%.o:%.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET)

all: $(TARGET)
