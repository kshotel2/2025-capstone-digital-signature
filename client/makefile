.SUFFIXES : .c .o
CFLAGS = -g -lcrypto -lssl

OBJS = client.o ecdsa.o send_certificate.o clnt_fork.o clnt_cmd.o clnt_to_serv.o make_csr.o cert_get_pubkey.o ecdsaVerify.o

SRCS = $(OBJS:.o=.c)
HFILES = common.h
EXE = client

exe : $(OBJS) $(HFILES)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

clean :
	rm -rf $(EXE)
	rm -f *.o core core.*
