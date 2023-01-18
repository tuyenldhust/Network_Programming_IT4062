all: bcast_sender bcast_receiver

bcast_sender: bcast_sender.c
	gcc -w bcast_sender.c -o bcast_sender

bcast_receiver: bcast_receiver.c
	gcc -w bcast_receiver.c -o bcast_receiver

clean:
	rm -f *.o *~ bcast_sender bcast_receiver