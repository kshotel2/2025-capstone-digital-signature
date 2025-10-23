.SUFFIXES : .c .o
CFLAGS = -g -lcrypto -lssl -pthread


OBJS = server.o serv_cmd.o ecdsa.o ecdsaVerify.o get_pubkey.o send_certificate.o

SRCS = $(OBJS:.o=.c)
HFILES = common.h
EXE = server

exe : $(OBJS) $(HFILES)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

clean :
	rm -rf $(EXE)
	rm -f *.o core core.*
