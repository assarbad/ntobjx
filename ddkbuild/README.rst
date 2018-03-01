=====================================
 Using WDK toolchain to build ntobjx
=====================================

Prerequisites
-------------
- Setting the environment variable ``WLHBASE`` or ``W7BASE`` depending on your WDK, as per the ``dkbuild.cmd`` help output.
- Visual C++ 2005 and the environment variable ``VS80COMNTOOLS`` set by it.
- Windows Vista SP1 WDK (6001.18002) or Windows 7 SP1 WDK (7600.16385.1).
    - I did not test this elsewhere.

About
-----
This subdirectory contains Visual Studio 8 projects which make use of my very
own ``ddkbuild.cmd`` to build against either the WLH (Windows Vista) or the W7
(Windows 7) Windows Driver Kit.

The Windows Driver Kits prior to those that again *require* the newer Visual
Studio versions (I think it started with Visual Studio 2013 and the Windows 8
WDK), contained their own toolchains. They were mostly self-contained, and
shaped after the internal build system used by Microsoft to build Windows.

This allowed the Windows **Driver** Kit to also be used to build user mode code.
What's more, it allowed one to build against msvcrt.dll as included in Windows
2000. So since that library (originally the VC6 runtime) became a first class
system library, this means we can scrape off some bytes from our binary and use
the system library instead (dynamic instead of static linking of the C runtime).

But there were and are glitches in these WDK versions and Microsoft won't help
you to fix these.

One such issue is that these WDKs contained ``aclapi.h`` and ``aclui.lib``, but
were missing the also needed ``aclui.h``. Gnarf!

Well, my workaround is wholly contained in the ``sources`` file in the form of
commands smuggled in via ``if`` conditions. This way we can use the environment
variable named ``VS80COMNTOOLS`` to locate the ``lib`` and ``include`` folders
of the Visual C++ 2005 compiler -- **a prerequisite!** -- and copy ``aclui.h``,
``aclapi.h`` and ``aclui.lib`` into the build folder. This way, without going
through too many hoops, a feature-complete ``ntobjx`` can be built using the WDK.

This scrapes off between 50 and 100 KiB.
