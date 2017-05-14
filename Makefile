PROGS += example
PROGS += secret
CC = gcc
NM = nm

all: $(PROGS)
clean:
	$(RM) $(PROGS)

CFLAGS = \
	-momit-leaf-frame-pointer \
	-O1 \
	-W \
	-Wall \

example: example.c
	$(CC) $(CFLAGS) -o $@ example.c -L/opt/local/lib/ -lz
	$(NM) $@ > $@.lst
	strip $@

secret: secret.c
	$(CC) $(CFLAGS) -o $@ secret.c
	$(NM) $@ > $@.lst
	strip $@


