.SUFFIXES : .c .o
CFLAGS = -g -lcrypto -lssl

OBJS = client.o hashFunction.o ecdsa.o
SRCS = $(OBJS:.o=.c)
HFILES = common.h
EXE = client

exe : $(OBJS) $(HFILES)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

clean :
	rm -rf $(EXE)
	rm -f *.o core core.*
