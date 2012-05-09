#include "../src/lwprof.hpp"

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>

// how long does it take to read an entire file to memory?
int main(int argc, char **argv)
{
	PROFILE_THREAD("File reader");
	
	FILE *f = NULL;
	int bytes = 0;
	
	if (argc < 2)
	{
		printf("You must provide a file to read!\n");
		return 1;
	}
	
	{
		PROFILE_BLOCK("Open");
		f = fopen(argv[1], "rb");
		
		if (!f)
		{
			printf("'%s' cannot be opened!\n", argv[1]);
			return 1;
		}
	}
	
	printf("file opened\n");
	
	{
		PROFILE_BLOCK("Read");
		
		char buffer[512 * 1024]; // buffer size of 512 KB
		while (!feof(f))
		{
			bytes += fread(buffer, 1, sizeof(buffer), f);
		}
	}
	
	printf("%d bytes read\n", bytes);
	
	{
		PROFILE_BLOCK("Close");
		fclose(f);
	}
	
	printf("file closed\n");
	
	PROFILE_DUMP(printf, false);
	
	return 0;
}
