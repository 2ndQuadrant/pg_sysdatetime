#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "utils/datetime.h"
#include "utils/guc.h"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PGDLLEXPORT void _PG_init(void);
PGDLLEXPORT void _PG_fini(void);

static bool adjust_timer_resolution;

#if defined(WIN32)

/* Timer resolution to request from Windows, in milliseconds */
static const UINT targetTimeResolution = 1;

static UINT requestedTimerRes = 0;

/* from src/port/gettimeofday.c */
/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = UINT64CONST(116444736000000000);

/* replaces gettimeofday() in src/port/gettimeofday.c */
static int
getetimeofday_highres(struct timeval * tp, struct timezone * tzp)
{
	FILETIME t;
	ULARGE_INTEGER ularge;

	GetSystemTimeAsFileTime(&t);
	ularge.LowPart = t.dwLowDateTime;
	ularge.HighPart = t.dwHighDateTime;

	/* from src/port/gettimeofday.c */
	tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long) (((ularge.QuadPart - epoch) % 10000000L) / 10);  // units of 100 nanos to units of micros

	return 0;
}

/* Replaces GetCurrentTimestamp in src/backend/utils/adt/timestamp.c */
static TimestampTz
GetCurrentTimestampHighres(void)
{
	TimestampTz result;
	struct timeval tp;

	(void) getetimeofday_highres(&tp, NULL);

	result = (TimestampTz) tp.tv_sec -
		((POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE) * SECS_PER_DAY);
#ifdef HAVE_INT64_TIMESTAMP
	result = (result * USECS_PER_SEC) + tp.tv_usec;
#else
	result = result + (tp.tv_usec / 1000000.0);
#endif
	return result;
}

static void
setTimerResolution()
{	
	TIMECAPS caps;
	
	if (!adjust_timer_resolution)
		return;

	Assert(requestedTimerRes == 0);

	if (timeGetDevCaps(&caps, sizeof(TIMECAPS)) != TIMERR_NOERROR)
	{
		ereport(WARNING,
				(errmsg("Failed to get current Windows timer resolution"),
				 errdetail("Call to timeGetDevCaps(...) failed")));
	}
	else
	{
		requestedTimerRes = min(max(caps.wPeriodMin, targetTimeResolution), caps.wPeriodMax);

		if (timeBeginPeriod(requestedTimerRes) != TIMERR_NOERROR)
		{
			ereport(WARNING,
					(errmsg("Failed to set timer resolution to %d ms", requestedTimerRes),
					 errdetail("timeBeginPeriod(...) call failed")));
			requestedTimerRes = 0;
		}
	}
}

static void
restoreTimerResolution()
{
	if (requestedTimerRes == 0)
		return;

	if ( timeEndPeriod(requestedTimerRes) != TIMERR_NOERROR)
	{
		ereport(WARNING,
				(errmsg("Failed to restore timer resolution", requestedTimerRes),
					errdetail("timeEndPeriod(...) call failed")));
	}
	/* Even if the restore failed, all we can do is keep on going, so ... */
	requestedTimerRes = 0;
}

#else

/* on non-Windows, simply use the existing GetCurrentTimestamp */
inline static TimestampTz
GetCurrentTimestampHighres(void)
{
	return GetCurrentTimestamp();
}

/* Linux's timing doesn't need correction */
inline static void setTimerResolution() 
{
}

inline static void restoreTimerResolution() 
{
}
#endif

PG_FUNCTION_INFO_V1(pg_sysutcdatetime);

PGDLLEXPORT Datum
pg_sysutcdatetime(PG_FUNCTION_ARGS)
{
	PG_RETURN_TIMESTAMP(GetCurrentTimestampHighres());
}

PG_FUNCTION_INFO_V1(pg_sysdatetimeoffset);

PGDLLEXPORT Datum
pg_sysdatetimeoffset(PG_FUNCTION_ARGS)
{
	PG_RETURN_TIMESTAMPTZ(GetCurrentTimestampHighres());
}

PG_FUNCTION_INFO_V1(pg_sysdatetime);

PGDLLEXPORT Datum
pg_sysdatetime(PG_FUNCTION_ARGS)
{
	Timestamp ts = GetCurrentTimestampHighres();

	return DirectFunctionCall1(timestamptz_timestamp, TimestampGetDatum(ts));
}

void
adjust_timer_resolution_assign_hook(bool newval, void *extra)
{
	if (newval != adjust_timer_resolution)
	{
		adjust_timer_resolution = newval;
		restoreTimerResolution();
		setTimerResolution();
	}
}

PGDLLEXPORT void
_PG_init()
{
	DefineCustomBoolVariable(
		"pg_sysdatetime.adjust_timer_resolution",
		"Increase the system timer resolution (only affects Windows)",
		NULL,
		&adjust_timer_resolution,
		false,
		PGC_SUSET, 0,
		NULL, &adjust_timer_resolution_assign_hook, NULL);

	setTimerResolution();
}

PGDLLEXPORT void
_PG_fini()
{
	restoreTimerResolution();
}