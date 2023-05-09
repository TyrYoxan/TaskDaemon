CC=gcc
CFLAGS=-Wall -Wextra -g -std=c99
LDFLAGS=-L. -lmessage

all: ../libmessage.so receiver sender #taskd taskcli

message.o: ../message.c message.h
	gcc -c -Wall -Werror -fpic message.c

libmessage.so: ../message.o
	$(CC) -shared -o libmessage.so message.o

receiver: ../receiver.c message.h libmessage.so
	$(CC) -o receiver receiver.c $(LDFLAGS)

sender: ../sender.c message.h libmessage.so
	$(CC) -o sender sender.c $(LDFLAGS)

taskd: ../taskd.c message.h
	$(CC) $(CFLAGS) -o taskd taskd.c $(LDFLAGS)

taskcli: ../taskcli.c message.h
	$(CC) $(CFLAGS) -o taskcli taskcli.c $(LDFLAGS)
clean:
	rm -f *.o *.so receiver sender taskcli taskd