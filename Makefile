MODULES = pg_sysdatetime
EXTENSION = pg_sysdatetime
DATA = pg_sysdatetime--1.0.sql
DOCS = README.pg_sysdatetime

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
