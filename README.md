`pg_sysdatetime`
==============

PostgreSQL SYSDATETIME() functions with support for higher precision timer
capture on Windows

This module may be compiled using Visual Studio on Windows, or using PGXS with
MinGW.

The timestamps returned are those provided by GetSystemTimeAsFileTime, which
can return up to 100ns precision, but may return much coarser timestamps
depending on hardware and operating system/version. Use `clockres.exe` from
SysInternals to check your system's clock resolution (see link below).

This functionality is equivalent to changing `src/port/gettimeofday.c` to use
`GetSystemTimeAsFileTime` directly, rather than reading `GetSystemTime` and
converting to `FileTime` with `SystemTimeToFileTime`, but doesn't require a
core server patch.

Future work could permit the use of GetSystemTimePreciseAsFiletime where 
running on Windows Server 2012 or Windows 8.

See:

* GetSystemTimeAsFileTime: http://msdn.microsoft.com/en-us/library/windows/desktop/ms724397(v=vs.85).aspx
* GetSystemTimePreciseAsFileTime: http://msdn.microsoft.com/en-us/library/windows/desktop/hh706895(v=vs.85).aspx
* clockres.exe: http://technet.microsoft.com/en-us/sysinternals/bb897568.aspx
* GetSystemTimeAdjustment: http://msdn.microsoft.com/en-us/library/windows/desktop/ms724394(v=vs.85).aspx
* Windows Timer Project: http://www.windowstimestamp.com/description

Compilation and installation (Windows)
---

To install this extension you must first compile it. Binaries are not provided.
Compiling it is relatively trivial:

* Install Visual Studio Express 2012 or a compatible product
* Open `pg_sysdatetime.sln`
* Open the Properties for the `pg_sysdatetime` project under the Solution Explorer
* Select "Configuration: All Configurations"
* Selct "Platform: x64" if you're building for 64-bit PostgreSQL, or "Platform: x86" if building for 32-bit PostgreSQL.
* Under C/C++ -> General, edit "Additional Include Directories" to point to your PostgreSQL install. Make sure to edit all four include-dir entries.
* Under Linker -> General, edit "Additional Library Directories" to point to your PostgreSQL install. If compiling for a 32-bit PostgreSQL on 64-bit Windows make sure to use the one in `%PROGRAMFILES(86)%`. This *must* be the same PostgreSQL install pointed to for include directories.
* Save the changes
* In the toolbar at the top of the window, choose "Release" from the "Solution Configurations" pulldown, and choose the same platform (x86 or x64) that you configured the solution properties for.
* From the Build menu, choose Rebuild Solution

Now copy `pg_sysdatetime--1.0.sql` and `pg_sysdatetime.control` to the
`share\extension` directory of the PostgreSQL install you compiled the
extension for.

Copy `x64\Release\pg_sysdatetime.dll` (for x64 builds) or
`x86\Release\pg_sysdatetime.dll` (for x86 builds) to your PostgreSQL install's
`lib` directory.

Installation (Linux)
---

Install as you would any other PGXS extension, e.g.

    make
    sudo make install

or if `pg_config` isn't on your default `PATH`, and replacing `/usr/pgsql-9.3/bin` with the path to `pg_config`:

    PATH=/usr/pgsql-9.3/bin:$PATH make
    sudo PATH=/usr/pgsql-9.3/bin:$PATH make install

Usage (all platforms)
---

Connect to the database you want to install the extension into, as a
superuser, and run:

    CREATE EXTENSION pg_sysdatetime;

Three functions are provided. All return a timestamp with the highest currently available precision.

* `sysutcdatetime` - returns a timestamp without time zone in UTC
* `sysdatetime` - returns a timestamp without time zone in local time as defined by `TimeZone`
* `sysdatetimeoffset` - returns a timestamp with time zone for the current UTC timestamp
