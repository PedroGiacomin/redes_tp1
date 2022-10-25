all: 
	gcc -Wall -c common.c -o common.o
	gcc -Wall client.c common.o -o client
	gcc -Wall server.c common.o -o server

clean:
	rm *.o client server teste

teste:
	gcc -Wall -o teste teste.c 