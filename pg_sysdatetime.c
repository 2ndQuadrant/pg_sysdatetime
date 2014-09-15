#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "utils/datetime.h"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#if defined(WIN32)

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

#else
/* on non-Windows, simply use the existing GetCurrentTimestamp */

inline static TimestampTz
GetCurrentTimestampHighres(void)
{
	return GetCurrentTimestamp();
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
