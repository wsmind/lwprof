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

#ifdef _WIN32
#	include <windows.h>
#else
#	include <unistd.h>
#	include <pthread.h>
#endif

#include <cstdio>
#include <cstdlib>

#include "../src/lwprof.hpp"

void someProcessing()
{
	PROFILE_BLOCK("SomeProcessing");
#ifdef _WIN32
	Sleep(rand() % 100);
#else
	usleep(rand() % 100000);
#endif
}

// Stupid worker thread
//
// Do 10 times:
//   Sleep between 0 and 100ms
//   Do SomeProcessing(), that will last between 0 and 100ms
//
#ifdef _WIN32
	DWORD WINAPI threadFunc(LPVOID param)
#else
	static void *threadFunc(void *data)
#endif
{
	PROFILE_THREAD("Worker");
	
	for (int i = 0; i < 10; i++)
	{
		#ifdef _WIN32
			Sleep(rand() % 100);
		#else
			usleep(rand() % 100000);
		#endif
		
		// process a bit
		someProcessing();
	}
	
	return 0;
}

int main()
{
	PROFILE_THREAD("Main");
	
	{
		PROFILE_BLOCK("Worker fork & join");
		
		#ifdef _WIN32
			HANDLE threads[10];
			for (int i = 0; i < 10; i++)
				threads[i] = CreateThread(NULL, 0, threadFunc, NULL, 0, NULL);
			
			for (int i = 0; i < 10; i++)
				WaitForSingleObject(threads[i], INFINITE);
		#else
			pthread_t threads[10];
			for (int i = 0; i < 10; i++)
				pthread_create(&threads[i], NULL, threadFunc, NULL);
			
			for (int i = 0; i < 10; i++)
				pthread_join(threads[i], NULL);
		#endif
	}
	
	PROFILE_DUMP(printf, true);
	
	return 0;
}
