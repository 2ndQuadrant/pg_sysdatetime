-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pair" to load this file. \quit

CREATE OR REPLACE FUNCTION sysutcdatetime()
RETURNS TIMESTAMP WITHOUT TIME ZONE
LANGUAGE c
AS 'pg_sysdatetime','pg_sysutcdatetime'
STABLE;

COMMENT ON FUNCTION sysutcdatetime()
IS 'Return high precision timestamp for current UTC time as timestamp';

CREATE OR REPLACE FUNCTION sysdatetimeoffset()
RETURNS TIMESTAMP WITHOUT TIME ZONE
LANGUAGE c
AS 'pg_sysdatetime','pg_sysdatetimeoffset'
STABLE;

COMMENT ON FUNCTION sysdatetimeoffset()
IS 'Return high precision timestamp for current UTC time as timestamptz';

CREATE OR REPLACE FUNCTION sysdatetime()
RETURNS TIMESTAMP WITHOUT TIME ZONE
LANGUAGE c
AS 'pg_sysdatetime','pg_sysdatetime'
STABLE;

COMMENT ON FUNCTION sysdatetime()
IS 'Return high precision timestamp for current local (TimeZone) time as timestamp';
