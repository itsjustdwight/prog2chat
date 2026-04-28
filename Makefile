# Makefile for CPE464 tcp test code
# written by Hugh Smith - April 2019

CC= gcc
CFLAGS= -g -Wall -std=gnu99
LIBS = 

COMMON_OBJS = networks.o gethostbyname.o pollLib.o safeUtil.o pdu.o chatPacket.o
SERVER_OBJS = $(COMMON_OBJS) handleTable.o
CLIENT_OBJS = $(COMMON_OBJS)

all:   cclient server

cclient: cclient.c $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o cclient cclient.c  $(CLIENT_OBJS) $(LIBS)

server: server.c $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o server server.c $(SERVER_OBJS) $(LIBS)

.c.o:
	gcc -c $(CFLAGS) $< -o $@ $(LIBS)

cleano:
	rm -f *.o

clean:
	rm -f server cclient *.o




