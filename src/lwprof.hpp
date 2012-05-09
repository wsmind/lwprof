#ifndef __LWPROF_HPP__
#define __LWPROF_HPP__

// use C99 types when available
#ifndef _MSC_VER
#	include <inttypes.h>
#endif

namespace lwprof {

// utility type definitions
#ifdef _MSC_VER
	typedef unsigned __int32 u32;
	typedef unsigned __int64 u64;
	#define LWPROF_THREAD_LOCAL __declspec(thread)
#else
	typedef uint32_t u32;
	typedef uint64_t u64;
	#define LWPROF_THREAD_LOCAL __thread
#endif // _MSC_VER

/************************
 * CONFIG
 ************************/

// Runtime memory usage (data segment) is MAX_EVENTS * MAX_PROFILED_THREADS * sizeof(ProfileEvent)

/// Maximum number of events that the profiler will collect per thread.
/// After this value is reached, all firing events will be discarded.
const u32 MAX_EVENTS = 4096;

/// Maximum number of threads that can be profiled during one
/// execution of the program.
const u32 MAX_PROFILED_THREADS = 16;

/************************
 * API
 ************************/

#define PROFILE_THREAD(name) \
	::lwprof::Profiler::instance.declareThread(name);

#define PROFILE_BLOCK(name) \
	::lwprof::ScopeProfile _scopeProfile(name);

#define PROFILE_DUMP(logger, json) \
	::lwprof::Profiler::instance.dump(logger, json);

/************************
 * INTERNALS
 ************************/

struct ProfileEvent
{
	u64 start;
	u64 end;
	const char *name;
	
	ProfileEvent();
};

class ProfileContext
{
	public:
		ProfileContext();
		
		/**
		 * \brief Print everything using the given logger
		 */
		template <class Logger>
		void dump(Logger logger, bool json) const;
		
		/**
		 * \internal
		 * \brief Create a new event
		 * When MAX_EVENTS has been reached, this method returns NULL.
		 */
		ProfileEvent *createEvent();
		
		/**
		 * \internal
		 * \brief Find bounds of this context
		 */
		void computeBounds(u64 *min, u64 *max) const;
		
	private:
		/**
		 * \brief Extract stack hierarchy information and print it
		 */
		template <class Logger>
		void traverse(Logger logger, bool json, u32 index = 0, u32 indent = 0) const;
		
		ProfileEvent events[MAX_EVENTS];
		u32 eventCount;
};

template <class Logger>
void ProfileContext::dump(Logger logger, bool json) const
{
	if (json)
		logger("[\n");
	
	if (this->eventCount > 0)
		this->traverse(logger, json, 0, 2);
	
	if (json)
		logger("  ]");
}

template <class Logger>
void ProfileContext::traverse(Logger logger, bool json, u32 index, u32 indent) const
{
	const ProfileEvent *event = &this->events[index];
	
	// print this event
	if (json)
	{
		for (u32 i = 0; i < indent; i++) logger("  ");
		logger("{\n");
		for (u32 i = 0; i < indent + 1; i++) logger("  ");
		logger("\"name\": \"%s\",\n", event->name);
		for (u32 i = 0; i < indent + 1; i++) logger("  ");
		logger("\"start\": \"%lu\",\n", event->start);
		for (u32 i = 0; i < indent + 1; i++) logger("  ");
		logger("\"end\": \"%lu\"", event->end);
	}
	else
	{
		for (u32 i = 0; i < indent; i++)
			logger("  ");
		
		const char *unit = "ns";
		u64 divider = 1;
		
		u64 duration = event->end - event->start;
		
		if (duration > 1000000000)
		{
			unit = "s";
			divider = 1000000000;
		}
		else if (duration > 1000000)
		{
			unit = "ms";
			divider = 1000000;
		}
		else if (duration > 1000)
		{
			unit = "us";
			divider = 1000;
		}
		
		u64 intPart = duration / divider;
		int decimalPart = (int)((duration - intPart * divider) / (divider / 100));
		#ifdef _WIN32
			// yep, %I64u works with MinGW and not just MSVC :)
			logger("%s: %I64u.%02d %s\n", event->name, intPart, decimalPart, unit);
		#else
			// FIXME: this won't work on 32-bit machines
			logger("%s: %lu.%02d %s\n", event->name, intPart, decimalPart, unit);
		#endif
	}
	
	u32 nextIndex = index + 1;
	if (nextIndex < this->eventCount)
	{
		if (this->events[nextIndex].end <= event->end)
		{
			// the next event is child to this one
			if (json)
			{
				logger(",\n");
				for (u32 i = 0; i < indent + 1; i++) logger("  ");
				logger("\"subs\": [\n");
			}
			
			traverse(logger, json, nextIndex, indent + 2);
			
			if (json)
			{
				for (u32 i = 0; i < indent + 1; i++) logger("  ");
				logger("]\n");
				for (u32 i = 0; i < indent; i++) logger("  ");
				logger("}\n");
			}
		}
		else
		{
			// the next event is a sibling
			if (json)
			{
				logger("\n");
				for (u32 i = 0; i < indent; i++) logger("  ");
				logger("},\n");
			}
			traverse(logger, json, nextIndex, indent);
		}
	}
	else
	{
		// there is no next event
		if (json)
		{
			logger("\n");
			for (u32 i = 0; i < indent; i++) logger("  ");
			logger("}\n");
		}
	}
}

class Profiler
{
	public:
		Profiler();
		
		// declare the current thread as a new thread with the given name
		void declareThread(const char *name);
		
		ProfileEvent *createEvent();
		
		template <class Logger>
		void dump(Logger logger, bool json = false) const;
		
		static Profiler instance;
		
	private:
		/**
		 * \brief Find first and last time stored in contexts
		 */
		void computeBounds(u64 *min, u64 *max) const;
		
		ProfileContext threadContexts[MAX_PROFILED_THREADS];
		const char *threadName[MAX_PROFILED_THREADS];
		volatile unsigned int threadCount; // atomic; used only with intrinsics
		
		static LWPROF_THREAD_LOCAL ProfileContext *currentThreadContext;
};

template <class Logger>
void Profiler::dump(Logger logger, bool json) const
{
	u64 minBound;
	u64 maxBound;
	this->computeBounds(&minBound, &maxBound);
	
	if (json)
	{
		logger("profileData = {\n");
		logger("  \"bounds\": {\"min\": %lu, \"max\": %lu},\n", minBound, maxBound);
		logger("  \"threads\": {\n");
	}
	
	for (unsigned int i = 0; i < this->threadCount; i++)
	{
		if (json)
			logger("  \"%d-%s\": ", i, this->threadName[i]);
		else
			logger("== %d-%s ==\n", i, this->threadName[i]);
		
		this->threadContexts[i].dump(logger, json);
		
		if (json)
		{
			if (i < (this->threadCount - 1))
				logger(",");
			
			logger("\n");
		}
	}
	
	if (json)
	{
		logger("  }\n");
		logger("}\n");
	}
}

class ScopeProfile
{
	public:
		ScopeProfile(const char *name)
		{
			this->event = Profiler::instance.createEvent();
			
			if (this->event)
			{
				this->event->start = this->readNanoseconds();
				this->event->name = name; // works only for global strings
			}
		}
		
		~ScopeProfile()
		{
			if (this->event)
				this->event->end = this->readNanoseconds();
		}
		
	private:
		// read precision time
		u64 readNanoseconds();
		
		ProfileEvent *event;
};

} // lwprof namespace

#endif // __LWPROF_HPP__
