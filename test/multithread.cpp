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
