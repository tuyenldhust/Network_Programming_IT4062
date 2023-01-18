all: client server

client: client.c
	gcc -w client.c -o client

server: server.c
	gcc -w server.c -o server

clean:
	rm -f *.o *~ server client