CC = gcc
CFLAGS = -Wall -pthread
OBJS = main.o vehicle.o ferry.o

ferry_tour: $(OBJS)
	$(CC) $(CFLAGS) -o ferry_tour $(OBJS)

main.o: main.c vehicle.h ferry.h
	$(CC) $(CFLAGS) -c main.c

vehicle.o: vehicle.c vehicle.h
	$(CC) $(CFLAGS) -c vehicle.c

ferry.o: ferry.c ferry.h
	$(CC) $(CFLAGS) -c ferry.c

clean:
	rm -f *.o ferry_tour
