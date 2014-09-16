#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "utils/datetime.h"
#include "executor/execdesc.h"
#include "executor/executor.h"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PGDLLEXPORT void _PG_init(void);
PGDLLEXPORT void _PG_fini(void);
PGDLLEXPORT void pg_sysdatetime_execstarthook(QueryDesc *queryDesc, int eflags);
static TimestampTz GetCurrentTimestampHighres(void);

static ExecutorStart_hook_type next_ExecutorStart_hook = NULL;
static TimestampTz pg_sysdatetime_statementtimestamp = 0;

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


static TimestampTz
GetStatementTimestampHighres()
{
	/*
	 * If no statement timestamp has been grabbed for this statement,
	 * collect one.
	 *
	 * This isn't set directly in the ExecutorStart_hook, partly to reduce
	 * impact on code that doesn't use these functions and partly because of an
	 * ordering problem when this extension isn't in local_ or shared_preload_libraries.
	 *
	 * The executor hook that clears the statement timeout is only installed after
	 * the executor starts and sees that this function is referenced, then goes to load it.
	 * So it's too late to clear the timestamp for that execution. We could always initialize
	 * pg_sysdatetime_statementtimestamp at DLL-load time, but we'd still have to do this
	 * branch unless we wanted to get the timestamp for all statements.
	 */
	if (pg_sysdatetime_statementtimestamp == 0)
		pg_sysdatetime_statementtimestamp = GetCurrentTimestampHighres();
	return pg_sysdatetime_statementtimestamp;
}

PG_FUNCTION_INFO_V1(pg_sysutcdatetime_statement);

PGDLLEXPORT Datum
pg_sysutcdatetime_statement(PG_FUNCTION_ARGS)
{
	PG_RETURN_TIMESTAMP(GetStatementTimestampHighres());
}

PG_FUNCTION_INFO_V1(pg_sysdatetimeoffset_statement);

PGDLLEXPORT Datum
pg_sysdatetimeoffset_statement(PG_FUNCTION_ARGS)
{
	PG_RETURN_TIMESTAMPTZ(GetStatementTimestampHighres());
}

PG_FUNCTION_INFO_V1(pg_sysdatetime_statement);

PGDLLEXPORT Datum
pg_sysdatetime_statement(PG_FUNCTION_ARGS)
{
	return DirectFunctionCall1(timestamptz_timestamp, TimestampGetDatum(GetStatementTimestampHighres()));
}

PGDLLEXPORT void
pg_sysdatetime_execstarthook(QueryDesc *queryDesc, int eflags)
{
	/* 
	 * Reset the statement timestamp. We don't grab a timestamp
	 * here because it might not be required in this statement.
	 */
	pg_sysdatetime_statementtimestamp = 0;

	if (next_ExecutorStart_hook != NULL)
		(*next_ExecutorStart_hook)(queryDesc, eflags);
	else
		standard_ExecutorStart(queryDesc, eflags);
}

PGDLLEXPORT void
_PG_init()
{
	next_ExecutorStart_hook = ExecutorStart_hook;
	ExecutorStart_hook = &pg_sysdatetime_execstarthook;
}

PGDLLEXPORT void 
_PG_fini()
{
	ExecutorStart_hook = next_ExecutorStart_hook;
}