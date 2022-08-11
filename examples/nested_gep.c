#include <stddef.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
	char * a[argc];
	for (size_t i = 0; i < argc; ++i)
		a[i] = argv[i];
	for (size_t i = 0; i < argc; ++i)
		printf("%s\n", a[i]);
	return 0;
}
