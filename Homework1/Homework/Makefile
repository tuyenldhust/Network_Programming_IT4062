CC=gcc
CFLAGS=-w

DEPS = genSLL.h

OBJ = UserManagement.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

UserManagement: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o UserManagement
