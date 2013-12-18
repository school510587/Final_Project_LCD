#include <stddef.h>

char *strncpy(char *dest, const char *src, size_t n)
{
	const unsigned char *s = (const unsigned char *)src;
	unsigned char *d = (unsigned char *)dest;
	while (n--)
		*d++ = *s++;
	return dest;
}
