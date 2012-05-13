/******************************************************************************
 * 
 * lwprof - Lightweight time profiler for C++
 * Copyright (c) 2012 Remi Papillie
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 * 
 *****************************************************************************/

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
