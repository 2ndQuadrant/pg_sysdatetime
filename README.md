`pg_sysdatetime`
==============

PostgreSQL SYSDATETIME() functions with support for higher precision timer
capture on Windows

This module may be compiled using Visual Studio on Windows. MinGW may also work
but is untested. Visual Studio 2012 was used in testing.

The timestamps returned are those provided by GetSystemTimeAsFileTime, which
can return up to 100ns precision, but in practice returns 1ms (10000ns) precision.

This functionality is equivalent to changing `src/port/gettimeofday.c` to use
`GetSystemTimeAsFileTime` directly, rather than reading `GetSystemTime` and
converting to `FileTime` with `SystemTimeToFileTime`, but doesn't require a
core server patch.

Quick install from binaries
---

If you downloaded a binary release zip  of this extension for your PostgreSQL major version and
platform (32-bit or 64-bit Windows), simply copy the contents of the "lib" folder to your
PostgreSQL install's "lib" folder, and do the same for the "share/extension" folder.
Then follow the instructions for "Usage (all platforms) below".

Compilation and installation (Windows)
---

To install this extension you must first compile it. Binaries are not published on the github page,
so you'll probably have to do this.

Compiling it is relatively trivial:

* Install Visual Studio Express 2012 or a compatible product

* Open `pg_sysdatetime.sln` in Visual Studio

* Change the paths to PostgreSQL, using Visual Studio's properties editor, or a simple text editor. If you only have one installed, just ignore the one you aren't interested in. The defaults will be fine except for the version number if you're using a 32-bit Visual Studio and installed PostgreSQL in the default location. To edit:
** With the properties editor
*** Open the Property Manager, from View -> Other Windows
*** Edit the "pg_sysdatetime" properties entry from any of the configuration/platform sections. It doesn't matter which, they're all the same file.
*** Change `PGMAJORVERSION` to your version, e.g. `9.3`.
*** If necessary, also change the values of `PGBASEDIR_x86` and `PGBASEDIR_x64` to point to the 32-bit and 64-bit PostgreSQL installs you want to build against.
*** Save the changes. When you save the changes they'll be applied to all sections.
** With a text editor
*** Open `pg_sysdatetime.props` in a text editor
*** Change `<PGMAJORVERSION>` to your major version, e.g. `9.3`
*** edit the values of the `<PGBASEDIR_x64>` and `<PGBASEDIR_x86>` elements to point to your Pg install(s), if needed

* In the toolbar at the top of the window, choose "Release" from the "Solution Configurations" pulldown, and choose the platform you want to compile for (32-bit or 64-bit).

* From the Build menu, choose Rebuild Solution

If you get an error about `libintl.h` being missing [then you've run into a packaging error in the 64-bit installer for PostgreSQL](http://blog.2ndquadrant.com/compiling-postgresql-extensions-visual-studio-windows/) and you will need to [copy `libintl.h` from here](https://gist.githubusercontent.com/ringerc/d57978ca0d3a3a13b5d7/raw/b7a695dcb451d2ac1dc4eecfbfa3198b8f29dff3/gistfile1.txt) into `include\libintl.h` in your PostgreSQL install then try the compile again.

After the compile completes, copy `pg_sysdatetime--1.0.sql` and
`pg_sysdatetime.control` to the `share\extension` directory of the PostgreSQL
install you compiled the extension for.

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

System timer frequency adjustment
---

Because GetSystemTimeAsFileTime may return much coarser timestamps
depending on hardware and operating system/version, the extension can use Windows
Multimedia features to request a higher timer resolution the first time it is
run. Just add:

    pg_sysdatetime.adjust_timer_resolution = on
	
to your `postgresql.conf`. 

The setting change can be applied with a `pg_ctl reload` and can be set per-session
by the superuser.

Querying system time resolution
---

You can use use `clockres.exe` from SysInternals to check your system's 
clock resolution (see link below), and can use:

    powercfg -energy -duration 5
	
to produce a report showing which applications have timer resolution requests active.

Future work
---

Future work could permit the use of GetSystemTimePreciseAsFiletime where 
running on Windows Server 2012 or Windows 8.

References
---

* GetSystemTimeAsFileTime: http://msdn.microsoft.com/en-us/library/windows/desktop/ms724397(v=vs.85).aspx
* GetSystemTimePreciseAsFileTime: http://msdn.microsoft.com/en-us/library/windows/desktop/hh706895(v=vs.85).aspx
* clockres.exe: http://technet.microsoft.com/en-us/sysinternals/bb897568.aspx
* GetSystemTimeAdjustment: http://msdn.microsoft.com/en-us/library/windows/desktop/ms724394(v=vs.85).aspx
* Windows Timer Project: http://www.windowstimestamp.com/description

