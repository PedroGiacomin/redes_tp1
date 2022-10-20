all: 
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o client
	gcc -Wall server.c common.o -o server

clean:
	rm *.o client server teste

test:
	gcc -Wall -c teste.c