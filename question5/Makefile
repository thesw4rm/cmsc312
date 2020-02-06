CC = gcc
BIN =  rhello hello_svc
GEN = hello_clnt.c hello_svc.c hello_xdr.c hello.h
#INC = -I/usr/include/tirpc/ -ltirpc
RPCCOM = rpcgen

all: $(BIN)


rhello: rhello.o hello_clnt.o hello_xdr.o
	$(CC) $(INC) -o $@ rhello.o hello_clnt.o hello_xdr.o -lnsl

rhello.o: rhello.c hello.h
	$(CC) $(INC) -g rhello.c -c



hello_svc: hello_proc.o hello_svc.o hello_xdr.o
	$(CC) $(INC) -o $@ hello_proc.o hello_svc.o hello_xdr.o -lnsl

hello_proc.o: hello_proc.c hello.h
	$(CC) $(INC) -g hello_proc.c -c


$(GEN): hello.x
	$(RPCCOM) hello.x
clean cleanup:
	rm -f $(GEN) *.o $(BIN)
