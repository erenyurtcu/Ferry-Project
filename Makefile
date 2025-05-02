CC = gcc
CFLAGS = -Wall -pthread
OBJS = main.o vehicle.o ferry.o

ferry_tour: main.o vehicle.o ferry.o visual.o
	gcc -Wall -pthread -o ferry_tour main.o vehicle.o ferry.o visual.o

main.o: main.c
	gcc -Wall -pthread -c main.c

vehicle.o: vehicle.c
	gcc -Wall -pthread -c vehicle.c

ferry.o: ferry.c
	gcc -Wall -pthread -c ferry.c

visual.o: visual.c
	gcc -Wall -pthread -c visual.c

clean:
	rm -f *.o ferry_tour
