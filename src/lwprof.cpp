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

#include "lwprof.hpp"

#ifdef _WIN32
#	include <windows.h>
#else
#	include <sys/time.h>
#	include <time.h>
#endif

namespace lwprof {

// global profiler instance
Profiler Profiler::instance;

LWPROF_THREAD_LOCAL ProfileContext *Profiler::currentThreadContext = 0;

ProfileEvent::ProfileEvent()
{
	this->start = 0;
	this->end = 0;
	this->name = NULL;
}

ProfileContext::ProfileContext()
{
	this->eventCount = 0;
}

ProfileEvent *ProfileContext::createEvent()
{
	if (this->eventCount == MAX_EVENTS)
		return 0;
	
	return &this->events[this->eventCount++];
}

void ProfileContext::computeBounds(u64 *min, u64 *max) const
{
	if (this->eventCount == 0)
	{
		*min = 0;
		*max = 0;
		return;
	}
	
	*min = this->events[0].start;
	*max = this->events[0].end;
	
	for (u32 i = 1; i < this->eventCount; i++)
	{
		if (this->events[i].end > *max)
			*max = this->events[i].end;
	}
}

Profiler::Profiler()
{
	this->threadCount = 0;
}

// declare the current thread as a new thread with the given name
void Profiler::declareThread(const char *name)
{
	// cross-platform atomic increment
	#ifdef _WIN32
		// WIN32 API (also works on MinGW)
		unsigned int threadIndex = InterlockedIncrement((volatile LONG *)&this->threadCount) - 1;
	#else
		// GCC intrinsic (Posix-only)
		unsigned int threadIndex = __sync_fetch_and_add(&this->threadCount, 1);
	#endif
	
	this->threadName[threadIndex] = name;
	Profiler::currentThreadContext = &this->threadContexts[threadIndex];
}

ProfileEvent *Profiler::createEvent()
{
	if (Profiler::currentThreadContext != NULL)
		return Profiler::currentThreadContext->createEvent();
	
	return NULL;
}

void Profiler::computeBounds(u64 *min, u64 *max) const
{
	if (this->threadCount == 0)
	{
		*min = 0;
		*max = 0;
		return;
	}
	
	this->threadContexts[0].computeBounds(min, max);
	
	for (u32 i = 1; i < this->threadCount; i++)
	{
		u64 currentMin;
		u64 currentMax;
		
		this->threadContexts[i].computeBounds(&currentMin, &currentMax);
		
		if (currentMin > *min)
			*min = currentMin;
		
		if (currentMax > *max)
			*max = currentMax;
	}
}

// read precision time
u64 ScopeProfile::readNanoseconds()
{
	u64 nanoseconds;
	
	#ifdef _WIN32
	{
		// QueryPerformanceCounter implementation (WIN32 API)
		
		// Side note: the frequency must be read every time because it may
		// not be a fixed value. For instance, recent laptop processors change
		// their frequency dynamically to accomodate for particular energy requirements.
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		
		LARGE_INTEGER ticks;
		QueryPerformanceCounter(&ticks);
		
		nanoseconds = (u64)(ticks.QuadPart * 1000000000 / frequency.QuadPart);
	}
	#else
	{
		// gettimeofday implementation (POSIX)
		struct timeval timeOfDay;
		gettimeofday(&timeOfDay, NULL);
		nanoseconds = (u64)(timeOfDay.tv_sec * 1000000 + (u64)timeOfDay.tv_usec) * 1000;
	}
	#endif
	
	return nanoseconds;
}

} // lwprof namespace
