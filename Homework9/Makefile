all: mcast_send mcast_recv

mcast_send: mcast_send.c
	gcc -w mcast_send.c -o mcast_send

mcast_recv: mcast_recv.c
	gcc -w mcast_recv.c -o mcast_recv

clean:
	rm -f *.o *~ mcast_recv mcast_send