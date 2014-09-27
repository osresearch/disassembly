all: example secret

example: example.c
	$(CC) $(CFLAGS) -o $@ example.c /opt/local/lib/libz.a
	strip $@

secret: secret.c
	$(CC) $(CFLAGS) -o $@ secret.c
	strip $@
