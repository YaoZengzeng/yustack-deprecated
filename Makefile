CC = gcc -pthread

ROOT = $(shell pwd)
OBJ = $(ROOT)/obj
LIB = $(ROOT)/include
BIN = $(OBJ)/yustack
OBJFILES = $(OBJ)/main.o \
		   $(OBJ)/tap.o \
		   $(OBJ)/dev.o \
		   $(OBJ)/ethernet.o \
		   $(OBJ)/skbuff.o \
		   $(OBJ)/af_inet.o \
		   $(OBJ)/ip_input.o \
		   $(OBJ)/arp.o \
		   $(OBJ)/route.o \
		   $(OBJ)/protocol.o \
		   $(OBJ)/icmp.o \
		   $(OBJ)/socket.o \
		   $(OBJ)/sock.o \
		   $(OBJ)/ip_output.o \
		   $(OBJ)/neighbour.o \
		   $(OBJ)/checksum.o \
		   $(OBJ)/fib_hash.o \
		   $(OBJ)/fib_frontend.o \
		   $(OBJ)/devinet.o \
		   $(OBJ)/udp.o \
		   $(OBJ)/datagram.o

SUBDIRS = $(shell ls -l | grep ^d | awk '{if($$9 != "obj" && $$9 != "include") print $$9}')

CFLAGS = -Wall -I$(LIB)

export ROOT OBJ LIB CFLAGS

all: $(BIN)
	@echo build yustack finished

$(BIN): $(SUBDIRS) $(LIB)
	mkdir -p $(OBJ)
	@for i in $(SUBDIRS); do \
		make -C $$i; \
	done
	$(CC) $(OBJFILES) -o $@

clean:
	rm -rf $(OBJ)
