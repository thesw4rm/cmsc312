CC = gcc
BIN =  ravg rhello avg_svc hello_svc
GEN = avg_clnt.c avg_svc.c avg_xdr.c avg.h hello_clnt.c hello_svc.c hello_xdr.c hello.h
#INC = -I/usr/include/tirpc/ -ltirpc
RPCCOM = rpcgen

all: $(BIN)

ravg: ravg.o avg_clnt.o avg_xdr.o
	$(CC) $(INC) -o $@ ravg.o avg_clnt.o avg_xdr.o -lnsl

ravg.o: ravg.c avg.h
	$(CC) $(INC) -g ravg.c -c

rhello: rhello.o hello_clnt.o hello_xdr.o
	$(CC) $(INC) -o $@ rhello.o hello_clnt.o hello_xdr.o -lnsl

rhello.o: rhello.c hello.h
	$(CC) $(INC) -g rhello.c -c


avg_svc: avg_proc.o avg_svc.o avg_xdr.o
	$(CC) $(INC) -o $@ avg_proc.o avg_svc.o avg_xdr.o -lnsl

avg_proc.o: avg_proc.c avg.h
	$(CC) $(INC) -g avg_proc.c -c


hello_svc: hello_proc.o hello_svc.o hello_xdr.o
	$(CC) $(INC) -o $@ hello_proc.o hello_svc.o hello_xdr.o -lnsl

hello_proc.o: hello_proc.c hello.h
	$(CC) $(INC) -g hello_proc.c -c


$(GEN): avg.x
	$(RPCCOM) avg.x
	$(RPCCOM) hello.x
clean cleanup:
	rm -f $(GEN) *.o $(BIN)
