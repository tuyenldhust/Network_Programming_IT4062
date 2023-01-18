all: client server

client: tcp_client.c
	gcc -w tcp_client.c -o client

server: tcp_server.c
	gcc -w tcp_server.c -o server

clean:
	rm -f *.o *~ server client