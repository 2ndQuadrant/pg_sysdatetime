-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pair" to load this file. \quit

CREATE OR REPLACE FUNCTION sysutcdatetime()
RETURNS TIMESTAMP WITHOUT TIME ZONE
LANGUAGE c
AS 'pg_sysdatetime','pg_sysutcdatetime'
VOLATILE;

COMMENT ON FUNCTION sysutcdatetime()
IS 'Return high precision timestamp for current UTC time as timestamp';

CREATE OR REPLACE FUNCTION sysdatetimeoffset()
RETURNS TIMESTAMP WITH TIME ZONE
LANGUAGE c
AS 'pg_sysdatetime','pg_sysdatetimeoffset'
VOLATILE;

COMMENT ON FUNCTION sysdatetimeoffset()
IS 'Return high precision timestamp for current UTC time as timestamptz';

CREATE OR REPLACE FUNCTION sysdatetime()
RETURNS TIMESTAMP WITHOUT TIME ZONE
LANGUAGE c
AS 'pg_sysdatetime','pg_sysdatetime'
VOLATILE;

COMMENT ON FUNCTION sysdatetime()
IS 'Return high precision timestamp for current local (TimeZone) time as timestamp';
