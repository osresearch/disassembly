// show some different constructions
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <zlib.h>

#define USED __attribute__((__used__)) __attribute__((__noinline__))

USED
int add(int a, int b)
{
	return a + b;
}

USED
int min(int a, int b)
{
	return a < b ? a : b;
}

USED
int sum(int n, int * a)
{
	int i;
	int s = 0;
	for (i = 0 ; i < n ; i++)
		s += a[i];
	return s;
}

USED
int my_strlen(char * c)
{
	int len = 0;
	while (*c++ != '\0')
		len++;
	return len;
}

USED
int max(const int * a, int n)
{
	int m = INT_MIN;
	for (int i = 0 ; i < n ; i ++)
	{
		if (m < a[i])
			m = a[i];
	}

	return m;
}



struct list
{
	int (*function)(int arg);
	struct list * next;
};

USED
struct list *
end_of_list(
	struct list * l
)
{
	while(l->next)
		l = l->next;
	return l;
}

USED
int call_last_function(struct list * l, int arg)
{
	l = end_of_list(l);
	return l->function(arg);
}


USED
int lots_of_args(int foo, ...)
{
	return foo;
}

USED
int function_call(void)
{
	return lots_of_args(0,2,3,4,5,6,7,8,9,10);
}


struct fops_t
{
	int (*open)(struct fops_t * f, const char * name, int arg);
	int (*close)(struct fops_t * f);
	ssize_t (*stat)(struct fops_t *f);
	int (*read)(struct fops_t * f, char * buf, size_t len);
};

USED
const char *
file_read(
	struct fops_t * f,
	const char * name,
	size_t * len_out
)
{
	if (f->open(f, name, 0) < 0)
	{
		printf("%s: failed to open\n", name);
		goto fail_open;
	}

	ssize_t len = f->stat(f);
	if (len < 0)
		goto fail_stat;
	*len_out = len;

	char * buf = malloc(len);
	if (!buf)
	{
		printf("%s: malloc %zu failed\n", name, len);
		goto fail_malloc;
	}

	if (f->read(f, buf, len) < 0)
	{
		printf("%s: read failed\n", name);
		goto fail_read;
	}

	f->close(f);

	return buf;

fail_read:
	free(buf);
fail_malloc:
fail_stat:
	f->close(f);
fail_open:
	return NULL;
}


USED
int checksum(const char * filename)
{
	size_t len;
	const char * buf = file_read(NULL, filename, &len);

	uint32_t crc = crc32(0L, Z_NULL, 0);
	uint32_t old_crc = *(uint32_t*) buf;
	if (old_crc == crc32(crc, (const uint8_t*) buf+4, len - 4))
		return 0;

	printf("%s: bad file!\n", filename);
	return -1;
}



int main(void)
{
}
