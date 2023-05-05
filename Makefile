CC=gcc
CFLAGS=-Wall -Wextra -g -std=c99
LDFLAGS=-L. -lmessage

all: libmessage.so receiver sender taskd taskcli

libmessage.so: message.c
	gcc -shared -o libmessage.so -fPIC message.c

receiver: receiver.c message.h
	gcc -o receiver receiver.c -L. -lmessage

sender: sender.c message.h
	gcc -o sender sender.c -L. -lmessage

taskd: taskd.c message.h
	gcc -g -Wall -Wextra -std=c99 -o taskd taskd.c -L. -lmessage

taskcli: taskcli.c message.h
	gcc -g -Wall -Wextra -std=c99 -o taskcli taskcli.c -L. -lmessage
clean:
	rm -f *.o *.so receiver sender