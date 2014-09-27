#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int compare1(const char * s)
{
	static const char secret[] = "tfufdbtuspopnz";
	for (size_t i = 0 ; i < sizeof(secret)-1 ; i++)
	{
		char c = *s++;
		//printf("%zu: '%c' '%c'\n", i, c, secret[i]-1);
		if (c != secret[i]-1)
			return -1;
	}

	return 0;
}

char * get_secret(void)
{
	static const uint8_t secret[] = {
0xd1, 0xca, 0xca, 0x85, 0xc8, 0xc4, 0xcb, 0xdc, 0x85, 0xd6, 0xc0, 0xc6,
  0xd7, 0xc0, 0xd1, 0xd6
};

	char * s = malloc(sizeof(secret));

	for (size_t i = 0 ; i < sizeof(secret) ; i++)
	{
		s[i] = secret[i] ^ 0xA5;
	}

	return s;
}

int main(int argc, char ** argv)
{
	if (argc != 2)
		goto failure;
	int rc;
	rc = compare1(argv[1]);
	if (rc < 0)
		goto failure;

	printf("magic words: %s\n", get_secret());
	return 0;

failure:
	printf("sorry!\n");
	return -1;
}
