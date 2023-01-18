all: client server

client: client.c
	gcc -w client.c -o client -pthread

server: tcp_server.c
	gcc -w server.c -o server -pthread

clean:
	rm -f *.o *~ server client